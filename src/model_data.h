#pragma once

#include "dmx_color.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <QColor>

#include <memory>
#include <vector>

const int min_value{ 0 };
const int max_value{ 65535 };

struct PTDataPoint {
	PTDataPoint()
	{}
	PTDataPoint(int ms, double p ,double t) :
		tilt(t),
		pan(p),
		time_ms(ms)
	{}
	double tilt{ 0.0 };
	double pan{ 0.0 };
	uint8_t tilt_dmx{ 0 };
	uint8_t pan_dmx{ 0 };
	int time_ms{ 0 };
};

struct RGBDataPoint {
	double r{ 0.0 };
	double g{ 0.0 };
	double b{ 0.0 };
	double w{ 0.0 };
	int time_ms{ 0 };
};

struct WheelDataPoint {
	double wheel{ 0.0 };
	double dimmer{ 0.0 };
	int time_ms{ 0 };
};

struct ColorDataPoint {
	ColorDataPoint()
	{}
	ColorDataPoint(int ms, QColor const& co) :
		color(co),
		time_ms(ms)
	{}
	QColor color;
	int time_ms{ 0 };
};

struct MotorData {
	int channel_coarse{ 0 };
	int channel_fine{ 0 };
	int min_limit{ -180 };
	int max_limit{ 180 };
	double range_of_motion{ 180.0 };
	int orient_zero{ 0 };
	int orient_home{ 0 };
	bool reverse{ false };
	int ConvertPostoCmd(float position) const;
};

struct ModelData : public QObject
{
	Q_OBJECT
public:
	ModelData(QSettings * sett);
	void ReadSettings(QSettings* sett);
	void SaveSettings(QSettings* sett) const;
	//~ModelData();
	std::tuple<QString, QString> CreatePanTiltVCDate() const;
	//QString CreatePanVCDate() const;
	//QString CreateTiltVCDate() const;

	std::vector<PTDataPoint> const& GetPanTiltValues() const { return m_pt_values;}

	void AddPanTilt(int time_ms, double pan, double tilt);
	void AddColor(int time_ms);
	void ClearData();
	void WriteXMLFile(QString const& xmlFileName) const;
	void ChangeColor(QColor color);

	void OpenModelFile(QString const& xmlFileName);

Q_SIGNALS:
	void SetChannelData(uint16_t chan, uint8_t value);

private:
	void CalcPanTiltDMX(PTDataPoint & point);
	void SaveFile(QString const& type, QString const& data, QString const& xmlFileName) const;
	void WriteCmdToPixel(int value, MotorData* motor);
	//std::vector<WheelDataPoint> m_wheel_values;
	//std::vector<RGBDataPoint> m_rgb_values;

	std::vector <ColorDataPoint> m_color_values;
	std::vector<PTDataPoint> m_pt_values;
	QColor m_last_color{ Qt::black };

	std::unique_ptr < MotorData> m_pan{nullptr};
	std::unique_ptr < MotorData> m_tilt{ nullptr };
	std::unique_ptr < DmxColor> m_color{ nullptr };
	uint8_t m_data[20];
};

