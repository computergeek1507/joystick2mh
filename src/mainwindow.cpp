#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include "config.h"

#include <QThread>
#include <QTimer>
#include <QLabel>

//#include <QNetworkAccessManager>
//#include <QNetworkReply>
#include <QStandardPaths>
//#include <QOperatingSystemVersion>

#include "spdlog/spdlog.h"

#include "spdlog/sinks/qt_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <filesystem>
#include <utility>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
	QCoreApplication::setApplicationName(PROJECT_NAME);
    QCoreApplication::setApplicationVersion(PROJECT_VER);
    m_ui->setupUi(this);

	auto const log_name{ "log.txt" };

	m_appdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	std::filesystem::create_directory(m_appdir.toStdString());
	QString logdir = m_appdir + "/log/";
	std::filesystem::create_directory(logdir.toStdString());

	try
	{
		auto file{ std::string(logdir.toStdString() + log_name) };
		auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( file, 1024 * 1024, 5, false);

		m_logger = std::make_shared<spdlog::logger>("joystick2vc", rotating);
		m_logger->flush_on(spdlog::level::debug);
		m_logger->set_level(spdlog::level::debug);
		m_logger->set_pattern("[%D %H:%M:%S] [%L] %v");
		spdlog::register_logger(m_logger);
	}
	catch (std::exception& /*ex*/)
	{
		QMessageBox::warning(this, "Logger Failed", "Logger Failed To Start.");
	}

	setWindowTitle(windowTitle() + " v" + PROJECT_VER);

	m_settings = std::make_unique< QSettings>(m_appdir + "/settings.txt", QSettings::IniFormat);

	m_output = std::make_unique< OutputManager>();
	m_model = std::make_unique< ModelData>(m_settings.get(), m_output.get());

	m_output->ReadSettings(m_settings.get());
	//connect(m_model.get(), &ModelData::SetChannelData, m_output.get(), &OutputManager::SetData);
	//connect(m_model.get(), &ModelData::SetChannelData, m_output.get(), &OutputManager::SetData);

	onUpdateSettingsGUI();

	connect(m_model.get(), &ModelData::OnSetColor, this, &MainWindow::onUpdateColor);

	//m_colorLabel = new QLabel(this);
	//m_ui->statusbar->addWidget(m_colorLabel, 1);

	m_controllerReader = new QTimer (this);
	connect(m_controllerReader, &QTimer::timeout, this, &MainWindow::ReadJoystick);

	m_ui->valuesPlot->plotLayout()->insertRow(0);
	m_ui->valuesPlot->plotLayout()->addElement(0, 0, new QCPTextElement(m_ui->valuesPlot, "", QFont("sans", 12, QFont::Bold)));

	m_ui->valuesPlot->setInteraction(QCP::iRangeDrag, true);
	m_ui->valuesPlot->setInteraction(QCP::iRangeZoom, true);

	QTimer::singleShot(500, this, SLOT(LoadControllers()));
}

MainWindow::~MainWindow()
{
	m_model->SaveSettings(m_settings.get());
	m_output->SaveSettings(m_settings.get());
	m_settings->sync();
	delete m_ui;
}

