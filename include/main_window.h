#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

  private slots:

    void on_bt_load_user_folder_clicked();
    void on_bt_mod_fps_clicked();

  private:
    Ui::MainWindow* ui;

    // Settings
    const uint max_fps_option = 1;

    // State
    QString _user_folder_path;
    QString _settings_path;

    uint _delay_multiplier;
    uint _fps_option;
    uint _fps;

    void read_fps();
    void save_fps();

    void patch_opd(QString opd_file_path);
};

#endif // MAIN_WINDOW_H
