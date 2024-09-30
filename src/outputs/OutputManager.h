#pragma once
#include "BaseOutput.h"

#include "spdlog/spdlog.h"

#include <QString>
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QSettings>

#include <memory>
#include <vector>

#define MAX_CHANNELS (512)

class OutputManager: public QObject
{
    Q_OBJECT

public:

    OutputManager();
    ~OutputManager();

    void ReadSettings(QSettings* sett);
    void SaveSettings(QSettings* sett);
    bool LoadOutput(QString const& type, QString const& ipAddress, uint32_t const& start_universe, uint32_t const& start_channel, uint32_t const& universe_size);

    bool OpenOutputs();
    void CloseOutputs();
    void OutputData(uint8_t* data);

    BaseOutput* const GetOutput() const { return m_output.get(); }

public Q_SLOTS:
    void TriggerTimedOutputData();
    void StopDataOut();
    void StartDataOut();
    void SetData(uint16_t chan, uint8_t value);

Q_SIGNALS:
    void AddController(bool enabled, QString const& type, QString const& ip, QString const& channels);
    void SetChannelCount(uint64_t channels);

private:
    std::unique_ptr<BaseOutput> m_output;
    std::shared_ptr<spdlog::logger> m_logger{ nullptr };

    std::unique_ptr<QTimer> m_playbackTimer{ nullptr };
    QThread m_playbackThread;
    int m_seqStepTime{ 50 };

    char m_seqData[MAX_CHANNELS];
};
