#include "openglwindow.h"
#include "ui_openglwindow.h"
#include <QFileDialog>
#include <QMessageBox>

/**
 * @brief Constructor for UI setup for main Viewing window
 */
OpenGLWindow::OpenGLWindow(QWidget *parent):
    QDialog(parent),
    ui(new Ui::OpenGLWindow) {
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data();
    initializeUI();
}

/**
 * @brief Constructor for UI setup for main Viewing window
 *
 * @param camSettings Camera settings file location from input window
 */
OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings):
    QDialog(parent),
    ui(new Ui::OpenGLWindow) {
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings);
    initializeUI();
}

/**
 * @brief Constructor for UI setup for main Viewing window
 *
 * @param camSettings Camera settings file location from input window
 * @param mapPoints Map file location from input window
 */
OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings, std::string mapPoints):
    QDialog(parent),
    ui(new Ui::OpenGLWindow) {
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings, mapPoints);
    initializeUI();
}

/**
 * @brief Constructor for UI setup for main Viewing window
 *
 * @param camSettings Camera settings file location from input window
 * @param imageDirectory Image directory location from input window
 * @param imageCSV Image csv location from input window
 */
OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings, std::string imageDirectory, std::string imageCSV):
    QDialog(parent),
    ui(new Ui::OpenGLWindow) {
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings, imageDirectory, imageCSV);
    initializeUI();
}


/**
 * @brief Constructor for UI setup for main Viewing window
 *
 * @param camSettings Camera settings file location from input window
 * @param imageDirectory Image directory location from input window
 * @param imageCSV Image csv location from input window
 * @param mapPoints Map file location from input window
 */
OpenGLWindow::OpenGLWindow(QWidget *parent, std::string camSettings, std::string imageDirectory, std::string imageCSV, std::string mapPoints):
    QDialog(parent),
    ui(new Ui::OpenGLWindow) {
    ui->setupUi(this);
    ui->openGLWidget->fill_image_data(camSettings, imageDirectory, imageCSV, mapPoints);
    initializeUI();
}

/**
 * @brief Add appropriate buttons to UI window
 */
void OpenGLWindow::initializeUI() {
    ui->pan_radioButton->setChecked(true);
    ui->radioButton_add->setChecked(true);
    ui->pushButton_playback->setVisible(false);
    ui->pushButton_save->setVisible(false);
}

OpenGLWindow::~OpenGLWindow() {
    delete ui;
}

/**
 * @brief Pass the mouse press event to Main GLWidget class
 */
void OpenGLWindow::mousePressEvent(QMouseEvent *event) {
    ui->openGLWidget->mousePress(event);
}

/**
 * @brief Pass the mouse move event to Main GLWidget class
 */
void OpenGLWindow::mouseMoveEvent(QMouseEvent *event) {
    bool select_mode = ui->select_radioButton->isChecked();
    std::string insert_mode;
    if (ui->radioButton_add->isChecked())
        insert_mode = "ADD_PLANE";
    if (ui->radioButton_remove->isChecked())
        insert_mode = "REMOVE_PLANE";
    if (ui->radioButton_adjust->isChecked())
        insert_mode = "ADJUST_PLANE";
    if (ui->radioButton_mesh->isChecked())
        insert_mode = "ADJUST_MESH";

    ui->openGLWidget->mouseMove(event, select_mode, insert_mode);
}

/**
 * @brief Pass the key press event to Main GLWidget class
 */
void OpenGLWindow::keyPressEvent(QKeyEvent *event) {
    if (ui->pan_radioButton->isChecked()) {
        ui->openGLWidget->keyPress(event);
    }
}

/**
 * @brief Pass the functionality of "Confirm" button click to Main GLWidget class
 */
void OpenGLWindow::on_pushButton_confirm_clicked()
{
    if (ui->radioButton_add->isChecked()) {
        ui->openGLWidget->add_plane();
    } else if (ui->radioButton_remove->isChecked()) {
        ui->openGLWidget->remove_plane();
    } else if (ui->radioButton_adjust->isChecked()) {
        ui->openGLWidget->adjust_planes();
    } else if (ui->radioButton_mesh->isChecked()) {
        ui->openGLWidget->adjust_mesh();
    }
}

/**
 * @brief Pass Functionality of "Add Mesh" Button click to Main GLWidget class
 */
void OpenGLWindow::on_pushButton_add_mesh_clicked()
{

//    QString meshFile = QFileDialog::getOpenFileName(this, tr("Choose Mesh Object File"), "", tr("Object (*.obj);; All files (*.*)"));
//    std::string meshFile_str = meshFile.toUtf8().constData();
    
    // Debug mode fixed mesh file
    std::string meshFile_str = MESH_FILEPATH;
    
    ui->openGLWidget->input_mesh(meshFile_str);
    ui->pushButton_playback->setVisible(true);
    ui->pushButton_save->setVisible(true);
}

/**
 * @brief Pass Functionality of "Playback" Button click to Main GLWidget class
 */
void OpenGLWindow::on_pushButton_playback_clicked() {
    ui->openGLWidget->playback();
}

/**
 * @brief Exit the application
 */
void OpenGLWindow::on_pushButton_exit_clicked() {
    QApplication::quit();
}

/**
 * @brief Pass Functionality of "Save" Button click to Main GLWidget class
 */
void OpenGLWindow::on_pushButton_save_clicked() {
    ui->openGLWidget->saveAll();
}
