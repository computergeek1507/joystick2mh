#pragma once

#include "dmx_color.h"
#include "outputs/OutputManager.h"

#include "motor_data.h"
#include "data_point.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <QColor>

#include <memory>
#include <vector>
#include <unordered_map>

struct ModelData : public QObject
{
	Q_OBJECT
public:
	ModelData(QSettings* sett, OutputManager* out);
	void ReadSettings(QSettings* sett);
	void SaveSettings(QSettings* sett) const;
	//~ModelData();

	std::vector<PTDataPoint> const& GetPanTiltValues() const { return m_pt_values;}

	MotorData* const GetPanMotor() const { return m_pan.get(); }
	MotorData* const GetTiltMotor() const { return m_tilt.get(); }
	DmxColor* const GetColor() const { return m_color.get(); }

	void AddPanTilt(int time_ms, double pan, double tilt, double pan_sen, double tilt_sen);
	void AddColor(int time_ms);
	void ClearData();
	void WriteXMLFile(QString const& xmlFileName) const;
	void ChangeColor(QColor color);

	void OpenModelFile(QString const& xmlFileName);

Q_SIGNALS:
	void SetChannelData(uint16_t chan, uint8_t value);
	void OnSetColor(QColor const& color);

private:
	DegreeStringData CreatePanTiltVCDate() const;
	DMXStringData CreatePanTiltDMXVCDate() const;
	void CalcPanTiltDMX(PTDataPoint & point);
	void SaveFile(QString const& type, QString const& data, QString const& xmlFileName) const;
	QString CreateColorVC() const;
	void SaveColorFile(QString const& type, QString const& xmlFileName) const;
	void WriteCmdToPixel(int value, MotorData* motor);
	//std::vector<WheelDataPoint> m_wheel_values;
	//std::vector<RGBDataPoint> m_rgb_values;

	std::vector <ColorDataPoint> m_color_values;
	std::vector <PTDataPoint> m_pt_values;
	QColor m_last_color{ Qt::black };

	std::unordered_map<QString,std::vector<GenericDMXPoint>> m_dmx_values;

	std::unique_ptr < MotorData> m_pan{nullptr};
	std::unique_ptr < MotorData> m_tilt{ nullptr };
	std::unique_ptr < DmxColor> m_color{ nullptr };
	uint8_t m_data[20];

	OutputManager* m_out;
};

