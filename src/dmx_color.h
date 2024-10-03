#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QColor>

#include <memory>
#include <vector>
#include <optional>

enum class DmxColorType : int { RGB = 0 , Wheel = 1, Unknown = -1 };

struct DmxColor : public QObject
{
    Q_OBJECT
public:
	DmxColor()
	{}

    virtual ~DmxColor() = default;
	
	virtual void SetColorPixels(const QColor& color) = 0;

    virtual DmxColorType GetColorType() const = 0;

    virtual void ReadSettings(QSettings* sett) = 0;
    virtual void SaveSettings(QSettings* sett) const = 0;

    [[nodiscard]] bool CheckChannel(uint32_t chan) const
    {
        return chan > 0 && 20 > chan;
    };

    uint32_t shutter_channel{ 0u };
    uint8_t shutter_on_value{ 255u };


Q_SIGNALS:
    void SetChannelData(uint16_t chan, uint8_t value);

protected:
    void SaveBaseSettings(QSettings* sett) const
    {
        sett->setValue("shutter_channel", shutter_channel);
        sett->setValue("shutter_on_value", shutter_on_value);
    }

    void ReadBaseSettings(QSettings* sett)
    {
        shutter_channel = sett->value("shutter_channel", 0u).toUInt();
        shutter_on_value = sett->value("shutter_on_value", 255u).toUInt();
    }    
};

struct DmxColorRGB : public DmxColor
{
    DmxColorRGB() :
        DmxColor()
    {
	}
	
    uint32_t red_channel{ 0u };
    uint32_t green_channel{ 0u };
    uint32_t blue_channel{ 0u };
    uint32_t white_channel{ 0u };

    DmxColorType GetColorType() const override { return DmxColorType::RGB;};

    void SetColorPixels(const QColor& color) override
    {
        if (CheckChannel(white_channel)
            && color.red() == color.green() && color.red() == color.blue()) 
        {
           // m_data[white_channel - 1] = color.red();
            emit SetChannelData(white_channel, color.red());
        }
        else
        {
            if (CheckChannel(red_channel ) )
            {
                //m_data[red_channel - 1] = color.red();
                emit SetChannelData(red_channel, color.red());
            }
            if (CheckChannel(green_channel))
            {
                //m_data[green_channel - 1] = color.green();
                emit SetChannelData(green_channel, color.green());
            }
            if (CheckChannel(blue_channel))
            {
                //m_data[blue_channel - 1] = color.blue();
                emit SetChannelData(blue_channel, color.blue());
            }
        }

        if (CheckChannel(shutter_channel))
        {
            emit SetChannelData(shutter_channel, shutter_on_value);
        }
    }

    void ReadSettings(QSettings* sett) override
    {
        red_channel = sett->value("red_channel", 0u).toUInt();
        green_channel = sett->value("green_channel", 0u).toUInt();
        blue_channel = sett->value("blue_channel", 0u).toUInt();
        white_channel = sett->value("white_channel", 0u).toUInt();
        ReadBaseSettings(sett);
    }
    void SaveSettings(QSettings* sett) const override 
    {
        sett->setValue("mode", std::to_underlying(GetColorType()));
        sett->setValue("red_channel", red_channel);
        sett->setValue("green_channel", green_channel);
        sett->setValue("blue_channel", blue_channel);
        sett->setValue("white_channel", white_channel);
        SaveBaseSettings(sett);
    }
};

struct WheelColor
{
    WheelColor(QColor col, uint8_t value) :
        color(std::move(col)), dmxValue(value)
    { }
    QColor color;
    uint8_t dmxValue{ 0u };
};

struct DmxColorWheel : public DmxColor
{
    DmxColorWheel() :
        DmxColor()
    {
	}
	
    uint32_t wheel_channel{ 0u };
	uint32_t dimmer_channel{ 0u };
	std::vector<WheelColor> colors;

    DmxColorType GetColorType() const override { return DmxColorType::Wheel; };

    std::optional<uint8_t> GetDMXWheelValue(QColor const& color) const
    {
        if (auto const found{ std::find_if(colors.begin(), colors.end(),
                                           [&color](auto const& col)
            {
                //return color == col.color;
                return (std::abs(color.hue() - col.color.hue()) < 1);
            }) };
            found != colors.end()) {
            uint8_t dmxV{ (*found).dmxValue };
            return dmxV;
        }
        return std::nullopt;
    }

    void SetColorPixels(const QColor& color) override
    {
        if (auto const& colordata = GetDMXWheelValue(color); colordata)
        {
            if (CheckChannel(wheel_channel)) 
            {
                //m_data[wheel_channel - 1] = colordata.value();
                emit SetChannelData(wheel_channel, colordata.value());
            }
            if (CheckChannel(dimmer_channel))
            {
                int intensity = color.toHsv().value();
                //int intensity = (hsv * 255.0);
                //m_data[dimmer_channel - 1] = intensity;
                emit SetChannelData(dimmer_channel, intensity);
            }
        }
        else
        {
            if (CheckChannel(wheel_channel))
            {
                //m_data[wheel_channel - 1] = 0;
                emit SetChannelData(wheel_channel, 0);
            }
            if (CheckChannel(dimmer_channel))
            {
                //m_data[dimmer_channel - 1] = 0;
                emit SetChannelData(dimmer_channel, 0);
            }
        }

        if (CheckChannel(shutter_channel))
        {
            emit SetChannelData(shutter_channel, shutter_on_value);
        }
    }

    void ReadSettings(QSettings* sett) override
    {
        wheel_channel = sett->value("wheel_channel", 0u).toUInt();
        dimmer_channel = sett->value("dimmer_channel", 0u).toUInt();
        QStringList colorsStr = sett->value("color_wheel").toStringList();
        colors.clear();
        for (auto const& col : colorsStr)
        {
            if (col.contains(':')) 
            {
                auto datastr = col.split(':');
                auto dmx = datastr[1].toUInt();
                colors.emplace_back(QColor(datastr[0]), dmx);
            }
        }
        ReadBaseSettings(sett);
    }

    void SaveSettings(QSettings* sett) const override 
    {
        sett->setValue("mode", std::to_underlying(GetColorType()));
        sett->setValue("wheel_channel", wheel_channel);
        sett->setValue("dimmer_channel", dimmer_channel);
        QStringList colorsStr;
        for (auto const& col : colors)
        {
            colorsStr.append(QString("%1:%2").arg(col.color.name()).arg(col.dmxValue));
        }
        sett->setValue("color_wheel", colorsStr);
        SaveBaseSettings(sett);
    }
};
