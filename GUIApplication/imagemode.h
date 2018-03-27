#ifndef IMAGEMODE_H
#define IMAGEMODE_H

#include <QDialog>

namespace Ui {
class ImageMode;
}

class ImageMode : public QDialog
{
    Q_OBJECT

public:
    explicit ImageMode(QWidget *parent, bool web_cam_mode, bool online_mode);
    ~ImageMode();

private slots:
    void on_image_dir_button_clicked();

    void on_image_csv_button_clicked();

    void on_run_pushButton_clicked();

    void on_cam_settings_button_clicked();

    void on_map_points_button_clicked();

private:
    Ui::ImageMode *ui;
    int m_mode;
};

#endif // IMAGEMODE_H
