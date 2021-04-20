#include "labeldialog.h"
#include "ui_labeldialog.h"
#include <iostream>

LabelDialog::LabelDialog(QVector<box_t>::iterator box_iter, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::LabelDialog),
        current_box(box_iter) {
    ui->setupUi(this);
    ui->comboBox->setCurrentText(current_box->getName());
}

LabelDialog::~LabelDialog() {
    delete ui;
}

void LabelDialog::on_buttonBox_accepted() {
    if (ui->checkBox->checkState() == Qt::Checked) {
        emit removeBoxEvent(current_box);
    } else {
        current_box->setByName(ui->comboBox->currentText());
        emit changeBoxEvent();
    }
}
