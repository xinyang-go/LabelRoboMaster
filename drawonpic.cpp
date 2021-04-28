#include "drawonpic.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTransform>
#include <iostream>

DrawOnPic::DrawOnPic(QWidget *parent) : QLabel(parent), model() {
    pen_point_focus.setWidth(4);
    pen_point_focus.setColor(Qt::red);

    pen_point.setWidth(4);
    pen_point.setColor(Qt::green);

    pen_line_focus.setWidth(1);
    pen_line_focus.setColor(Qt::red);

    pen_line.setWidth(1);
    pen_line.setColor(Qt::green);

    pen_text.setWidth(4);
    pen_text.setColor(Qt::green);

    standard_tag_render[0].load(QString(":/pic/tags/resource/G.svg"));
    standard_tag_render[1].load(QString(":/pic/tags/resource/1.svg"));
    standard_tag_render[2].load(QString(":/pic/tags/resource/2.svg"));
    standard_tag_render[3].load(QString(":/pic/tags/resource/3.svg"));
    standard_tag_render[4].load(QString(":/pic/tags/resource/4.svg"));
    standard_tag_render[5].load(QString(":/pic/tags/resource/5.svg"));
    standard_tag_render[6].load(QString(":/pic/tags/resource/B.svg"));

    this->setMouseTracking(true);
}

