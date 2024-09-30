#pragma once
#include "SerialOutput.h"

#include <QString>

#include <memory>

#define OPENDMX_MAX_CHANNELS 512

struct OpenDMXOutput : SerialOutput
{
	OpenDMXOutput();
	bool Open() override;
	void Close() override;
	void OutputFrame(uint8_t* data) override;
	QString GetName() const override { return "OpenDMX"; };

	uint8_t _data[OPENDMX_MAX_CHANNELS + 1];
};
