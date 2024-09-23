#include "OutputManager.h"

#include "ArtNetOutput.h"
#include "DDPOutput.h"
#include "E131Output.h"
#include "DMXOutput.h"
#include "OpenDMXOutput.h"

OutputManager::OutputManager():
	m_logger(spdlog::get("joystick2vc")),
	m_playbackTimer(std::make_unique<QTimer>(this))
{

	memset(m_seqData, 0, sizeof(m_seqData));

	//m_playbackTimer = std::make_unique<QTimer>(this);
	m_playbackTimer->setTimerType(Qt::PreciseTimer);
	m_playbackTimer->setInterval(50);

	//m_playbackThread = std::make_unique<QThread>(this);
	//moveToThread(&m_playbackThread);
	//m_playbackTimer->moveToThread(&m_playbackThread);
	//this->moveToThread(thread);
	m_playbackTimer->moveToThread(&m_playbackThread);

	connect(&m_playbackThread, SIGNAL(started()), m_playbackTimer.get(), SLOT(start()));
	connect(m_playbackTimer.get(), SIGNAL(timeout()), this, SLOT(TriggerOutputData()));
	connect(this, SIGNAL(finished()), m_playbackTimer.get(), SLOT(stop()));
	connect(this, SIGNAL(finished()), &m_playbackThread, SLOT(quit()));

}

OutputManager::~OutputManager()
{
	m_playbackTimer->stop();
	m_playbackThread.requestInterruption();
	m_playbackThread.quit();
	m_playbackThread.wait();
	//delete m_seqFile;
}

void OutputManager::TriggerTimedOutputData()
{
	OutputData((uint8_t*)m_seqData);
	//m_lastFrameData = m_seqFile->getFrame(m_lastFrameRead);
}

void OutputManager::StopDataOut()
{
	m_playbackTimer->stop();
	m_playbackThread.requestInterruption();
	m_playbackThread.quit();
	m_playbackThread.wait();


	//stop timer
		CloseOutputs();

}

void OutputManager::SetData(uint16_t chan, uint8_t value)
{
	m_seqData[chan - 1] = value;
}

void OutputManager::StartDataOut()
{
	m_playbackTimer->setInterval(m_seqStepTime);
	m_playbackThread.start();
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