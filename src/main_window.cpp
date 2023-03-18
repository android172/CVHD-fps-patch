#include "main_window.h"
#include "../forms/ui_main_window.h"

#include <QFileDialog>
#include <QDirIterator>
#include <QMessageBox>
#include <file_utils.h>

uint compute_fps(uint option);
uint compute_option(uint fps);

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setAcceptDrops(true);

    // Disable future steps
    ui->cb_fps->setEnabled(false);
    ui->bt_mod_fps->setEnabled(false);
}
MainWindow::~MainWindow() {}

// ///////////////// //
// MAIN WINDOW SLOTS //
// ///////////////// //

#include <QDebug>

void MainWindow::on_bt_load_user_folder_clicked() {
    // Get path to USER folder
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle(tr("Select a folder"));
    if (!dialog.exec()) return;

    const QString user_folder_path = dialog.selectedFiles().at(0);
    const QString user_folder_name = user_folder_path.split('/').last();

    // Check if right folder is selected
    if (user_folder_name.compare("USRDIR") != 0) {
        QMessageBox::warning(
            this,
            "Invalid folder detected",
            "You must select the USRDIR folder of your CVHD copy."
        );
        return;
    }

    // Save user folder path
    _user_folder_path = user_folder_path;

    // Get fps from the settings file
    _settings_path = user_folder_path + "/" + "fps.patch";
    read_fps();

    // Update fps combo box
    ui->cb_fps->setCurrentIndex(_fps_option);
    ui->cb_fps->setEnabled(true);

    // Enable mod
    ui->bt_mod_fps->setEnabled(true);
}

void MainWindow::on_bt_mod_fps_clicked() {
    // Get new fps choice
    const auto new_fps = compute_fps(ui->cb_fps->currentIndex());

    // Check if change is necessary
    if (new_fps == _fps) return;

    // Calculate delay multiplier
    _delay_multiplier = new_fps / _fps;

    // Filter all .opd files within user folder & get their paths
    QDirIterator it(
        _user_folder_path,
        QStringList() << "*.opd",
        QDir::Files,
        QDirIterator::Subdirectories
    );
    QStringList opd_file_paths {};
    while (it.hasNext()) {
        it.next();
        opd_file_paths << it.filePath();
    }

    // Apply delay multiplier to all opd files
    for (const auto& opd_file_path : opd_file_paths)
        patch_opd(opd_file_path);

    // Save new settings
    _fps = new_fps;
    save_fps();
}

// /////////////////////////// //
// MAIN WINDOW PRIVATE METHODS //
// /////////////////////////// //

void MainWindow::read_fps() {
    // Get setting file if exists
    QFile setting_file(_settings_path);
    if (!setting_file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QMessageBox::warning(
            this,
            "Unexpected error",
            "There has been an error loading setting file. Something went "
            "wrong..."
        );
        return;
    }

    _fps_option = 0;

    // Get current fps option
    const auto first_line = QString::fromUtf8(setting_file.readLine());
    if (first_line.size() == 0) setting_file.write(QString::number(0).toUtf8());
    else _fps_option = first_line.toUInt();
    setting_file.close();

    // Check if option is legit
    if (_fps_option > max_fps_option) _fps_option = 0;

    // Compute fps
    _fps = compute_fps(_fps_option);
}
void MainWindow::save_fps() {
    // Compute option
    _fps_option = compute_option(_fps);

    // Get setting file if exists
    QFile setting_file(_settings_path);
    if (!setting_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            this,
            "Unexpected error",
            "There has been an error loading setting file. Something went "
            "wrong..."
        );
        return;
    }

    // Write the option
    setting_file.write(QString::number(_fps_option).toUtf8());
}

void MainWindow::patch_opd(QString opd_file_path) {
    // Open the opd file
    QFile opd_file(opd_file_path);
    if (!opd_file.open(QIODevice::ReadWrite | QIODevice::ExistingOnly)) {
        QMessageBox::warning(
            this,
            "Unexpected error",
            "File \"" + opd_file_path + "\" was skipped. I couldn't open it :/."
        );
        return;
    }

    // Get to animations section
    opd_file.seek(0x44);
    const auto animations_offset = read_type<uint>(opd_file);

    // Read animation count
    opd_file.seek(animations_offset);
    const auto animation_count = read_type<ushort>(opd_file);

    // Iterate animations
    opd_file.seek(animations_offset + 16);
    for (auto i = 0; i < animation_count; i++) {
        // Read frame count
        opd_file.read(33); // Skip name
        const auto frame_count = read_type<ushort>(opd_file);
        opd_file.read(4); // Skip unknown

        // Iterate frames
        for (auto i = 0; i < animation_count; i++) {
            opd_file.read(2); // Skip index

            // Get current delay
            const auto current_delay = peek_type<ushort>(opd_file);

            // Write new delay
            if (current_delay == 0x01 && _delay_multiplier < 1)
                read_type<ushort>(opd_file);
            else if (_delay_multiplier > 0 && current_delay > 0xFFFF / _delay_multiplier)
                write_type<ushort>(opd_file, 0xFFFF);
            else
                write_type<ushort>(opd_file, current_delay * _delay_multiplier);

            opd_file.read(17); // Skip rest
        }
    }

    opd_file.close();
}

// //////////////////////////// //
// MAIN WINDOW HELPER FUNCTIONS //
// //////////////////////////// //

uint compute_fps(uint option) { return (option + 1) * 30; }
uint compute_option(uint fps) { return fps / 30 - 1; }
