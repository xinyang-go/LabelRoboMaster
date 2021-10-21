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

    QString model_mode() const { return model.get_mode(); }

    void reset();
    
    QVector<box_t> &get_current_label();

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

    void updateBox();

signals:

    void labelChanged(const QVector<box_t> &);

private:

    void loadLabel();

    void drawROI(QPainter& painter);

    QPointF *checkPoint();

private:
    QString current_file;

    QSvgRenderer standard_tag_render[9];

    SmartModel model;

    QTransform norm2img;        // 归一化图像坐标到图像坐标
    QTransform img2label;       // 图像坐标到实际显示的坐标

    // double ratio;
    // int dx, dy;
    QImage *img = nullptr;

    QPolygonF big_svg_ploygen, small_svg_ploygen;
    QPolygonF big_pts, small_pts;

    QVector<box_t> current_label;   // 归一化坐标

    QPointF *draging = nullptr;
    int focus_box_index = -1;
    QVector<QPointF> adding;
    QPointF pos;

    QPointF right_drag_pos;

    QPen pen_point_focus;
    QPen pen_point;
    QPen pen_box_focus;
    QPen pen_box;
    QPen pen_line;
    QPen pen_text;

    int latency_ms = -1;

    enum mode_t {
        NORMAL_MODE,
        ADDING_MODE,
    } mode = NORMAL_MODE;
};

#endif // DRAWONPIC_H
