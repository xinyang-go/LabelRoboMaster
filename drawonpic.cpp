#include "drawonpic.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTransform>
#include <iostream>
#include <fstream>

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
    // 大装甲标注点对应在svg图中的4个坐标
    big_pts.append({11., 141.});
    big_pts.append({11., 344.});
    big_pts.append({860., 344.});
    big_pts.append({860., 141.});
    // 小装甲标注点对应在svg图中的4个坐标
    small_pts.append({11., 146.});
    small_pts.append({11., 371.});
    small_pts.append({546., 371.});
    small_pts.append({546., 146.});

    standard_tag_render[0].load(QString(":/pic/tags/resource/G.svg"));
    standard_tag_render[1].load(QString(":/pic/tags/resource/1.svg"));
    standard_tag_render[2].load(QString(":/pic/tags/resource/2.svg"));
    standard_tag_render[3].load(QString(":/pic/tags/resource/3.svg"));
    standard_tag_render[4].load(QString(":/pic/tags/resource/4.svg"));
    standard_tag_render[5].load(QString(":/pic/tags/resource/5.svg"));
    standard_tag_render[6].load(QString(":/pic/tags/resource/O.svg"));
    standard_tag_render[7].load(QString(":/pic/tags/resource/Bs.svg"));
    standard_tag_render[8].load(QString(":/pic/tags/resource/Bb.svg"));

    this->setMouseTracking(true);
    this->grabKeyboard();
}

