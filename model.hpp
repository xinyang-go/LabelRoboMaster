//
// Created by xinyang on 2021/4/28.
//

#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <opencv2/opencv.hpp>
#include <QMap>
#include <QString>
#include <QPoint>
#include <QPolygon>

struct box_t {
    QPointF pts[4];
    int color_id = 0, tag_id = 0;
    float conf = -1;

    QString getName() const {
        static const QString tag2name[] = {"G", "1", "2", "3", "4", "5", "O", "Bs", "Bb"};
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
                                                    {"O", 6},
                                                    {"s", 7},
                                                    {"b", 8}};
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
        pts.append({0., (2 <= tag_id && tag_id <= 7) ? (725.) : (660.)});
        pts.append({(2 <= tag_id && tag_id <= 7) ? (780.) : (1180.), (2 <= tag_id && tag_id <= 7) ? (725.) : (660.)});
        pts.append({(2 <= tag_id && tag_id <= 7) ? (780.) : (1180.), 0.});
        return pts;
    }
};

class SmartModel {
public:
    explicit SmartModel();

    bool run(const QString &image_file, QVector<box_t> &boxes);

    QString get_mode() const { return mode; }

private:
    cv::dnn::Net net;
    QString mode;
};

#endif /* _MODEL_HPP_ */
