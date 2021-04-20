#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include <QDialog>
#include "drawonpic.h"

namespace Ui {
    class LabelDialog;
}

class LabelDialog : public QDialog {
Q_OBJECT

public:
    explicit LabelDialog(QVector<box_t>::iterator box_iter, QWidget *parent = 0);

    ~LabelDialog();

signals:

    void removeBoxEvent(QVector<box_t>::iterator box_iter);

    void changeBoxEvent();

private slots:

    void on_buttonBox_accepted();

private:
    Ui::LabelDialog *ui = nullptr;

    QVector<box_t>::iterator current_box;
};

#endif // LABELDIALOG_H
