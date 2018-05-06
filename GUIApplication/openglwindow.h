#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QDialog>
#include<QMouseEvent>

namespace Ui {
class OpenGLWindow;
}
/**
 * @brief The main UI window where user performs all interactions with the real world and, creates and views augmentations
 */
class OpenGLWindow : public QDialog
{
    Q_OBJECT
public:
    explicit OpenGLWindow(QWidget *parent);
    explicit OpenGLWindow(QWidget *parent, std::string  camSettings);
    explicit OpenGLWindow(QWidget *parent, std::string  camSettings, std::string mapPoints);
    explicit OpenGLWindow(QWidget *parent, std::string  camSettings, std::string imageDirectory, std::string imageCSV);
    explicit OpenGLWindow(QWidget *parent, std::string  camSettings, std::string imageDirectory, std::string imageCSV, std::string mapPoints);
    ~OpenGLWindow();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private slots:
    void on_pushButton_confirm_clicked();

    void on_pushButton_add_mesh_clicked();

    void on_pushButton_playback_clicked();

    void on_pushButton_exit_clicked();

    void on_pushButton_save_clicked();

private:
    Ui::OpenGLWindow *ui;
    void initializeUI();
};

#endif // OPENGLWINDOW_H
