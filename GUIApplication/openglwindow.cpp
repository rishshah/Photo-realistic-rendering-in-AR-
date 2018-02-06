#include "openglwindow.h"
#include "ui_openglwindow.h"

OpenGLWindow::OpenGLWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenGLWindow)
{
    ui->setupUi(this);
    ui->pan_radioButton->setChecked(true);
}

OpenGLWindow::~OpenGLWindow()
{
    delete ui;
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event){
    if(ui->select_radioButton->isChecked()){
        ui->openGLWidget->select_mousePressEvent(event);
    } else{
        ui->openGLWidget->pan_mousePressEvent(event);
    }
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event){
    if(ui->select_radioButton->isChecked()){
        ui->openGLWidget->select_mouseMoveEvent(event);
    } else{
        ui->openGLWidget->pan_mouseMoveEvent(event);
    }
}

void OpenGLWindow::on_pushButton_clicked(){
    ui->openGLWidget->best_plane();
}
