#include "drawonpic.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTransform>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <chrono>

DrawOnPic::DrawOnPic(QWidget *parent) : QLabel(parent), model() {
    pen_point_focus.setWidth(5);
    pen_point_focus.setColor(Qt::green);

    pen_point.setWidth(5);
    pen_point.setColor(Qt::green);

    pen_line.setWidth(1);
    pen_line.setColor(Qt::green);

    pen_text.setWidth(4);
    pen_text.setColor(Qt::green);

    pen_box.setWidth(2);
    pen_box.setColor(Qt::green);

    pen_box_focus.setWidth(3);
    pen_box_focus.setColor(Qt::red);

    // 大装甲svg宽高
    big_svg_ploygen.append({0., 0.});
    big_svg_ploygen.append({0., 478.});
    big_svg_ploygen.append({871., 478.});
    big_svg_ploygen.append({871., 0.});
    // 小装甲svg宽高
    small_svg_ploygen.append({0., 0.});
    small_svg_ploygen.append({0., 516.});
    small_svg_ploygen.append({557., 516.});
    small_svg_ploygen.append({557., 0.});
    
// 重新测量后的坐标点定义
    // 大装甲标注点对应在svg图中的4个坐标
    big_pts.append({0., 140.61});
    big_pts.append({0., 347.39});
    big_pts.append({871., 347.39});
    big_pts.append({871., 140.61});
    // 小装甲标注点对应在svg图中的4个坐标
    small_pts.append({0., 143.26});
    small_pts.append({0., 372.74});
    small_pts.append({557., 372.74});
    small_pts.append({557., 143.26});
    
// 历史坐标点定义    
//     // 大装甲标注点对应在svg图中的4个坐标
//     big_pts.append({11., 141.});
//     big_pts.append({11., 344.});
//     big_pts.append({860., 344.});
//     big_pts.append({860., 141.});
//     // 小装甲标注点对应在svg图中的4个坐标
//     small_pts.append({11., 146.});
//     small_pts.append({11., 371.});
//     small_pts.append({546., 371.});
//     small_pts.append({546., 146.});
    
    // 加载svg图片
    standard_tag_render[0].load(QString(":/pic/tags/resource/G.svg"));
    standard_tag_render[1].load(QString(":/pic/tags/resource/1.svg"));
    standard_tag_render[2].load(QString(":/pic/tags/resource/2.svg"));
    standard_tag_render[3].load(QString(":/pic/tags/resource/3.svg"));
    standard_tag_render[4].load(QString(":/pic/tags/resource/4.svg"));
    standard_tag_render[5].load(QString(":/pic/tags/resource/5.svg"));
    standard_tag_render[6].load(QString(":/pic/tags/resource/O.svg"));
    standard_tag_render[7].load(QString(":/pic/tags/resource/Bs.svg"));
    standard_tag_render[8].load(QString(":/pic/tags/resource/Bb.svg"));
    // 读取鼠标键盘的数据
    this->setMouseTracking(true);
    this->grabKeyboard();
}

void DrawOnPic::mousePressEvent(QMouseEvent *event) {
    pos = event->pos();     // 记录当前鼠标位置

    if (event->button() == Qt::LeftButton) {
        switch (mode) {
            case NORMAL_MODE:
                // normal模式下，鼠标左键用于确定兴趣点。
                // 用于拖动定位点。
                draging = checkPoint();
                if (draging) {
                    for (int i = 0; i < current_label.size(); ++i) {
                        for (int j = 0; j < 4; ++j) {
                            if (draging == current_label[i].pts + j) {
                                focus_box_index = i;
                                break;
                            }
                        }
                    }
                }
                break;
            case ADDING_MODE:
                break;
            default:
                break;
        }
        update();   // 更新绘图
    } else if (event->button() == Qt::RightButton) {
        // 右键用于图像拖动，记录拖动起点
        right_drag_pos = pos;   
        setNormalMode();
        update();   // 更新绘图
    }
}

