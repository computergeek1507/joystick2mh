#pragma once

#include <QColor>
#include <QString>

struct DMXStringData 
{
	DMXStringData() {};
	DMXStringData(QString const& pc, QString const& pf,
		QString const& tc, QString const& tf ) :
		pan_coarse(pc),
		pan_fine(pf),
		tilt_coarse(tc),
		tilt_fine(tf)
	{};
	QString tilt_coarse;
	QString tilt_fine;
	QString pan_coarse;
	QString pan_fine;
};

struct DegreeStringData
{
	DegreeStringData() {};
	DegreeStringData(QString const& p, QString const& t) :
		pan(p),
		tilt(t)
	{};
	QString pan;
	QString tilt;
};

struct PTDataPoint {
	PTDataPoint()
	{}
	PTDataPoint(int ms, double p, double t) :
		tilt(t),
		pan(p),
		time_ms(ms)
	{}
	double tilt{ 0.0 };
	double pan{ 0.0 };
	uint8_t tilt_coarse_dmx{ 0 };
	uint8_t pan_coarse_dmx{ 0 };
	uint8_t tilt_fine_dmx{ 0 };
	uint8_t pan_fine_dmx{ 0 };
	int time_ms{ 0 };
};

struct RGBDataPoint {
	double r{ 0.0 };
	double g{ 0.0 };
	double b{ 0.0 };
	double w{ 0.0 };
	int time_ms{ 0 };
};

struct GenericDMXPoint {
	GenericDMXPoint(int ms, uint8_t dmx_) :
		dmx(dmx_),
		time_ms(ms)
	{}
	uint8_t dmx{ 0 };
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

	QString AsQString() const;
};