#pragma once

#include "BaseOutput.h"

#include <QString>
#include <QtNetwork>

#include <memory>

struct IPOutput : BaseOutput
{
	IPOutput():m_UdpSocket(std::make_unique<QUdpSocket>()) {}
	std::unique_ptr<QUdpSocket> m_UdpSocket;
};
