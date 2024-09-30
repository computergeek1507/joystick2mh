#pragma once

#include <QColor>
#include <QString>

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