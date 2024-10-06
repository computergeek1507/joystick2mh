#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model_data.h"
#include "outputs/OutputManager.h"

#include <QMainWindow>
#include <QSettings>
#include <QtGamepad/QGamepad>

#include "spdlog/spdlog.h"
#include "spdlog/common.h"

#include <memory>
#include <filesystem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }

class QListWidgetItem;
class QListWidget;
class QTableWidget;
class QGamepad;
class QTimer;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public Q_SLOTS:

	void on_actionImport_Model_triggered();
	void on_actionSave_X_triggered();
	void on_actionClose_triggered();

	void on_actionOpen_Logs_triggered();
	void on_actionOpen_Settings_triggered();
	void on_actionAbout_triggered();
	void on_pushButtonStart_clicked();
	void on_pushButtonStop_clicked();
	void on_pushButtonReset_clicked();
	void on_checkBoxOutput_stateChanged(int state) ;
	void on_spinBoxDelay_valueChanged(int val);
	void on_tableWidgetChannels_cellDoubleClicked(int row, int column);
	void LoadControllers();
	void ReadJoystick();
	void DrawPlot();
	void onUpdateColor(QColor const& color);
	void onUpdateSettingsGUI();
	void RedrawModelSettings();

	void OnSetChannelData(uint32_t chan, uint8_t value);

	void LogMessage(QString const& message , spdlog::level::level_enum llvl = spdlog::level::level_enum::debug);

private:
	Ui::MainWindow *m_ui;
	std::shared_ptr<spdlog::logger> m_logger{ nullptr };
	std::unique_ptr<QSettings> m_settings{ nullptr };
	std::unique_ptr<ModelData> m_model{ nullptr };
	std::unique_ptr < OutputManager> m_output{ nullptr };
	QString m_appdir;
	bool m_recording{false};

	std::unique_ptr <QGamepad> m_gamepad{ nullptr };
	QTimer* m_controllerReader{ nullptr };
	//QLabel* m_colorLabel{ nullptr };

	void OpenFile(QString const& path);

};
#endif // MAINWINDOW_H
