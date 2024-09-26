#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QColor>

#include <memory>
#include <vector>
#include <optional>

struct DmxColor {
	DmxColor()
	{}

    virtual ~DmxColor() = default;
	
	virtual void SetColorPixels(const QColor& color, uint8_t* m_data) const = 0;

    virtual void ReadSettings(QSettings* sett) = 0;
    virtual void SaveSettings(QSettings* sett) const = 0;

    [[nodiscard]] bool CheckChannel(uint32_t chan) const
    {
        return chan > 0 && 20 > chan;
    };
};

struct DmxColorRGB : public DmxColor
{
    DmxColorRGB() :
        DmxColor()
    {
	}
	
    uint32_t red_channel{ 0 };
    uint32_t green_channel{ 0 };
    uint32_t blue_channel{ 0 };
    uint32_t white_channel{ 0 };

    void SetColorPixels(const QColor& color, uint8_t* m_data) const override
    {
        if (CheckChannel(white_channel)
            && color.red() == color.green() && color.red() == color.blue()) {
            m_data[white_channel - 1] = color.red();
        }
        else {
            if (CheckChannel(red_channel ) ){
                m_data[red_channel - 1] = color.red();
            }
            if (CheckChannel(green_channel)) {
                m_data[green_channel - 1] = color.green();
            }
            if (CheckChannel(blue_channel)) {
                m_data[blue_channel - 1] = color.blue();
            }
        }
    }

    void ReadSettings(QSettings* sett) override
    {
        red_channel = sett->value("red_channel", 0).toUInt();
        green_channel = sett->value("green_channel", 0).toUInt();
        blue_channel = sett->value("blue_channel", 0).toUInt();
        white_channel = sett->value("white_channel", 0).toUInt();
    }
    void SaveSettings(QSettings* sett) const override 
    {
        sett->setValue("mode", 0);
        sett->setValue("red_channel", red_channel);
        sett->setValue("green_channel", green_channel);
        sett->setValue("blue_channel", blue_channel);
        sett->setValue("white_channel", white_channel);
    }
};

struct WheelColor
{
    WheelColor(QColor col, uint8_t value) :
        color(std::move(col)), dmxValue(value)
    { }
    QColor color;
    uint8_t dmxValue{ 0 };
};

struct DmxColorWheel : public DmxColor
{
    DmxColorWheel() :
        DmxColor()
    {
	}
	
    uint32_t wheel_channel{0};
	uint32_t dimmer_channel{ 0 };
	std::vector<WheelColor> colors;

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

    void SetColorPixels(const QColor& color, uint8_t* m_data) const override
    {
        if (auto const& colordata = GetDMXWheelValue(color); colordata) {
            if (CheckChannel(wheel_channel)) {
                m_data[wheel_channel - 1] = colordata.value();
            }
            if (CheckChannel(dimmer_channel)) {
                int intensity = color.toHsv().value();
                //int intensity = (hsv * 255.0);
                m_data[dimmer_channel - 1] = intensity;
            }
        }
        else {
            if (CheckChannel(wheel_channel)) {
                m_data[wheel_channel - 1] = 0;
            }
            if (CheckChannel(dimmer_channel)) {
                m_data[dimmer_channel - 1] = 0;
            }
        }
    }

    void ReadSettings(QSettings* sett) override
    {
        wheel_channel = sett->value("wheel_channel", 0).toUInt();
        dimmer_channel = sett->value("dimmer_channel", 0).toUInt();
    }
    void SaveSettings(QSettings* sett) const override 
    {
        sett->setValue("mode", 1);
        sett->setValue("wheel_channel", wheel_channel);
        sett->setValue("dimmer_channel", dimmer_channel);
        QStringList colorsStr;
        for (auto const& col : colors) {
            colorsStr.append(QString("%1:%2").arg(col.color.rgb()).arg(col.dmxValue));
        }
        sett->setValue("color_wheel", colorsStr);
    }
};