void DrawOnPic::mouseMoveEvent(QMouseEvent *event) {
    pos = event->pos();

    switch (mode) {
        case NORMAL_MODE:
            if (draging) {  // 如果正在拖动一个定位点，则将定位点位置设置成鼠标位置
                *draging = norm2img.inverted().map(img2label.inverted().map(pos));
            }
            update();
            break;
        case ADDING_MODE:
            update();
            break;
        default:
            break;
    }

    // 右键拖动，计算图片移动后的QTransform
    if(event->buttons() & Qt::RightButton){ 
        QTransform delta;
        delta.translate(pos.x() - right_drag_pos.x(), pos.y() - right_drag_pos.y());
        img2label = img2label * delta;
        right_drag_pos = pos;
        update();
    }
}

void DrawOnPic::mouseReleaseEvent(QMouseEvent *event) {
    pos = event->pos();
    if (event->button() == Qt::LeftButton) {
        switch (mode) {
            case NORMAL_MODE:   // 松开左键，停止拖动定位点
                draging = nullptr;
                break;
            case ADDING_MODE:   // 松开左键，添加一个定位点
                adding.append(norm2img.inverted().map(img2label.inverted().map(pos)));
                if (adding.size() == 4) {
                    box_t box;
                    for(int i=0; i<4; i++) box.pts[i] = adding[i];
                    current_label.append(box);
                    setNormalMode();
                    emit labelChanged(current_label);
                }
                update();
                break;
            default:
                break;
        }
    }
}

void DrawOnPic::mouseDoubleClickEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        // 右键双击恢复默认视图
        // 偷懒，使用重新加载图像实现上述功能
        loadImage(); 
    }
}

void DrawOnPic::wheelEvent(QWheelEvent* event){
    // 滚轮缩放图像，计算缩放后的QTransform

    const double delta = (event->delta() > 0) ? (1.1) : (1 / 1.1);
    
    double mx = event->pos().x();
    double my = event->pos().y();

    QTransform delta_transform;
    delta_transform.translate(mx*(1-delta), my*(1-delta)).scale(delta, delta);

    img2label = img2label * delta_transform;
}

void DrawOnPic::keyPressEvent(QKeyEvent* event) {
    switch (event->key())    {
    case Qt::Key_Escape: // ESC取消选中
        focus_box_index = -1;
        update();
        break;
    case Qt::Key_Delete: // Delete删除选中
        if(focus_box_index >= 0){
            current_label.removeAt(focus_box_index);
            focus_box_index = -1;
            emit labelChanged(current_label);
            update();
        }
    default:
        break;
    }
    
}

void DrawOnPic::paintEvent(QPaintEvent *) {
    if (img == nullptr) return;
    
    // 绘制图片
    QPainter painter(this);
    painter.setTransform(img2label);
    painter.drawImage(0, 0, *img);

    // 绘制添加中的目标
    if (!adding.empty()) {
        painter.setTransform(QTransform());
        painter.setPen(pen_line);
        painter.drawPolygon(img2label.map(norm2img.map(adding)));
        painter.setPen(pen_point);
        painter.drawPoints(img2label.map(norm2img.map(adding)));
    }

    // 绘制已添加目标
    for (int i = 0; i < current_label.size(); i++) {
        const auto &box = current_label[i];
        bool is_big = (box.tag_id == 0 || box.tag_id == 1 || box.tag_id == 8);
        // 计算svg到显示坐标系的变换
        QPolygonF painter_ploygen;
        painter_ploygen.append({0., 0.});
        painter_ploygen.append({0., (double) geometry().height()});
        painter_ploygen.append({(double) geometry().width(), (double) geometry().height()});
        painter_ploygen.append({(double) geometry().width(), 0.});
        QTransform svg2painter;
        QTransform::quadToQuad(is_big ? big_svg_ploygen : small_svg_ploygen, painter_ploygen, svg2painter);
        QPolygonF pts_on_painter = svg2painter.map(is_big ? big_pts : small_pts);
        QPolygonF pts_for_show;
        for(auto &pt: box.pts) pts_for_show.append(pt);
        pts_for_show = img2label.map(norm2img.map(pts_for_show));
        QTransform transform;
        QTransform::quadToQuad(pts_on_painter, pts_for_show, transform);
        // 绘制svg
        painter.setTransform(transform);
        standard_tag_render[box.tag_id].render(&painter);
        // 绘制目标四边形边框
        if (i == focus_box_index) {
            painter.setPen(pen_box_focus);
        } else {
            if(box.getName()[0] == 'R'){
                pen_box.setColor(Qt::red);
            }else if(box.getName()[0] == 'B'){
                pen_box.setColor(Qt::blue);
            }else if(box.getName()[0] == 'P'){
                pen_box.setColor(Qt::cyan);
            }else{
                pen_box.setColor(Qt::green);
            }
            painter.setPen(pen_box);
        }
        painter.setTransform(QTransform());
        painter.drawPolygon(transform.map(painter_ploygen));
        // 绘制4个定位点
        if(i == focus_box_index) painter.setPen(pen_point_focus);
        else painter.setPen(pen_point);
        painter.drawPoints(pts_for_show);
        // 绘制标签名
        painter.setPen(pen_text);
        painter.drawText(img2label.map(norm2img.map(box.pts[0])), box.getName());
    }
    
    // 绘制鼠标相关
    painter.setTransform(QTransform());
    QPointF *focus = nullptr;
    switch (mode) {
        case NORMAL_MODE:
            if(draging){
                painter.setPen(pen_line);
                painter.drawLine(QPoint(0, pos.y()), QPoint(QLabel::geometry().width(), pos.y()));
                painter.drawLine(QPoint(pos.x(), 0), QPoint(pos.x(), QLabel::geometry().height()));
                drawROI(painter);
            }
            break;
        case ADDING_MODE:
            painter.setPen(pen_line);
            painter.drawLine(QPoint(0, pos.y()), QPoint(QLabel::geometry().width(), pos.y()));
            painter.drawLine(QPoint(pos.x(), 0), QPoint(pos.x(), QLabel::geometry().height()));
            drawROI(painter);
            break;
        default:
            break;
    }
}

