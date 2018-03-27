#include "openglwindow.h"
#include "ui_openglwindow.h"
#include <QFileDialog>
#include <QMessageBox>

OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings):
    QDialog(parent),
    ui(new Ui::OpenGLWindow){
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings);
    initializeUI();
}

OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings, std::string mapPoints):
    QDialog(parent),
    ui(new Ui::OpenGLWindow){
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings, mapPoints);
    initializeUI();
}


OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings, std::string imageDirectory, std::string imageCSV):
    QDialog(parent),
    ui(new Ui::OpenGLWindow){
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings, imageDirectory, imageCSV);
    initializeUI();
}



OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings, std::string imageDirectory, std::string imageCSV, std::string mapPoints):
    QDialog(parent),
    ui(new Ui::OpenGLWindow){
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings, imageDirectory, imageCSV, mapPoints);
    initializeUI();
}

void OpenGLWindow::initializeUI(){
    ui->pan_radioButton->setChecked(true);
    ui->radioButton_add->setChecked(true);
    ui->pushButton_playback->setVisible(false);
}

OpenGLWindow::~OpenGLWindow(){
    delete ui;
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event){
  ui->openGLWidget->mousePress(event);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event){
    bool select_mode = ui->select_radioButton->isChecked();
    std::string insert_mode;
    if(ui->radioButton_add->isChecked())
        insert_mode = "ADD_PLANE";
    if(ui->radioButton_remove->isChecked())
        insert_mode = "REMOVE_PLANE";
    if(ui->radioButton_adjust->isChecked())
        insert_mode = "ADJUST_PLANE";
    if(ui->radioButton_mesh->isChecked())
        insert_mode = "ADJUST_MESH";

    ui->openGLWidget->mouseMove(event, select_mode, insert_mode);
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
    } else if(ui->radioButton_remove->isChecked()){
        ui->openGLWidget->remove_plane();
    } else if(ui->radioButton_adjust->isChecked()){
        ui->openGLWidget->adjust_planes();
    } else if(ui->radioButton_mesh->isChecked()){
        ui->openGLWidget->adjust_mesh();
    }
}

void OpenGLWindow::on_pushButton_add_mesh_clicked()
{
//    QString meshFile = QFileDialog::getOpenFileName(this, tr("Choose Mesh Object File"), "", tr("Object (*.obj);; All files (*.*)"));
//    std::string meshFile_str = meshFile.toUtf8().constData();
    std::string meshFile_str = BASE_DIR "sample_mesh.obj";
    ui->openGLWidget->input_mesh(meshFile_str);
    ui->pushButton_playback->setVisible(true);
}

void OpenGLWindow::on_pushButton_playback_clicked(){
   ui->openGLWidget->playback();
}

void OpenGLWindow::on_pushButton_exit_clicked(){
    QApplication::quit();
}
