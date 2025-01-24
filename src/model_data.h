#pragma once

#include "dmx_color.h"
#include "outputs/OutputManager.h"

#include "motor_data.h"
#include "data_point.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <QColor>
#include <QSharedPointer>

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

	QColor const& GetQColor()const { return m_last_color; };

	void AddPanTilt(int time_ms, double pan, double tilt, double pan_sen, double tilt_sen, int pan_off, int tilt_off);
	void CalcPanTilt(double pan, double tilt, double pan_sen, double tilt_sen, int pan_off, int tilt_off);
	void AddColor(int time_ms);
	void ClearData();
	void WriteXMLFile(QString const& xmlFileName) const;
	void ChangeColor(QColor color, uint8_t dimmer);
	void ChangeBrightness(uint8_t dimmer);

	void OpenModelFile(QString const& xmlFileName);

	void ChangeGobo(int diff);

	void ToggleBlur();
	void TogglePrism();
	void LampOn();
	void LampOff();

	uint32_t GetGoboChan() const { return gobo_chan; }
	uint32_t GetPrismChan() const { return prism_chan; }
	uint32_t GetBlurChan() const { return blur_chan; }
	uint32_t GetLampChan() const { return lamp_chan; }
	uint32_t GetDimmerChan() const { return dimmer_chan; }

Q_SIGNALS:
	void SetChannelData(uint32_t chan, uint8_t value);
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

	std::unique_ptr <MotorData> m_pan{nullptr};
	std::unique_ptr <MotorData> m_tilt{ nullptr };
	//std::unique_ptr < DmxColor> m_color{ nullptr };
	QSharedPointer < DmxColor> m_color{ nullptr };
	//uint8_t m_data[20];

	uint32_t gobo_chan{ 0u };
	std::vector <uint8_t> gobo_values{0u};
	int gobo_index{0};

	uint32_t prism_chan{ 0u };
	uint8_t prism_on_value{ 255u };
	uint8_t prism_off_value{ 0u };
	bool prism_on{ false };

	uint32_t blur_chan{ 0u };
	uint8_t blur_on_value{ 255u };
	uint8_t blur_off_value{ 0u };
	bool blur_on{ false };

	uint32_t lamp_chan{ 0u };
	uint8_t lamp_on_value{ 255u };
	uint8_t lamp_off_value{ 0u };
	uint32_t lamp_off_on_delay{ 5000u };

	uint32_t dimmer_chan{ 0u };

	OutputManager* m_out;
};

