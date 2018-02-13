#include "openglwindow.h"
#include "ui_openglwindow.h"
#include <QFileDialog>
#include <QMessageBox>

OpenGLWindow::OpenGLWindow(QWidget *parent, std::string image_dir, std::string image_info_csv) :
    QDialog(parent),
    ui(new Ui::OpenGLWindow)
{
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(image_dir, image_info_csv);
    ui->pan_radioButton->setChecked(true);
    ui->radioButton_add->setChecked(true);
    ui->pushButton_playback->setVisible(false);
}



OpenGLWindow::~OpenGLWindow()
{
    delete ui;
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
  ui->openGLWidget->mousePress(event);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event){
    bool select_mode = ui->select_radioButton->isChecked();
    bool add_mode = ui->radioButton_add->isChecked();
    ui->openGLWidget->mouseMove(event, select_mode, add_mode);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *event){
    if(ui->pan_radioButton->isChecked()){
        ui->openGLWidget->keyPress(event);
    }
}

void OpenGLWindow::on_pushButton_confirm_clicked()
{
    if(ui->radioButton_add->isChecked()){
        ui->openGLWidget->add_plane();
    } else {
        ui->openGLWidget->remove_plane();
    }
}

void OpenGLWindow::on_pushButton_add_mesh_clicked()
{
    QString meshFile = QFileDialog::getOpenFileName(this, tr("Choose Mesh Object File"), "", tr("Object (*.obj);; All files (*.*)"));
    ui->openGLWidget->input_mesh(meshFile.toUtf8().constData());
    ui->pushButton_playback->setVisible(true);
}

void OpenGLWindow::on_pushButton_playback_clicked()
{
   ui->openGLWidget->playback();
}