void DrawOnPic::setCurrentFile(QString file) {
    // 切换标注中的文件
    reset();
    current_file = file;
    loadImage();
    loadLabel();
    setNormalMode();
}

void DrawOnPic::loadImage() {
    // 加载图片时，需要计算两个QTransform的初值
    // 一个用于缩放图像显示
    // 一个用于将归一化标签坐标变为对应像素坐标
    delete img;
    img = new QImage();
    img->load(current_file);
    double ratio = std::min((double) QLabel::geometry().width() / img->width(),
                            (double) QLabel::geometry().height() / img->height());
    
    QPolygonF norm_polygen;
    norm_polygen.append({0., 0.});
    norm_polygen.append({0., 1.});
    norm_polygen.append({1., 1.});
    norm_polygen.append({1., 0.});

    QPolygonF image_polygen;
    image_polygen.append({0., 0.});
    image_polygen.append({0., (double)img->height()});
    image_polygen.append({(double)img->width(), (double)img->height()});
    image_polygen.append({(double)img->width(), 0.});

    double x1 = (geometry().width() - img->width() * ratio) / 2;
    double y1 = (geometry().height() - img->height() * ratio) / 2;
    QPolygonF label_polygen;
    label_polygen.append({x1, y1});
    label_polygen.append({x1, y1 + ratio * img->height()});
    label_polygen.append({x1 + ratio * img->width(), y1 + ratio * img->height()});
    label_polygen.append({x1 + ratio * img->width(), y1});
    
    QTransform::quadToQuad(norm_polygen, image_polygen, norm2img);
    QTransform::quadToQuad(image_polygen, label_polygen, img2label);
    
    update();
}

void DrawOnPic::setAddingMode() {
    // 设置当前模式为正在添加一个新目标
    if (img == nullptr) return;
    mode = ADDING_MODE;
    adding.clear();
    update();
}

void DrawOnPic::setNormalMode() {
    // 设置当前模式为normal
    mode = NORMAL_MODE;
    draging = nullptr;
    adding.clear();
}

void DrawOnPic::setFocusBox(int index) {
    // 设置当前选中目标（高亮，快捷删除）
    if (0 <= index && index < current_label.size()) {
        focus_box_index = index;
        update();
    }
}

void DrawOnPic::removeBox(QVector<box_t>::iterator box_iter) {
    // 删除一个目标
    current_label.erase(box_iter);
    emit labelChanged(current_label);
}

