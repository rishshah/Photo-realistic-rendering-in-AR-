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

ImageMode::ImageMode(QWidget *parent, bool web_cam_mode, bool online_mode) :
    QDialog(parent),
    ui(new Ui::ImageMode){

    ui->setupUi(this);
    if (web_cam_mode){
        if(online_mode){
            m_mode = ONLINE_WEBCAM;
            ui->image_csv_button->setEnabled(false);
            ui->image_dir_button->setEnabled(false);
        } else {
            m_mode = OFFLINE_WEBCAM;
            ui->image_csv_button->setEnabled(false);
            ui->image_dir_button->setEnabled(false);
            ui->map_points_button->setEnabled(false);
        }
    } else{
        if(online_mode){
            m_mode = ONLINE_IMAGES;
        } else {
            m_mode = OFFLINE_IMAGES;
            ui->map_points_button->setEnabled(false);
        }
    }
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

void ImageMode::on_cam_settings_button_clicked()
{
    QString camSettings = QFileDialog::getOpenFileName(this, tr("Choose Settings file for Camera"), "", tr("All files (*.*)"));
    ui->cam_settings_label->setText(camSettings);
}

void ImageMode::on_map_points_button_clicked()
{
    QString mapPoints = QFileDialog::getOpenFileName(this, tr("Choose Precomputed Map Points file"), "", tr("All files (*.*)"));
    ui->map_points_label->setText(mapPoints);
}

void ImageMode::on_run_pushButton_clicked()
{
    std::string imageDirectory = ui->image_dir_label->text().toUtf8().constData();
    std::string imageCSV = ui->image_csv_label->text().toUtf8().constData();
    std::string camSettings = ui->cam_settings_label->text().toUtf8().constData();
    std::string mapPoints = ui->map_points_label->text().toUtf8().constData();

    imageDirectory = BASE_DIR "V1_01_easy/mav0/cam0/data";
    imageCSV  = BASE_DIR "V1_01_easy/mav0/cam0/data.csv";
    camSettings  = BASE_DIR "ORB_SLAM2/Examples/Monocular/EuRoC.yaml";
    mapPoints = OUTPUT_BASE "abc.txt";

    OpenGLWindow* o;
    switch (m_mode) {
    case ONLINE_WEBCAM:
        if (camSettings == "NA" or mapPoints == "NA"){
            QMessageBox::critical(this,"Error","Choose appropriate files before run");
            return;
        }
        o = new OpenGLWindow(this, camSettings, mapPoints);
        break;
    case ONLINE_IMAGES:
        if (camSettings == "NA" or imageDirectory == "NA" or imageCSV == "NA" or mapPoints == "NA"){
            QMessageBox::critical(this,"Error","Choose appropriate files before run");
            return;
        }
        o = new OpenGLWindow(this, camSettings, imageDirectory, imageCSV, mapPoints);
        break;
    case OFFLINE_IMAGES:
        if (camSettings == "NA" or imageDirectory == "NA" or imageCSV == "NA"){
            QMessageBox::critical(this,"Error","Choose appropriate files before run");
            return;
        }
        o = new OpenGLWindow(this, camSettings, imageDirectory, imageCSV);
        break;
    case OFFLINE_WEBCAM:
        if (camSettings == "NA"){
            QMessageBox::critical(this,"Error","Choose appropriate files before run");
            return;
        }
        o = new OpenGLWindow(this, camSettings);
        break;
    default:
        break;
    }
    hide();
    o->setModal(true);
    o->exec();
}

