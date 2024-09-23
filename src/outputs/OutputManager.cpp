#include "OutputManager.h"

#include "ArtNetOutput.h"
#include "DDPOutput.h"
#include "E131Output.h"
#include "DMXOutput.h"
#include "OpenDMXOutput.h"

OutputManager::OutputManager():
	m_logger(spdlog::get("joystick2vc"))
{
}

bool OutputManager::OpenOutputs()
{
	//for (auto const& o : m_outputs)
	//{
		m_output->Open();
	//}
	return true;
}

void OutputManager::CloseOutputs()
{
	//for (auto const& o : m_outputs)
	//{
		m_output->Close();
	//}
}

void OutputManager::OutputData(uint8_t* data)
{
	//TODO: multithread
	//for (auto const& o : m_outputs)
	//{
		m_output->OutputFrame(data);
	//}
}

bool OutputManager::LoadOutput(QString const& type, QString const& ipAddress, uint32_t const& start_universe, uint32_t const& start_channel, uint32_t const& universe_size)
{

	if ("DDP" == type)
	{
		auto ddp = std::make_unique<DDPOutput>();
		ddp->IP = ipAddress;
		ddp->PacketSize = 1400;
		ddp->KeepChannels = true;
		ddp->StartChannel = start_channel;
		ddp->Channels = universe_size;
		m_output = (std::move(ddp));
		//emit AddController(active, nType, ipAddress, sChannels);
	}
	else if ("E131" == type)
	{
		auto e131 = std::make_unique<E131Output>();
		e131->IP = ipAddress;
		e131->PacketSize = universe_size;
		e131->Universe = start_universe;
		e131->StartChannel = start_channel;
		e131->Channels = universe_size;//todo fix
		m_output = (std::move(e131));
		//emit AddController(active, nType, ipAddress, sChannels);
	}
	else if ("ArtNet" == type)
	{
		auto artnet = std::make_unique<ArtNetOutput>();
		artnet->IP = ipAddress;
		artnet->PacketSize = universe_size;
		artnet->Universe = start_universe;
		artnet->StartChannel = start_channel;
		artnet->Channels = universe_size;//todo fix
		m_output = (std::move(artnet));
		//emit AddController(active, nType, ipAddress, sChannels);
	}
	else if ("DMX" == type)
	{
		auto dmx = std::make_unique<DMXOutput>();
		dmx->IP = ipAddress;
		dmx->BaudRate = start_universe;
		dmx->StartChannel = start_channel;
		dmx->Channels = universe_size;//todo fix
		m_output = (std::move(dmx));
		//emit AddController(active, nType, ipAddress, sChannels);
	}
	else if ("OpenDMX" == type)
	{
		auto opendmx = std::make_unique<OpenDMXOutput>();
		opendmx->IP = ipAddress;
		opendmx->BaudRate = start_universe;
		opendmx->StartChannel = start_channel;
		opendmx->Channels = universe_size;//todo fix
		m_output = (std::move(opendmx));
		//emit AddController(active, nType, ipAddress, sChannels);
	}
			
	else
	{
		m_logger->warn("Unsupported output type: {}",type.toStdString());
		//unsupported type
	}


	//emit SetChannelCount(startChannel - 1);
	return true;
}