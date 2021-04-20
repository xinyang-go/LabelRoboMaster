#ifndef DRAWONPIC_H
#define DRAWONPIC_H

#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QtSvg/QSvgRenderer>
#include <QMap>

struct box_t {
    QPointF pts[4];
    int color_id = 0, tag_id = 0;

    QString getName() const {
        static const QString tag2name[] = {"G", "1", "2", "3", "4", "5", "B"};
        static const QString color2name[] = {"B", "R", "N", "P"};
        return color2name[color_id] + tag2name[tag_id];
    }

    bool setByName(const QString &name) {
        static const QMap<QString, int> name2tag = {{"G", 0},
                                                    {"1", 1},
                                                    {"2", 2},
                                                    {"3", 3},
                                                    {"4", 4},
                                                    {"5", 5},
                                                    {"B", 6}};
        static const QMap<QString, int> name2color = {{"B", 0},
                                                      {"R", 1},
                                                      {"N", 2},
                                                      {"P", 3}};
        if (name2color.contains(name[0]) && name2tag.contains(name[1])) {
            color_id = name2color[name[0]];
            tag_id = name2tag[name[1]];
            return true;
        } else {
            return false;
        }
    }

    QPolygonF getStandardPloygon() const {
        QPolygonF pts;
        pts.append({0., 0.});
        pts.append({0., (2 <= tag_id && tag_id <= 5) ? (725.) : (660.)});
        pts.append({(2 <= tag_id && tag_id <= 5) ? (780.) : (1180.), (2 <= tag_id && tag_id <= 5) ? (725.) : (660.)});
        pts.append({(2 <= tag_id && tag_id <= 5) ? (780.) : (1180.), 0.});
        return pts;
    }
};

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
