#include "openglwindow.h"
#include "ui_openglwindow.h"
#include <QFileDialog>
#include <QMessageBox>

OpenGLWindow::OpenGLWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenGLWindow)
{
    ui->setupUi(this);
    ui->pan_radioButton->setChecked(true);
    ui->radioButton_add->setChecked(true);
}

OpenGLWindow::~OpenGLWindow()
{
    delete ui;
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
//    bool select_mode = ui->select_radioButton->isChecked();
//    bool add_mode = ui->radioButton_add->isChecked();
    ui->openGLWidget->mousePress(event);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event){
    bool select_mode = ui->select_radioButton->isChecked();
    bool add_mode = ui->radioButton_add->isChecked();
    ui->openGLWidget->mouseMove(event, select_mode, add_mode);
}


void OpenGLWindow::on_pushButton_confirm_clicked()
{
    if(ui->radioButton_add->isChecked()){
        ui->openGLWidget->add_plane();
    } else {
        ui->openGLWidget->remove_plane();
    }
}

void OpenGLWindow::on_pushButton_clicked()
{
    QString meshFile = QFileDialog::getOpenFileName(this, tr("Choose Mesh Object File"), "", tr("Object (*.obj);; All files (*.*)"));
    ui->openGLWidget->input_mesh(meshFile.toUtf8().constData());
}