void DrawOnPic::mousePressEvent(QMouseEvent *event) {
    pos = event->pos();

    if (event->button() == Qt::LeftButton) {
        switch (mode) {
            case NORMAL_MODE:
                draging = checkPoint();
                if (draging) {
                    for (int i = 0; i < current_label_of_pic.size(); ++i) {
                        for (int j = 0; j < 4; ++j) {
                            if (draging == current_label_of_pic[i].pts + j) {
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
        update();
    } else if (event->button() == Qt::RightButton) {
        right_drag_pos = pos;
        setNormalMode();
    }
}

void DrawOnPic::mouseMoveEvent(QMouseEvent *event) {
    pos = event->pos();

    switch (mode) {
        case NORMAL_MODE:
            if (draging) {
                *draging = event->pos();
                // std::cout << "draging: [" << draging->x() << ", " << draging->y() << "]" << std::endl;
            }
            update();
            break;
        case ADDING_MODE:
            update();
            break;
        default:
            break;
    }

    // 右键拖动
    if(event->buttons() & Qt::RightButton){
        dx += pos.rx() - right_drag_pos.rx();
        dy += pos.ry() - right_drag_pos.ry();
        right_drag_pos = pos;
        update_label_of_pic(); // 更新已添加目标的绘图位置
    }
}

void DrawOnPic::mouseReleaseEvent(QMouseEvent *event) {
    pos = event->pos();

    if (event->button() == Qt::LeftButton) {
        switch (mode) {
            case NORMAL_MODE:
                draging = nullptr;
                break;
            case ADDING_MODE:
                std::cout << "add point: [" << event->pos().x() << ", " << event->pos().y() << "]" << std::endl;
                adding.append(event->pos());
                if (adding.size() == 4) {
                    box_t box;
                    box.pts[0].rx() = (adding[0].x() - dx) / ratio;
                    box.pts[0].ry() = (adding[0].y() - dy) / ratio;
                    box.pts[1].rx() = (adding[1].x() - dx) / ratio;
                    box.pts[1].ry() = (adding[1].y() - dy) / ratio;
                    box.pts[2].rx() = (adding[2].x() - dx) / ratio;
                    box.pts[2].ry() = (adding[2].y() - dy) / ratio;
                    box.pts[3].rx() = (adding[3].x() - dx) / ratio;
                    box.pts[3].ry() = (adding[3].y() - dy) / ratio;
                    current_label_of_raw.append(box);
                    update_label_of_pic();
                    setNormalMode();
                    emit labelChanged(current_label_of_raw);
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
        update_label_of_pic();
    }
}

void DrawOnPic::wheelEvent(QWheelEvent* event){
    constexpr double delta = 0.1;
    double delta_ratio = ratio * delta;
    if(event->delta() > 0){ 
        // 滚轮向上，缩小
        ratio += delta_ratio;
        dx -= (event->pos().rx() - dx) * delta;
        dy -= (event->pos().ry() - dy) * delta;
        update_label_of_pic();
        if(im2show && img_raw) *im2show = img_raw->scaled(img_raw->width() * ratio, img_raw->height() * ratio);
    }else{
        // 滚轮向下，放大
        ratio -= delta_ratio;
        dx += (event->pos().rx() - dx) * delta;
        dy += (event->pos().ry() - dy) * delta;
        update_label_of_pic();
        if(im2show && img_raw) *im2show = img_raw->scaled(img_raw->width() * ratio, img_raw->height() * ratio);
    }
}

void DrawOnPic::keyPressEvent(QKeyEvent* event) {
    switch (event->key())    {
    case Qt::Key_Escape: // ESC取消选中
        focus_box_index = -1;
        update();
        break;
    case Qt::Key_Delete: // Delete删除选中
        if(focus_box_index >= 0){
            current_label_of_raw.removeAt(focus_box_index);
            update_label_of_pic();
            emit labelChanged(current_label_of_raw);
            update();
        }
    default:
        break;
    }
    
}

void DrawOnPic::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    // 绘制图片
    if (im2show != nullptr) {
        painter.drawImage(QPoint(dx, dy), *im2show);
    }

    // 绘制添加中的目标
    if (!adding.empty()) {
        painter.setPen(pen_line);
        painter.drawPolygon(adding.data(), adding.size());
        painter.setPen(pen_point);
        painter.drawPoints(adding.data(), adding.size());
    }

    // 绘制已添加目标
    for (int i = 0; i < current_label_of_pic.size(); i++) {
        const auto &box = current_label_of_pic[i];
        bool is_big = (box.tag_id == 0 || box.tag_id == 1 || box.tag_id == 8);
        // 计算svg到绘图坐标系的变换，并绘制svg图
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
        QTransform transform;
        QTransform::quadToQuad(pts_on_painter, pts_for_show, transform);
        painter.setTransform(transform);
        standard_tag_render[box.tag_id].render(&painter);
        // 清除变换，确保线段粗细不变
        painter.setTransform(QTransform());
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
        painter.drawPolygon(transform.map(painter_ploygen));
        // 绘制4个定位点
        if(i == focus_box_index) painter.setPen(pen_point_focus);
        else painter.setPen(pen_point);
        painter.drawPoints(box.pts, 4);
        
//         double delta_x1, delta_y1, delta_x2, delta_y2, proportion;
//         delta_x1 = (box.pts[0].x() - box.pts[1].x()) / 2;
//         delta_y1 = (box.pts[0].y() - box.pts[1].y()) / 2;
//         delta_x2 = (box.pts[2].x() - box.pts[3].x()) / 2;
//         delta_y2 = (box.pts[2].y() - box.pts[3].y()) / 2;
//         switch (box.tag_id % 9) {
//             case 0:
//                 proportion = 324. / 660.;
//                 break;
//             case 1:
//                 proportion = 323. / 660.;
//                 break;
//             case 2:
//                 proportion = 364. / 725.;
//                 break;
//             case 3:
//                 proportion = 361. / 725.;
//                 break;
//             case 4:
//                 proportion = 363. / 725.;
//                 break;
//             case 5:
//                 proportion = 359. / 725.;
//                 break;
//             case 6:
//                 proportion = 359. / 725.;
//                 break;
//             case 7:
//                 proportion = 359. / 725.;
//                 break;
//             case 8:
//                 proportion = 321. / 725.;
//                 break;
//         }
//         QPointF p1((box.pts[0].x() + box.pts[1].x()) / 2 + delta_x1 / proportion,
//                    (box.pts[0].y() + box.pts[1].y()) / 2 + delta_y1 / proportion);
//         QPointF p2((box.pts[0].x() + box.pts[1].x()) / 2 - delta_x1 / proportion,
//                    (box.pts[0].y() + box.pts[1].y()) / 2 - delta_y1 / proportion);
//         QPointF p3((box.pts[2].x() + box.pts[3].x()) / 2 + delta_x2 / proportion,
//                    (box.pts[2].y() + box.pts[3].y()) / 2 + delta_y2 / proportion);
//         QPointF p4((box.pts[2].x() + box.pts[3].x()) / 2 - delta_x2 / proportion,
//                    (box.pts[2].y() + box.pts[3].y()) / 2 - delta_y2 / proportion);
//         box_t new_box;
//         new_box.pts[0] = p1;
//         new_box.pts[1] = p2;
//         new_box.pts[2] = p3;
//         new_box.pts[3] = p4;
//         QTransform transform;
//         painter.setTransform(transform);
//         if (i == focus_box_index) {
//             painter.setPen(pen_box_focus);
//         } else {
//             if(box.getName()[0] == 'R'){
//                 pen_box.setColor(Qt::red);
//             }else if(box.getName()[0] == 'B'){
//                 pen_box.setColor(Qt::blue);
//             }else if(box.getName()[0] == 'P'){
//                 pen_box.setColor(Qt::cyan);
//             }else{
//                 pen_box.setColor(Qt::green);
//             }
//             painter.setPen(pen_box);
//         }
//         painter.drawPolygon(new_box.pts, 4);
//         if (i == focus_box_index) {
//             painter.setPen(pen_point_focus);
//         } else {
//             painter.setPen(pen_point);
//         }
//         painter.drawPoints(box.pts, 4);
//         painter.setPen(pen_text);
//         painter.drawText(box.pts[0], box.getName());

//         QSvgRenderer &tag_render = standard_tag_render[box.tag_id];

//         QPolygonF painter_ploygon;
//         painter_ploygon.append({0., 0.});
//         painter_ploygon.append({0., (double) geometry().height()});
//         painter_ploygon.append({(double) geometry().width(), (double) geometry().height()});
//         painter_ploygon.append({(double) geometry().width(), 0.});
// //        QPolygonF std_tag_ploygon = box.getStandardPloygon();
//         QPolygonF man_tag_ploygon;

//         man_tag_ploygon.append(p1);
//         man_tag_ploygon.append(p2);
//         man_tag_ploygon.append(p3);
//         man_tag_ploygon.append(p4);

//         if (QTransform::quadToQuad(painter_ploygon, man_tag_ploygon, transform)) {
//             painter.setTransform(transform);
//             tag_render.render(&painter);
//         }
    }
    
    // 绘制鼠标相关
    painter.setTransform(QTransform());
    QPointF *focus = nullptr;
    switch (mode) {
        case NORMAL_MODE:
            focus = checkPoint();
            if (focus != nullptr) {
                painter.setPen(pen_point_focus);
            }
            if (draging != nullptr) {
                drawRoi(painter);
            }
            break;
        case ADDING_MODE:
            painter.setPen(pen_line);
            painter.drawLine(QPoint(0, pos.y()), QPoint(QLabel::geometry().width(), pos.y()));
            painter.drawLine(QPoint(pos.x(), 0), QPoint(pos.x(), QLabel::geometry().height()));
            drawRoi(painter);
            break;
        default:
            break;
    }
}

void DrawOnPic::setCurrentFile(QString file) {
    reset();
    current_file = file;
    loadImage();
    loadLabel();
    setNormalMode();
}

void DrawOnPic::loadImage() {
    delete img_raw;
    img_raw = new QImage();
    img_raw->load(current_file);
    ratio = std::min((double) QLabel::geometry().width() / img_raw->width(),
                     (double) QLabel::geometry().height() / img_raw->height());
    delete im2show;
    im2show = new QImage();
    *im2show = img_raw->scaled(img_raw->width() * ratio, img_raw->height() * ratio);
    dx = (QLabel::geometry().width() - im2show->width()) / 2;
    dy = (QLabel::geometry().height() - im2show->height()) / 2;
    update();
}

void DrawOnPic::setAddingMode() {
    if (im2show == nullptr) return;
    mode = ADDING_MODE;
    adding.clear();
}

void DrawOnPic::setNormalMode() {
    mode = NORMAL_MODE;
    draging = nullptr;
    adding.clear();
}

void DrawOnPic::setFocusBox(int index) {
    if (0 <= index && index < current_label_of_pic.size()) {
        focus_box_index = index;
        update();
    }
}

void DrawOnPic::removeBox(QVector<box_t>::iterator box_iter) {
    current_label_of_raw.erase(box_iter);
    update_label_of_pic();
    emit labelChanged(current_label_of_raw);
}

void DrawOnPic::smart() {
    if (current_file.isEmpty()) return;
    if (!model.run(current_file, current_label_of_raw)) {
        QMessageBox::warning(nullptr, "warning", "Cannot run smart!\n"
                                                 "This maybe due to compiling without openvino or a broken model file.\n"
                                                 "See warning.txt for detailed information.",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    update_label_of_pic();
    updateBox();
}

void DrawOnPic::update_label_of_pic(){
    current_label_of_pic = current_label_of_raw;
    for (auto &label : current_label_of_pic) {
        label.pts[0].rx() = label.pts[0].x() * ratio + dx;
        label.pts[1].rx() = label.pts[1].x() * ratio + dx;
        label.pts[2].rx() = label.pts[2].x() * ratio + dx;
        label.pts[3].rx() = label.pts[3].x() * ratio + dx;
        label.pts[0].ry() = label.pts[0].y() * ratio + dy;
        label.pts[1].ry() = label.pts[1].y() * ratio + dy;
        label.pts[2].ry() = label.pts[2].y() * ratio + dy;
        label.pts[3].ry() = label.pts[3].y() * ratio + dy;
    }
}

QVector<box_t> &DrawOnPic::get_current_label() {
    return current_label_of_raw;
}

void DrawOnPic::updateBox() {
    update_label_of_pic();
    update();
    emit labelChanged(current_label_of_raw);
}

void DrawOnPic::reset() {
    current_file = "";
    delete img_raw;
    delete im2show;
    delete roi;
    img_raw = nullptr;
    im2show = nullptr;
    roi = nullptr;

    current_label_of_raw.clear();
    current_label_of_pic.clear();
    draging = nullptr;
    focus_box_index = -1;
    adding.clear();

    mode = NORMAL_MODE;
}

void DrawOnPic::loadLabel() {
    current_label_of_pic.clear();
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
                label.pts[0].rx() = label.pts[0].x() * img_raw->width();
                label.pts[1].rx() = label.pts[1].x() * img_raw->width();
                label.pts[2].rx() = label.pts[2].x() * img_raw->width();
                label.pts[3].rx() = label.pts[3].x() * img_raw->width();
                label.pts[0].ry() = label.pts[0].y() * img_raw->height();
                label.pts[1].ry() = label.pts[1].y() * img_raw->height();
                label.pts[2].ry() = label.pts[2].y() * img_raw->height();
                label.pts[3].ry() = label.pts[3].y() * img_raw->height();
                current_label_of_raw.append(label);
                update_label_of_pic();
            }
        }
    }
    emit labelChanged(current_label_of_pic);
}

void DrawOnPic::saveLabel() {
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.completeSuffix(), "txt");
    QFile fp(label_file.absoluteFilePath());
    if (current_label_of_pic.empty()) {
        fp.remove();
        return;
    }
    if (fp.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QTextStream stream(&fp);
        for (const box_t &box: current_label_of_raw) {
            stream << (box.color_id * 9 + box.tag_id) << " "
                   << box.pts[0].x() / img_raw->width() << " " << box.pts[0].y() / img_raw->height() << " "
                   << box.pts[1].x() / img_raw->width() << " " << box.pts[1].y() / img_raw->height() << " "
                   << box.pts[2].x() / img_raw->width() << " " << box.pts[2].y() / img_raw->height() << " "
                   << box.pts[3].x() / img_raw->width() << " " << box.pts[3].y() / img_raw->height() << endl;
        }
    }
}

QPointF *DrawOnPic::checkPoint() {
    for (box_t &box: current_label_of_pic) {
        for (int i = 0; i < 4; i++) {
            QPointF dif = box.pts[i] - pos;
            if (dif.manhattanLength() < 5) {
                return box.pts + i;
            }
        }
    }
    return nullptr;
}

void DrawOnPic::drawRoi(QPainter &painter) {
    if (roi == nullptr) roi = new QImage();
    *roi = im2show->copy(QRect(pos.x() - dx - 16, pos.y() - dy - 16, 32, 32)).scaled(128, 128);
    painter.setPen(pen_line);
    painter.drawImage(pos.x() + 32, pos.y() + 32, *roi);
    painter.drawLine(QPoint(pos.x() + 32 + 64, pos.y() + 32), QPoint(pos.x() + 32 + 64, pos.y() + 32 + 128));
    painter.drawLine(QPoint(pos.x() + 32, pos.y() + 32 + 64), QPoint(pos.x() + 32 + 128, pos.y() + 32 + 64));
    painter.drawRect(pos.x() + 32, pos.y() + 32, 128, 128);
}
