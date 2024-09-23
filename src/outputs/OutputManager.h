#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include "BaseOutput.h"

#include "spdlog/spdlog.h"

#include <QString>
#include <QObject>

#include <memory>
#include <vector>

class OutputManager: public QObject
{
    Q_OBJECT

public:

    OutputManager();
    bool LoadOutput(QString const& type, QString const& ipAddress, uint32_t const& start_universe, uint32_t const& start_channel, uint32_t const& universe_size);

    bool OpenOutputs();
    void CloseOutputs();
    void OutputData(uint8_t* data);

Q_SIGNALS:
    void AddController(bool enabled, QString const& type, QString const& ip, QString const& channels);
    void SetChannelCount(uint64_t channels);

private:
    std::unique_ptr<BaseOutput> m_output;
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };
};

#endif