void MainWindow::LoadControllers()
{
	QGamepadManager* gamepad_manager = QGamepadManager::instance();
	QList<int> gamepads;
	int i = 0;
	while (i < 10)
	{
		QCoreApplication ::processEvents();
		//qInfo() << "get connected gamepads iteration : " << i;
		gamepads = gamepad_manager->connectedGamepads();
		if (!gamepads.isEmpty())
		{
			i = 10;
		}
		i++;
	}
	if (!gamepads.isEmpty())
	{
		m_gamepad = std::make_unique < QGamepad>(*gamepads.begin(), this);
		connect(m_gamepad.get(), &QGamepad::buttonAChanged, this, [&](bool pressed) {
			if(pressed) m_model->ChangeColor(Qt::green);
			});
		connect(m_gamepad.get(), &QGamepad::buttonBChanged, this, [&](bool pressed) {
			if (pressed) m_model->ChangeColor(Qt::red);
			});
		connect(m_gamepad.get(), &QGamepad::buttonXChanged, this, [&](bool pressed) {
			if (pressed) m_model->ChangeColor(Qt::blue);
			});
		connect(m_gamepad.get(), &QGamepad::buttonYChanged, this, [&](bool pressed) {
			if (pressed) m_model->ChangeColor(Qt::yellow);
			});

		connect(m_gamepad.get(), &QGamepad::buttonStartChanged, this, [&](bool pressed) {
			if (pressed) { 
				!m_recording ? on_pushButtonStart_clicked() : on_pushButtonStop_clicked(); }
			});

		connect(m_gamepad.get(), &QGamepad::buttonSelectChanged, this, [&](bool pressed) {
			if (pressed) on_pushButtonReset_clicked();
			});

		connect(m_gamepad.get(), &QGamepad::buttonR1Changed, this, [&](bool pressed) {
			if (pressed) m_model->ChangeColor(Qt::black);
			});
		connect(m_gamepad.get(), &QGamepad::buttonR2Changed, this, [&](bool pressed) {
			if (pressed) m_model->ChangeColor(Qt::white);
			});

		connect(m_gamepad.get(), &QGamepad::connectedChanged, this, [&](bool connected) {
			if (!connected) 
			{ 
				on_pushButtonStop_clicked(); 
				LogMessage("Controller Disconnected", spdlog::level::warn);
			}
			});
	}
	else 
	{
		LogMessage("No Controller Found",spdlog::level::warn);
		QMessageBox::warning(this, "No Controller", "No Controller Found");
	}
}

void MainWindow::on_actionImport_Model_triggered()
{
	QString const filename = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + QDir::separator() + "ValueCurve.xvc";

	QString const path = QFileDialog::getOpenFileName(this, tr("Open Model Files"), filename, tr("Model Files (*.xmodel);;All files (*.*)"));
	if (path.isEmpty())
	{
		return;
	}
	m_model->OpenModelFile(path);
	RedrawModelSettings();
}

void MainWindow::on_actionSave_X_triggered()
{
	QString const filename = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + QDir::separator() + "ValueCurve.xvc";

	QString const path = QFileDialog::getSaveFileName(this, tr("Save ValueCurve"), filename, tr("ValueCurve Files (*.xvc);;All files (*.*)"));
	if (path.isEmpty())
	{
		return;
	}
	m_model->WriteXMLFile(path);
}

void MainWindow::on_actionSave_Y_triggered() 
{

}

void MainWindow::on_actionClose_triggered()
{
	close();
}

void MainWindow::on_actionAbout_triggered()
{
	QString text = QString("Joystick 2 MovingHead v%1<br>QT v%2<br><br>Icons by:")
		.arg(PROJECT_VER, QT_VERSION_STR) +
		QStringLiteral("<br><a href='http://www.famfamfam.com/lab/icons/silk/'>www.famfamfam.com</a>");
		//http://www.famfamfam.com/lab/icons/silk/
	QMessageBox::about( this, "About Joystick 2 MovingHead", text );
}

void MainWindow::on_actionOpen_Logs_triggered()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(m_appdir + "/log/"));
}

void MainWindow::on_pushButtonStart_clicked() 
{
	m_recording = true;
	m_ui->pushButtonStop->setEnabled(true);
	m_ui->pushButtonStart->setEnabled(false);
	m_controllerReader->start(m_ui->spinBoxDelay->value());
}

void MainWindow::on_pushButtonStop_clicked() 
{
	m_recording = false;
	m_controllerReader->stop();
	m_ui->pushButtonStop->setEnabled(false);
	m_ui->pushButtonStart->setEnabled(true);
}

void MainWindow::on_pushButtonReset_clicked()
{
	m_model->ClearData();
	DrawPlot();
}

void MainWindow::on_checkBoxOutput_stateChanged(int state)
{
	if (m_output)
	{
		if (state)
		{
			m_output->OpenOutputs();
			m_output->StartDataOut();
		}
		else 
		{
			m_output->StopDataOut();
		}
	}
}

void MainWindow::ReadJoystick()
{
	if (m_gamepad) 
	{
		m_model->AddPanTilt(m_ui->spinBoxDelay->value(), m_gamepad->axisLeftX(), -m_gamepad->axisLeftY() );
		m_model->AddColor(m_ui->spinBoxDelay->value());
	}
	DrawPlot();
}