void DrawOnPic::mousePressEvent(QMouseEvent *event) {
    pos = event->pos();

    if (event->button() == Qt::LeftButton) {
        switch (mode) {
            case NORMAL_MODE:
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
        update();
    } else if (event->button() == Qt::RightButton) {
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
                    box.pts[0] = adding[0];
                    box.pts[1] = adding[1];
                    box.pts[2] = adding[2];
                    box.pts[3] = adding[3];
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

void DrawOnPic::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    // 绘制图片
    if (im2show != nullptr) {
        painter.drawImage(QPoint(dx, dy), *im2show);
    }

    if (!adding.empty()) {
        painter.setPen(pen_line);
        painter.drawPolygon(adding.data(), adding.size());
        painter.setPen(pen_point);
        painter.drawPoints(adding.data(), adding.size());
    }

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

    //
    for (int i = 0; i < current_label.size(); i++) {
        const auto &box = current_label[i];
        double delta_x1, delta_y1, delta_x2, delta_y2, proportion;
        delta_x1 = (box.pts[0].x() - box.pts[1].x()) / 2;
        delta_y1 = (box.pts[0].y() - box.pts[1].y()) / 2;
        delta_x2 = (box.pts[2].x() - box.pts[3].x()) / 2;
        delta_y2 = (box.pts[2].y() - box.pts[3].y()) / 2;
        switch (box.tag_id % 7) {
            case 0:
                proportion = 324. / 660.;
                break;
            case 1:
                proportion = 323. / 660.;
                break;
            case 2:
                proportion = 364. / 725.;
                break;
            case 3:
                proportion = 361. / 725.;
                break;
            case 4:
                proportion = 363. / 725.;
                break;
            case 5:
                proportion = 359. / 725.;
                break;
            case 6:
                proportion = 321. / 725.;
                break;
        }
        QPointF p1((box.pts[0].x() + box.pts[1].x()) / 2 + delta_x1 / proportion,
                   (box.pts[0].y() + box.pts[1].y()) / 2 + delta_y1 / proportion);
        QPointF p2((box.pts[0].x() + box.pts[1].x()) / 2 - delta_x1 / proportion,
                   (box.pts[0].y() + box.pts[1].y()) / 2 - delta_y1 / proportion);
        QPointF p3((box.pts[2].x() + box.pts[3].x()) / 2 + delta_x2 / proportion,
                   (box.pts[2].y() + box.pts[3].y()) / 2 + delta_y2 / proportion);
        QPointF p4((box.pts[2].x() + box.pts[3].x()) / 2 - delta_x2 / proportion,
                   (box.pts[2].y() + box.pts[3].y()) / 2 - delta_y2 / proportion);
        box_t new_box;
        new_box.pts[0] = p1;
        new_box.pts[1] = p2;
        new_box.pts[2] = p3;
        new_box.pts[3] = p4;
        QTransform transform;
        painter.setTransform(transform);
        if (i == focus_box_index) {
            painter.setPen(pen_line_focus);
        } else {
            painter.setPen(pen_line);
        }
        painter.drawPolygon(new_box.pts, 4);
        if (i == focus_box_index) {
            painter.setPen(pen_point_focus);
        } else {
            painter.setPen(pen_point);
        }
        painter.drawPoints(box.pts, 4);
        painter.setPen(pen_text);
        painter.drawText(box.pts[0], box.getName());

        QSvgRenderer &tag_render = standard_tag_render[box.tag_id];

        QPolygonF painter_ploygon;
        painter_ploygon.append({0, 0});
        painter_ploygon.append({0, geometry().height()});
        painter_ploygon.append({geometry().width(), geometry().height()});
        painter_ploygon.append({geometry().width(), 0});
//        QPolygonF std_tag_ploygon = box.getStandardPloygon();
        QPolygonF man_tag_ploygon;

        man_tag_ploygon.append(p1);
        man_tag_ploygon.append(p2);
        man_tag_ploygon.append(p3);
        man_tag_ploygon.append(p4);

        if (QTransform::quadToQuad(painter_ploygon, man_tag_ploygon, transform)) {
            painter.setTransform(transform);
            tag_render.render(&painter);
        }
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
    QImage *img = new QImage();
    img->load(current_file);
    ratio = std::min((double) QLabel::geometry().width() / img->width(),
                     (double) QLabel::geometry().height() / img->height());
    img_w = img->width();
    img_h = img->height();
    int w = img_w * ratio;
    int h = img_h * ratio;
    delete im2show;
    im2show = new QImage();
    *im2show = img->scaled(w, h);
    dx = (QLabel::geometry().width() - im2show->width()) / 2;
    dy = (QLabel::geometry().height() - im2show->height()) / 2;
    delete img;
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
    if (0 <= index && index < current_label.size()) {
        focus_box_index = index;
        update();
    }
}

void DrawOnPic::removeBox(QVector<box_t>::iterator box_iter) {
    current_label.erase(box_iter);
    emit labelChanged(current_label);
}

void DrawOnPic::smart() {
    if (current_file.isEmpty()) return;
    model.run(current_file, current_label);
    for(auto &label : current_label){
        label.pts[0].rx() = label.pts[0].x() * ratio + dx;
        label.pts[1].rx() = label.pts[1].x() * ratio + dx;
        label.pts[2].rx() = label.pts[2].x() * ratio + dx;
        label.pts[3].rx() = label.pts[3].x() * ratio + dx;
        label.pts[0].ry() = label.pts[0].y() * ratio + dy;
        label.pts[1].ry() = label.pts[1].y() * ratio + dy;
        label.pts[2].ry() = label.pts[2].y() * ratio + dy;
        label.pts[3].ry() = label.pts[3].y() * ratio + dy;
    }
    updateBox();
}

QVector<box_t> &DrawOnPic::get_current_label() {
    return current_label;
}

void DrawOnPic::updateBox() {
    update();
    emit labelChanged(current_label);
}

void DrawOnPic::reset() {
    current_file = "";
    delete im2show;
    delete roi;
    im2show = nullptr;
    roi = nullptr;

    current_label.clear();
    draging = nullptr;
    focus_box_index = -1;
    adding.clear();

    mode = NORMAL_MODE;
}

void DrawOnPic::loadLabel() {
    current_label.clear();
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.completeSuffix(), "txt");
    if (label_file.exists()) {
        QFile fp(label_file.absoluteFilePath());
        if (fp.open(QIODevice::ReadOnly)) {
            QTextStream stream(&fp);
            while (!fp.atEnd()) {
                box_t label;
                int idx;
                stream >> idx;
                label.color_id = idx / 7;
                label.tag_id = idx % 7;
                stream >> label.pts[0].rx() >> label.pts[0].ry()
                       >> label.pts[1].rx() >> label.pts[1].ry()
                       >> label.pts[2].rx() >> label.pts[2].ry()
                       >> label.pts[3].rx() >> label.pts[3].ry();
                label.pts[0].rx() = label.pts[0].x() * img_w * ratio + dx;
                label.pts[1].rx() = label.pts[1].x() * img_w * ratio + dx;
                label.pts[2].rx() = label.pts[2].x() * img_w * ratio + dx;
                label.pts[3].rx() = label.pts[3].x() * img_w * ratio + dx;
                label.pts[0].ry() = label.pts[0].y() * img_h * ratio + dy;
                label.pts[1].ry() = label.pts[1].y() * img_h * ratio + dy;
                label.pts[2].ry() = label.pts[2].y() * img_h * ratio + dy;
                label.pts[3].ry() = label.pts[3].y() * img_h * ratio + dy;
                current_label.append(label);
            }
        }
    }
    emit labelChanged(current_label);
}

void DrawOnPic::saveLabel() {
    if (current_label.empty()) return;
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.completeSuffix(), "txt");
    QFile fp(label_file.absoluteFilePath());
    if (fp.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QTextStream stream(&fp);
        for (const box_t &box: current_label) {
            stream << (box.color_id * 7 + box.tag_id) << " "
                   << (box.pts[0].x() - dx) / ratio / img_w << " " << (box.pts[0].y() - dy) / ratio / img_h << " "
                   << (box.pts[1].x() - dx) / ratio / img_w << " " << (box.pts[1].y() - dy) / ratio / img_h << " "
                   << (box.pts[2].x() - dx) / ratio / img_w << " " << (box.pts[2].y() - dy) / ratio / img_h << " "
                   << (box.pts[3].x() - dx) / ratio / img_w << " " << (box.pts[3].y() - dy) / ratio / img_h << endl;
        }
    }
}

QPointF *DrawOnPic::checkPoint() {
    for (box_t &box: current_label) {
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
