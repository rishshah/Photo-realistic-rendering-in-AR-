#include "mainwindow.h"
#include "utils.h"
#include "ui_mainwindow.h"
#include "openglwindow.h"
#include "imagemode.h"
#include<QMessageBox>
#include<QFileDialog>
#include<QString>

#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    ui->images_radiobutton->setChecked(true);
    ui->analysis_radiobutton->setChecked(true);
}

MainWindow::~MainWindow(){
    delete ui;
}



void MainWindow::on_continue_pushbutton_clicked()
{
    bool web_cam_mode;
    if (ui->images_radiobutton->isChecked()){
        web_cam_mode = false;
    } else if(ui->webcam_radiobutton->isChecked()){
        web_cam_mode = true;
    } else {
        QMessageBox::critical(this,"Error","Choose atleast one input option");
        return;
    }

    bool online_mode;
    if (ui->analysis_radiobutton->isChecked()){
        online_mode = false;
    } else if(ui->online_radiobutton->isChecked()){
        online_mode = true;
    } else {
        QMessageBox::critical(this,"Error","Choose atleast one mode");
        return;
    }

    hide();
    ImageMode m(this, web_cam_mode, online_mode);
    m.setModal(true);
    m.exec();
}