void MainWindow::LogMessage(QString const& message, spdlog::level::level_enum llvl)
{
	m_logger->log(llvl, message.toStdString());
}

void MainWindow::DrawPlot()
{
	m_ui->valuesPlot->yAxis->setRange(-260.0, 260.0);
	auto const& panTilt = m_model->GetPanTiltValues();
	QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
	m_ui->valuesPlot->xAxis->setTicker(fixedTicker);
	m_ui->valuesPlot->xAxis->setLabel("Points");
	m_ui->valuesPlot->xAxis->setRange(0, (panTilt.size() + 1));
	m_ui->valuesPlot->xAxis2->setVisible(true);
	m_ui->valuesPlot->xAxis->setVisible(false);
	m_ui->valuesPlot->xAxis2->setTicks(false);
	m_ui->valuesPlot->xAxis2->setTickLabels(false);
	
	m_ui->valuesPlot->yAxis->grid()->setSubGridVisible(true);
	m_ui->valuesPlot->clearGraphs();

	int offset = 0;
	auto AddTesterPointGraph = [&](std::vector<PTDataPoint> const& data)
		{
			int size = data.size();
			QVector<double> x1(size), y1(size);
			QVector<double> x2(size), y2(size);

			QVector<double> a1(size), b1(size);
			QVector<double> a2(size), b2(size);

			for (int i = 0; i < size; ++i)
			{
				x1[i] = offset + i + 1;
				y1[i] = data[i].pan;
				x2[i] = offset + i + 1;
				y2[i] = data[i].tilt;

				a1[i] = offset + i + 1;
				b1[i] = data[i].pan_coarse_dmx;
				a2[i] = offset + i + 1;
				b2[i] = data[i].tilt_coarse_dmx;
			}
			auto ngr = m_ui->valuesPlot->addGraph();
			ngr->setName("Pan");
			ngr->setPen(QPen(Qt::red));
			ngr->setLineStyle(QCPGraph::lsLine);
			ngr->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
			ngr->setData(x1, y1);

			auto ngr2 = m_ui->valuesPlot->addGraph();
			ngr2->setName("Tilt");
			ngr2->setPen(QPen(Qt::blue));
			ngr2->setLineStyle(QCPGraph::lsLine);
			ngr2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
			ngr2->setData(x2, y2);

			auto ngr3 = m_ui->valuesPlot->addGraph();
			ngr3->setName("Pan DMX");
			ngr3->setPen(QPen(Qt::green));
			ngr3->setLineStyle(QCPGraph::lsLine);
			ngr3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
			ngr3->setData(a1, b1);

			auto ngr4 = m_ui->valuesPlot->addGraph();
			ngr4->setName("Tilt DMX");
			ngr4->setPen(QPen(Qt::yellow));
			ngr4->setLineStyle(QCPGraph::lsLine);
			ngr4->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
			ngr4->setData(a2, b2);
			offset += size;
		};
	AddTesterPointGraph(panTilt);
	m_ui->valuesPlot->legend->setVisible(true);
	m_ui->valuesPlot->replot();
}

void MainWindow::onUpdateColor(QColor const& color)
{
	QPixmap pix(40, 20);
	pix.fill(color);
	//m_colorLabel->setPixmap(pix);
	m_ui->labelColor->setPixmap(pix);
	//QString trc = QString("background-color: %1; border-radius: 10px;").arg(color.name());
	//m_colorLabel->setStyleSheet(trc);
}

void MainWindow::onUpdateSettingsGUI()
{
	m_ui->pushButtonA->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::green).name()));
	m_ui->pushButtonB->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::red).name()));
	m_ui->pushButtonX->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::blue).name()));
	m_ui->pushButtonY->setStyleSheet(QString("background-color: %1").arg(QColor(Qt::yellow).name()));

	if (m_output && m_output->GetOutput())
	{
		int idx = m_ui->comboBoxOutputType->findText(m_output->GetOutput()->GetName());
		if (-1 != idx) 
		{
			m_ui->comboBoxOutputType->setCurrentIndex(idx);
		}
		m_ui->lineEditIPAddress->setText(m_output->GetOutput()->IP);
		//m_ui->spinBoxStartUniverse->setValue(m_output->GetOutput()->Sta);
		m_ui->spinBoxStartChannel->setValue(m_output->GetOutput()->StartChannel);
		m_ui->spinBoxSize->setValue(m_output->GetOutput()->Channels);
	}
	m_ui->comboBoxColorMode->setCurrentIndex(0);


	RedrawModelSettings();
}

