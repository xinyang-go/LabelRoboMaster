#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>

class IndexQListWidgetItem : public QListWidgetItem {
public:
    IndexQListWidgetItem(QString name, int index) : QListWidgetItem(name), index(index) {

    }

    int getIndex() const { return index; }

private:
    int index;
};

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);
    QObject::connect(ui->addLabelPushButton, &QPushButton::clicked, ui->label, &DrawOnPic::setAddingMode);
    QObject::connect(ui->savePushButton, &QPushButton::clicked, ui->label, &DrawOnPic::saveLabel);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_openDirectoryPushButton_clicked() {
    QStringList image_filter = {"*.jpg", "*.png", "*.jpeg"};
    QDir dir = QFileDialog::getExistingDirectory(this, "", ".", QFileDialog::ShowDirsOnly);
    ui->fileListWidget->clear();
    int idx = 0;
    for (QString file : dir.entryList(image_filter)) {
        if (file == "." || file == "..") continue;
        ui->fileListWidget->addItem(new IndexQListWidgetItem(dir.absoluteFilePath(file), idx++));
    }
    ui->fileListWidget->setCurrentItem(ui->fileListWidget->item(0));
    ui->fileListHorizontalSlider->setMinimum(1);
    ui->fileListHorizontalSlider->setMaximum(ui->fileListWidget->count());
    ui->fileListHorizontalSlider->setValue(1);
}

void MainWindow::on_fileListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    if (current == nullptr) return;
    ui->label->setCurrentFile(current->text());
    int idx = static_cast<IndexQListWidgetItem *>(current)->getIndex();
    ui->fileListHorizontalSlider->setValue(idx + 1);
}

void MainWindow::on_label_labelChanged(const QVector<box_t> &labels) {
    ui->labelListWidget->clear();
    for (int i = 0; i < labels.size(); i++) {
        ui->labelListWidget->addItem(new IndexQListWidgetItem(labels[i].getName(), i));
    }
}

void MainWindow::on_labelListWidget_itemDoubleClicked(QListWidgetItem *item) {
    int idx = static_cast<IndexQListWidgetItem *>(item)->getIndex();
    delete dialog;
    dialog = new LabelDialog(ui->label->get_current_label().begin() + idx);
    dialog->setModal(true);
    QObject::connect(dialog, &LabelDialog::removeBoxEvent, ui->label, &DrawOnPic::removeBox);
    QObject::connect(dialog, &LabelDialog::changeBoxEvent, ui->label, &DrawOnPic::updateBox);
    dialog->show();
}

void MainWindow::on_labelListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    if (current == nullptr) return;
    int idx = static_cast<IndexQListWidgetItem *>(current)->getIndex();
    ui->label->setFocusBox(idx);
}

void MainWindow::on_smartPushButton_clicked() {
    ui->label->smart();

}

void MainWindow::on_nextPushButton_clicked() {
    if (ui->autoSaveCheckBox->checkState() == Qt::Checked) {
        ui->label->saveLabel();
    }
    int next_idx = ui->fileListWidget->currentRow() + 1;
    std::cout << "next: " << next_idx << std::endl;
    if (next_idx < ui->fileListWidget->count()) {
        ui->fileListWidget->setCurrentRow(next_idx);
    }
}

void MainWindow::on_prevPushButton_clicked() {
    if (ui->autoSaveCheckBox->checkState() == Qt::Checked) {
        ui->label->saveLabel();
    }
    int prev_idx = ui->fileListWidget->currentRow() - 1;
    std::cout << "prev: " << prev_idx << std::endl;
    if (prev_idx >= 0) {
        ui->fileListWidget->setCurrentRow(prev_idx);
    }
}

void MainWindow::on_fileListHorizontalSlider_valueChanged(int value) {
//    printf("on_fileListHorizontalSlider_valueChanged\n");
    QString text;
    ui->fileListLabel->setText(text.sprintf("[%d/%d]", value, ui->fileListHorizontalSlider->maximum()));
    ui->fileListWidget->setCurrentItem(ui->fileListWidget->item(value - 1));
}

void MainWindow::on_fileListHorizontalSlider_rangeChanged(int min, int max) {
    QString text;
    ui->fileListLabel->setText(text.sprintf("[%d/%d]", ui->fileListHorizontalSlider->value(), max));
}
