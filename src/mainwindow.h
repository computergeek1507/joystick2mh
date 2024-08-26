#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public Q_SLOTS:

	void on_actionSave_X_triggered();
	void on_actionSave_Y_triggered();
	void on_actionClose_triggered();

	void on_actionAbout_triggered();
	void on_actionOpen_Logs_triggered();
	void on_pushButtonStart_clicked();
	void on_pushButtonStop_clicked();
	void on_pushButtonReset_clicked();
	void LoadControllers();
	void ReadJoystick();
	void DrawPlot();

	QString CreateVCDate();
	void writeXMLFile(QString const& xmlFileName);

		void LogMessage(QString const& message , spdlog::level::level_enum llvl = spdlog::level::level_enum::debug);

private:
	Ui::MainWindow *m_ui;
	std::shared_ptr<spdlog::logger> m_logger{ nullptr };
	std::unique_ptr<QSettings> m_settings{ nullptr };
	QString m_appdir;

	std::unique_ptr <QGamepad> m_gamepad{ nullptr };
	QTimer* m_controllerReader{ nullptr };

	std::vector<std::pair<int, double>> m_values;
	void OpenFile(QString const& path);

};
#endif // MAINWINDOW_H