void MainWindow::RedrawModelSettings()
{
	if (m_model && m_model->GetPanMotor())
	{
		m_ui->spinBoxPanChannel->setValue(m_model->GetPanMotor()->channel_coarse);
		m_ui->spinBoxPanFineChannel->setValue(m_model->GetPanMotor()->channel_fine);
		m_ui->spinBoxPanMin->setValue(m_model->GetPanMotor()->min_limit);
		m_ui->spinBoxPanMax->setValue(m_model->GetPanMotor()->max_limit);
		m_ui->spinBoxPanRange->setValue(m_model->GetPanMotor()->range_of_motion);
		m_ui->spinBoxPanZero->setValue(m_model->GetPanMotor()->orient_zero);
		m_ui->spinBoxPanForward->setValue(m_model->GetPanMotor()->orient_home);
		m_ui->checkBoxPanReverse->setChecked(m_model->GetPanMotor()->reverse);
	}
	if (m_model && m_model->GetTiltMotor())
	{
		m_ui->spinBoxTiltChannel->setValue(m_model->GetTiltMotor()->channel_coarse);
		m_ui->spinBoxTiltFineChannel->setValue(m_model->GetTiltMotor()->channel_fine);
		m_ui->spinBoxTiltMin->setValue(m_model->GetTiltMotor()->min_limit);
		m_ui->spinBoxTiltMax->setValue(m_model->GetTiltMotor()->max_limit);
		m_ui->spinBoxTiltRange->setValue(m_model->GetTiltMotor()->range_of_motion);
		m_ui->spinBoxTiltZero->setValue(m_model->GetTiltMotor()->orient_zero);
		m_ui->spinBoxTiltUp->setValue(m_model->GetTiltMotor()->orient_home);
		m_ui->checkBoxTiltReverse->setChecked(m_model->GetTiltMotor()->reverse);
	}

	if (m_model && m_model->GetColor())
	{
		auto type = std::to_underlying(m_model->GetColor()->GetColorType());

		switch (m_model->GetColor()->GetColorType())
		{
			case DmxColorType::RGB: 
			{
				m_ui->comboBoxColorMode->setCurrentIndex(0);
				auto rgbc = dynamic_cast<DmxColorRGB*>(m_model->GetColor());
				m_ui->spinBoxColorR->setValue(rgbc->red_channel);
				m_ui->spinBoxColorG->setValue(rgbc->green_channel);
				m_ui->spinBoxColorB->setValue(rgbc->blue_channel);
				m_ui->spinBoxColorW->setValue(rgbc->white_channel);
			}
			break;
			case DmxColorType::Wheel:
			{
				m_ui->comboBoxColorMode->setCurrentIndex(1);
				auto wheelc = dynamic_cast<DmxColorWheel*>(m_model->GetColor());
				m_ui->spinBoxWheelDimmer->setValue(wheelc->dimmer_channel);
				m_ui->spinBoxWheelChannel->setValue(wheelc->wheel_channel);
			}
			break;
		}
	}
}

void MainWindow::OpenFile(QString const& path)
{
#if defined( Q_OS_WIN )
	static const QString notepadpp_x64Path = R"(C:\Program Files\Notepad++\notepad++.exe)";
	static const QString notepadpp_x32Path = R"(C:\Program Files (x86)\Notepad++\notepad++.exe)";
	if (QFile::exists(notepadpp_x64Path))
	{
		QProcess::startDetached(notepadpp_x64Path, QStringList() << path);
		return;
	}
	else if (QFile::exists(notepadpp_x32Path))
	{
		QProcess::startDetached(notepadpp_x32Path, QStringList() << path);
		return;
	}
	else
#endif
	{
		QDesktopServices::openUrl(QUrl("File:///" + path, QUrl::TolerantMode));
	}
}
