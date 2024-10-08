#pragma once
#include "BaseOutput.h"

#include <QString>
#include <QSerialPort>

#include <memory>

struct SerialOutput : BaseOutput
{
	SerialOutput()
		//:		m_SerialPort(std::make_unique<QSerialPort>())
	{
		m_SerialPort = std::make_unique<QSerialPort>();
	}
	std::unique_ptr<QSerialPort> m_SerialPort{nullptr};

	int BaudRate { 9600 };
};