void DrawOnPic::smart() {
    // 对当前图片进行一次智能标注。
    if (current_file.isEmpty()) return;
    using namespace std::chrono;
    auto t1 = high_resolution_clock::now(); // 统计运行时间
    if (!model.run(current_file, current_label)) {
        // 运行失败报错
        QMessageBox::warning(nullptr, "warning", "Cannot run smart!\n"
                                                 "This maybe due to compiling without openvino or a broken model file.\n"
                                                 "See warning.txt for detailed information.",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    auto t2 = high_resolution_clock::now(); // 统计运行时间
    latency_ms = duration_cast<milliseconds>(t2-t1).count();
    qDebug("latency=%dms", latency_ms);
    // 模型输出的像素坐标变为归一化坐标
    for(auto &l: current_label){
        for(auto &pt: l.pts) {
            pt.rx() /= img->width();
            pt.ry() /= img->height();
        }
    }
    updateBox();
}

void DrawOnPic::updateBox() {
    update();
    emit labelChanged(current_label);
}

void DrawOnPic::reset() {
    current_file.clear();
    delete img;
    img = nullptr;
    current_label.clear();
    emit labelChanged(current_label);
    draging = nullptr;
    focus_box_index = -1;
    adding.clear();
    mode = NORMAL_MODE;
    latency_ms = -1;
}

void DrawOnPic::loadLabel() {
    // 加载当前图片对应的标签文件
    current_label.clear();
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.completeSuffix(), "txt");
    if (label_file.exists()) {
        QFile fp(label_file.absoluteFilePath());
        if (fp.open(QIODevice::ReadOnly)) {
            QTextStream stream(&fp);
            while (true) {
                box_t label;
                double idx;
                stream >> idx;
                if (stream.atEnd()) break;
                label.color_id = (int)idx / 9;
                label.tag_id = (int)idx % 9;
                stream >> label.pts[0].rx() >> label.pts[0].ry()
                       >> label.pts[1].rx() >> label.pts[1].ry()
                       >> label.pts[2].rx() >> label.pts[2].ry()
                       >> label.pts[3].rx() >> label.pts[3].ry();
                current_label.append(label);
            }
        }
    }
    emit labelChanged(current_label);
}

void DrawOnPic::saveLabel() {
    // 保存当前图片的目标到对应的标签文件
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.completeSuffix(), "txt");
    QFile fp(label_file.absoluteFilePath());
    if (current_label.empty()) {    
        // 如果当前图片没有任何目标，则删除标签文件。
        // 主要避免之前保存过有目标的文件。
        fp.remove();
        return;
    }
    if (fp.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QTextStream stream(&fp);
        for (const box_t &box: current_label) {
            stream << (box.color_id * 9 + box.tag_id) << " "
                   << box.pts[0].x() << " " << box.pts[0].y() << " "
                   << box.pts[1].x() << " " << box.pts[1].y() << " "
                   << box.pts[2].x() << " " << box.pts[2].y() << " "
                   << box.pts[3].x() << " " << box.pts[3].y() << endl;
        }
    }
}

void DrawOnPic::drawROI(QPainter &painter) {
    // 绘制ROI放大图
    QRect label_rect = QRect(QPoint(pos.x(), pos.y()) - QPoint{16, 16}, QPoint(pos.x(), pos.y()) + QPoint{16, 16});
    QRect img_rect = img2label.inverted().mapRect(label_rect);
    QImage roi_img = img->copy(img_rect).scaled(128, 128);
    painter.setTransform(QTransform());
    painter.drawImage(pos+QPoint(32, 32), roi_img);
    painter.setPen(pen_line);
    painter.drawRect(pos.x() + 32, pos.y() + 32, 128, 128);
    painter.drawLine(QPoint(pos.x() + 32 + 64, pos.y() + 32), QPoint(pos.x() + 32 + 64, pos.y() + 32 + 128));
    painter.drawLine(QPoint(pos.x() + 32, pos.y() + 32 + 64), QPoint(pos.x() + 32 + 128, pos.y() + 32 + 64));
}

QPointF *DrawOnPic::checkPoint() {
    // 检查当前鼠标位置距离哪个定位点最近
    // 如果距离小于5，则判定为选中该点
    // 用于拖动定位点
    for (box_t &box: current_label) {
        for (int i = 0; i < 4; i++) {
            QPointF dif = img2label.map(norm2img.map(box.pts[i])) - pos;
            if (dif.manhattanLength() < 5) {
                return box.pts + i;
            }
        }
    }
    return nullptr;
}

QVector<box_t>& DrawOnPic::get_current_label() {
    return current_label;
}
