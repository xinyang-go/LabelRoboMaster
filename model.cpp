//
// Created by xinyang on 2021/4/28.
//

#include "model.hpp"
#include <fstream>
#include <QFile>

template<class F, class T, class ...Ts>
T reduce(F &&func, T x, Ts ...xs) {
    if constexpr (sizeof...(Ts) > 0) {
        return func(x, reduce(std::forward<F>(func), xs...));
    } else {
        return x;
    }
}

template<class T, class ...Ts>
T reduce_min(T x, Ts ...xs) {
    return reduce([](auto a, auto b) { return std::min(a, b); }, x, xs...);
}

template<class T, class ...Ts>
T reduce_max(T x, Ts ...xs) {
    return reduce([](auto a, auto b) { return std::max(a, b); }, x, xs...);
}

// 判断目标外接矩形是否相交，用于nms。
// 等效于thres=0的nms。
static inline bool is_overlap(const QPointF pts1[4], const QPointF pts2[4]) {
    cv::Rect2f box1, box2;
    box1.x = reduce_min(pts1[0].x(), pts1[1].x(), pts1[2].x(), pts1[3].x());
    box1.y = reduce_min(pts1[0].y(), pts1[1].y(), pts1[2].y(), pts1[3].y());
    box1.width = reduce_max(pts1[0].x(), pts1[1].x(), pts1[2].x(), pts1[3].x()) - box1.x;
    box1.height = reduce_max(pts1[0].y(), pts1[1].y(), pts1[2].y(), pts1[3].y()) - box1.y;
    box2.x = reduce_min(pts2[0].x(), pts2[1].x(), pts2[2].x(), pts2[3].x());
    box2.y = reduce_min(pts2[0].y(), pts2[1].y(), pts2[2].y(), pts2[3].y());
    box2.width = reduce_max(pts2[0].x(), pts2[1].x(), pts2[2].x(), pts2[3].x()) - box2.x;
    box2.height = reduce_max(pts2[0].y(), pts2[1].y(), pts2[2].y(), pts2[3].y()) - box2.y;
    return (box1 & box2).area() > 0;
}

static inline int argmax(const float *ptr, int len) {
    int max_arg = 0;
    for (int i = 1; i < len; i++) {
        if (ptr[i] > ptr[max_arg]) max_arg = i;
    }
    return max_arg;
}

float inv_sigmoid(float x) {
    return -std::log(1 / x - 1);
}

float sigmoid(float x) {
    return 1 / (1 + std::exp(-x));
}


SmartModel::SmartModel() {
    qDebug("initializing smart model... please wait.");
    try {
        // 首先尝试加载openvino-int8模型，并进行一次空运行。
        // 用于判断该模型在当前环境下是否可用。
        QFile xml_file(":/nn/resource/model-opt-int8.xml");
        QFile bin_file(":/nn/resource/model-opt-int8.bin");
        xml_file.open(QIODevice::ReadOnly);
        bin_file.open(QIODevice::ReadOnly);
        auto xml_bytes = xml_file.readAll();
        auto bin_bytes = bin_file.readAll();
        net = cv::dnn::readNetFromModelOptimizer((uchar*)xml_bytes.data(), xml_bytes.size(), 
                                                 (uchar*)bin_bytes.data(), bin_bytes.size());
        cv::Mat input(640, 640, CV_8UC3);       // 构造输入数据
        auto x = cv::dnn::blobFromImage(input);
        net.setInput(x);
        net.forward();
        mode = "openvino-int8-cpu";     // 设置当前模型模式
        return;
    } catch (cv::Exception &) {
        // openvino int8 unavailable
    }

    // int8模型不可用，加载fp32模型
    QFile onnx_file(":/nn/resource/model-opt.onnx");
    onnx_file.open(QIODevice::ReadOnly);
    auto onnx_bytes = onnx_file.readAll();

    net = cv::dnn::readNetFromONNX(onnx_bytes.data(), onnx_bytes.size());

    try {
        // 尝试使用openvino模式运行fp32模型
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        cv::Mat input(640, 640, CV_8UC3);
        auto x = cv::dnn::blobFromImage(input) / 255.;
        net.setInput(x);
        net.forward();
        mode = "openvino-fp32-cpu"; // 设置当前模型模式
    } catch (cv::Exception &) {
        // 无法使用openvino运行fp32模型，则使用默认的opencv-dnn模式。
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        mode = "dnn-fp32-cpu";      // 设置当前模型模式
    }
}

bool SmartModel::run(const QString &image_file, QVector<box_t> &boxes) {
    try {
        // 加载图片，并等比例resize为640x640。空余部分用0进行填充。
        auto img = cv::imread(image_file.toStdString());
        float scale = 640.f / std::max(img.cols, img.rows);
        cv::resize(img, img, {(int)round(img.cols * scale), (int)round(img.rows * scale)});
        cv::Mat input(640, 640, CV_8UC3, 127);
        img.copyTo(input({0, 0, img.cols, img.rows}));
        
        // TODO: 为了兼容int8模型和fp32模型的不同输入格式而加的临时操作
        //       后续会统一两个模型的输入格式
        cv::Mat x;
        if(mode == "openvino-int8-cpu") {
            x = cv::dnn::blobFromImage(input);
        } else {
            cv::cvtColor(input, input, cv::COLOR_BGR2RGB);
            x = cv::dnn::blobFromImage(input) / 255;
        }
        // 模型推理
        net.setInput(x);
        auto y = net.forward();
        // 模型后处理
        QVector<box_t> before_nms;
        for (int i = 0; i < y.size[1]; i++) {
            float *result = (float *) y.data + i * y.size[2];
            if (result[8] < inv_sigmoid(0.5)) continue;
            box_t box;
            for (int i = 0; i < 4; i++) {
                box.pts[i].rx() = (result[i * 2 + 0]) / scale;
                box.pts[i].ry() = (result[i * 2 + 1]) / scale;
            }
            box.color_id = argmax(result + 9, 4);
            box.tag_id = argmax(result + 13, 9);
            box.conf = sigmoid(result[8]);
            before_nms.append(box);
        }
        std::sort(before_nms.begin(), before_nms.end(), [](box_t &b1, box_t &b2) {
            return b1.conf > b2.conf;
        });
        boxes.clear();
        boxes.reserve(before_nms.size());
        std::vector<bool> is_removed(before_nms.size());
        for (int i = 0; i < before_nms.size(); i++) {
            if (is_removed[i]) continue;
            boxes.append(before_nms[i]);
            for (int j = i + 1; j < before_nms.size(); j++) {
                if (is_removed[j]) continue;
                if (is_overlap(before_nms[i].pts, before_nms[j].pts)) is_removed[j] = true;
            }
        }
        return true;
    } catch (std::exception &e) {
        std::ofstream ofs("warning.txt", std::ios::app);
        time_t t;
        time(&t);
        ofs << asctime(localtime(&t)) << "\t" << e.what() << std::endl;
        return false;
    }
}
