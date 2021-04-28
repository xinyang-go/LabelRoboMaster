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

protected:
    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *);

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

private:
    QString current_file;

    QSvgRenderer standard_tag_render[7];

    SmartModel model;

    double ratio;
    int dx, dy, img_w, img_h;
    QImage *im2show = nullptr;
    QImage *roi = nullptr;

    QVector<box_t> current_label;
    QPointF *draging = nullptr;
    int focus_box_index = -1;
    QVector<QPoint> adding;
    QPoint pos;

    QPen pen_point_focus;
    QPen pen_point;
    QPen pen_line_focus;
    QPen pen_line;
    QPen pen_text;

    enum mode_t {
        NORMAL_MODE,
        ADDING_MODE,
    } mode = NORMAL_MODE;
};

#endif // DRAWONPIC_H
