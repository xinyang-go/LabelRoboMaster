#ifndef DRAWONPIC_H
#define DRAWONPIC_H

#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QtSvg/QSvgRenderer>
#include "model.hpp"

class DrawOnPic : public QLabel {
Q_OBJECT

public:
    explicit DrawOnPic(QWidget *parent = nullptr);

    bool with_openvino() const { return model.with_openvino(); }

protected:
    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void mouseDoubleClickEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent* event);

    void keyPressEvent(QKeyEvent* event);

    void paintEvent(QPaintEvent *event);

public slots:

    void setCurrentFile(QString file);

    void loadImage();

    void saveLabel();

    void setAddingMode();

    void setNormalMode();

    void setFocusBox(int index);

    void removeBox(box_t *box);

    void smart();

    QVector<box_t> &get_current_label();

    void updateBox();

signals:

    void labelChanged(const QVector<box_t> &);

private:
    void reset();

    void loadLabel();

    QPointF *checkPoint();

    void drawRoi(QPainter &painter);

    void update_label_of_pic();

private:
    QString current_file;

    QSvgRenderer standard_tag_render[9];

    SmartModel model;

    double ratio;
    int dx, dy;
    QImage *img_raw = nullptr;
    QImage *im2show = nullptr;
    QImage *roi = nullptr;

    QPolygonF big_svg_ploygen, small_svg_ploygen;
    QPolygonF big_pts, small_pts;

    QVector<box_t> current_label_of_pic;
    QVector<box_t> current_label_of_raw;
    QPointF *draging = nullptr;
    int focus_box_index = -1;
    QVector<QPoint> adding;
    QPoint pos;

    QPoint right_drag_pos;

    QPen pen_point_focus;
    QPen pen_point;
    QPen pen_box_focus;
    QPen pen_box;
    QPen pen_line;
    QPen pen_text;

    enum mode_t {
        NORMAL_MODE,
        ADDING_MODE,
    } mode = NORMAL_MODE;
};

#endif // DRAWONPIC_H
