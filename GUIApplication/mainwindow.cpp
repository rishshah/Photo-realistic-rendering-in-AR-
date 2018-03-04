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
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_continue_pushbutton_clicked()
{
    if (ui->images_radiobutton->isChecked()){
        hide();
        ImageMode m;
        m.setModal(true);
        m.exec();

    } else if(ui->webcam_radiobutton->isChecked()){
        putenv("LD_LIBRARY_PATH=" LD_LIBRARY_PATH);
        chdir(BASE_DIR);
        int result = std::system("./test " VOCABULARY " " CAM_SETTING);
        std::string image_dir = BASE_DIR "images";
        std::string image_csv = BASE_DIR "image_info.csv";
        if (result == 0){
            hide();
            OpenGLWindow o(this,image_dir, image_csv);
            o.setModal(true);
            o.exec();
        } else {
            QMessageBox::critical(this,"Error","Internal");
        }

    } else {
        QMessageBox::critical(this,"Error","Choose atleast one option");
    }
}

