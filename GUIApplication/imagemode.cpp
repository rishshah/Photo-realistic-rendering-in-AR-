#include "imagemode.h"
#include "myglwidget.h"
#include "utils.h"
#include "ui_imagemode.h"
#include "openglwindow.h"
#include<QMessageBox>
#include<QFileDialog>
#include<QString>

#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>

ImageMode::ImageMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageMode)
{
    ui->setupUi(this);
}

ImageMode::~ImageMode()
{
    delete ui;
}

void ImageMode::on_image_dir_button_clicked()
{
    QString imageDirectory= QFileDialog::getExistingDirectory(this, tr("Choose Image Directory"),"", QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    ui->image_dir_label->setText(imageDirectory);
}

void ImageMode::on_image_csv_button_clicked()
{
    QString imageInfoCSV = QFileDialog::getOpenFileName(this, tr("Choose Images Info CSV"), "", tr("CSV (*.csv);; All files (*.*)"));
    ui->image_csv_label->setText(imageInfoCSV);
}

void ImageMode::on_run_pushButton_clicked()
{
    std::string imageDirectory = ui->image_dir_label->text().toUtf8().constData();
    std::string imageCSV = ui->image_csv_label->text().toUtf8().constData();
    imageDirectory = BASE_DIR "/V1_01_easy/mav0/cam0/data";
    imageCSV  = BASE_DIR "/V1_01_easy/mav0/cam0/data.csv";
//    if (imageDirectory == "-" or imageCSV == "-"){
//        QMessageBox::critical(this,"Error","Choose both files before run");
//        return;
//    }
//    putenv("LD_LIBRARY_PATH=" LD_LIBRARY_PATH);
//    chdir(BASE_DIR);
//    std::string s= "./test " VOCABULARY " " CAM_SETTING;
//    s += " ";
//    s += imageDirectory;
//    s += " ";
//    s += imageCSV;
//    int res = std::system(s.c_str());
//    if(res == 0){
        hide();
        OpenGLWindow o(this, imageDirectory, imageCSV);
        o.setModal(true);
        o.exec();
//    } else {
//        QMessageBox::critical(this,"Error","One or more of files chosen incorrect");
//    }
}
