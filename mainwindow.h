#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "drawonpic.h"
#include "labeldialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:

    void on_openDirectoryPushButton_clicked();

    void on_fileListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_label_labelChanged(const QVector<box_t> &);

    void on_labelListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_labelListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_smartPushButton_clicked();

    void on_smartAllPushButton_clicked();

    void on_nextPushButton_clicked();

    void on_prevPushButton_clicked();

    void on_fileListHorizontalSlider_valueChanged(int value);

    void on_fileListHorizontalSlider_rangeChanged(int min, int max);

private:
    Ui::MainWindow *ui = nullptr;

    LabelDialog *dialog = nullptr;
};

#endif // MAINWINDOW_H
