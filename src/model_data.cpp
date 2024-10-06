#include "model_data.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>
#include <QXmlStreamReader>

ModelData::ModelData(QSettings* sett, OutputManager* out):
	m_out(out)
{
	//memset(m_data, 0x00, sizeof(m_data));
	m_pan = std::make_unique<MotorData>();
	m_tilt = std::make_unique<MotorData>();
	ReadSettings(sett);

	connect(this, &ModelData::SetChannelData, out, &OutputManager::SetData);
	if (m_color) {
		//connect(m_color.get(), &DmxColor::SetChannelData, out, &OutputManager::SetData, Qt::UniqueConnection);
	}
}

void ModelData::ReadSettings(QSettings* sett)
{
	sett->beginGroup("tilt");
	m_tilt->channel_coarse = sett->value("channel_coarse", 0).toUInt();
	m_tilt->channel_fine = sett->value("channel_fine", 0).toUInt();
	m_tilt->min_limit = sett->value("min_limit", -180).toInt();
	m_tilt->max_limit = sett->value("max_limit", 180).toInt();
	m_tilt->range_of_motion = sett->value("range_of_motion", 150.0).toFloat();
	m_tilt->orient_zero = sett->value("orient_zero", 0).toInt();
	m_tilt->orient_home = sett->value("orient_home", 90).toInt();
	m_tilt->reverse = sett->value("reverse", false).toBool();
	sett->endGroup();

	sett->beginGroup("pan");
	m_pan->channel_coarse = sett->value("channel_coarse", 0).toUInt();
	m_pan->channel_fine = sett->value("channel_fine", 0).toUInt();
	m_pan->min_limit = sett->value("min_limit", -180).toInt();
	m_pan->max_limit = sett->value("max_limit", 180).toInt();
	m_pan->range_of_motion = sett->value("range_of_motion", 540.0).toFloat();
	m_pan->orient_zero = sett->value("orient_zero", 0).toInt();
	m_pan->orient_home = sett->value("orient_home", 90).toInt();
	m_pan->reverse = sett->value("reverse", false).toBool();
	sett->endGroup();

	sett->beginGroup("gobo");
	gobo_chan = sett->value("channel", 0).toUInt();
	auto values = sett->value("values", 0).toList();
	gobo_values.clear();
	for (auto val: values) 
	{
		gobo_values.push_back(val.toUInt());
	}
	sett->endGroup();

	sett->beginGroup("color");
	DmxColorType col_mode = static_cast<DmxColorType>(sett->value("mode", 0).toInt());

	switch (col_mode) {
	case DmxColorType::RGB:
		if (m_color) {
			disconnect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData);
		}
		m_color = QSharedPointer<DmxColorRGB>(new DmxColorRGB);
		m_color->ReadSettings(sett);
		connect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData, Qt::UniqueConnection);
		break;
	case DmxColorType::Wheel:
		if (m_color) {
			disconnect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData);
		}
		m_color = QSharedPointer<DmxColorWheel>(new DmxColorWheel);
		m_color->ReadSettings(sett);
		connect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData, Qt::UniqueConnection);
		break;

	}
	sett->endGroup();
}

void ModelData::SaveSettings(QSettings* sett) const
{
	sett->beginGroup("tilt");
	sett->setValue("channel_coarse", m_tilt->channel_coarse);
	sett->setValue("channel_fine", m_tilt->channel_fine);
	sett->setValue("min_limit", m_tilt->min_limit);
	sett->setValue("max_limit", m_tilt->max_limit);
	sett->setValue("range_of_motion", m_tilt->range_of_motion);
	sett->setValue("orient_zero", m_tilt->orient_zero);
	sett->setValue("orient_home", m_tilt->orient_home);
	sett->setValue("reverse", m_tilt->reverse);
	sett->endGroup();

	sett->beginGroup("pan");
	sett->setValue("channel_coarse", m_pan->channel_coarse);
	sett->setValue("channel_fine", m_pan->channel_fine);
	sett->setValue("min_limit", m_pan->min_limit);
	sett->setValue("max_limit", m_pan->max_limit);
	sett->setValue("range_of_motion", m_pan->range_of_motion);
	sett->setValue("orient_zero", m_pan->orient_zero);
	sett->setValue("orient_home", m_pan->orient_home);
	sett->setValue("reverse", m_pan->reverse);
	sett->endGroup();

	sett->beginGroup("color");
	m_color->SaveSettings(sett);
	sett->endGroup();
}

void ModelData::ClearData()
{
	//m_wheel_values.clear();
	//m_rgb_values.clear();
	m_color_values.clear();
	m_pt_values.clear();
	m_last_color = QColor(Qt::black);
}

//-1.0 to 1.0
void ModelData::AddPanTilt(int time_ms, double pan, double tilt, double pan_sen, double tilt_sen)
{
	double scale_pan = (pan * pan_sen);
	double scale_tilt = (tilt * tilt_sen);
	//if (!m_pt_values.empty())
	//{
	//	auto const& last_v = m_pt_values.back();
	//	if (last_v.pan == scale_pan &&
	//		last_v.tilt == scale_tilt)
	//	{
	//		return;
	//	}
	//}
	auto& pt = m_pt_values.emplace_back(time_ms, scale_pan, scale_tilt);
	CalcPanTiltDMX(pt);
}

void ModelData::CalcPanTilt( double pan, double tilt, double pan_sen, double tilt_sen)
{
	double scale_pan = (pan * pan_sen);
	double scale_tilt = (tilt * tilt_sen);
	PTDataPoint pt(0, scale_pan, scale_tilt);
	CalcPanTiltDMX(pt);
}

void ModelData::AddColor(int time_ms)
{
	//auto& pt = m_pt_values.emplace_back(time_ms, pan, tilt);
	//auto& pt = m_pt_values.emplace_back(time_ms, pan, tilt);
	//CalcPanTiltDMX(pt);
	if (m_color) {
		m_color->SetColorPixels(m_last_color);
	}
	if (!m_color_values.empty()) 
	{
		auto const& col = m_color_values.back().color;
		if (col == m_last_color)
		{
			return;
		}
	}
	m_color_values.emplace_back(time_ms, m_last_color);
}

void ModelData::ChangeColor(QColor color) {
	m_last_color = color;
	if (m_color) {
		m_color->SetColorPixels(m_last_color);
	}

	emit OnSetColor(m_last_color);
}

void ModelData::ChangeGobo(int diff)
{
	if (0u == gobo_chan)
	{
		return; 
	}
	gobo_index += diff;
	if (gobo_index < 0) 
	{
		gobo_index = gobo_values.size() - 1;
	}
	if (gobo_index >= gobo_values.size())
	{
		gobo_index = 0;
	}
	emit SetChannelData(gobo_chan, gobo_values[gobo_index]);
}

void ModelData::CalcPanTiltDMX(PTDataPoint& point)
{
	int tilt_value_dmx = m_tilt->ConvertPostoCmd(point.tilt);
	WriteCmdToPixel(tilt_value_dmx, m_tilt.get());
	uint8_t tlsb = tilt_value_dmx & 0xFF;
	uint8_t tmsb = tilt_value_dmx >> 8;

	point.tilt_coarse_dmx = tmsb;
	point.tilt_fine_dmx = tlsb;
	int pan_value_dmx = m_pan->ConvertPostoCmd(point.pan);
	WriteCmdToPixel(pan_value_dmx, m_pan.get());
	uint8_t plsb = pan_value_dmx & 0xFF;
	uint8_t pmsb = pan_value_dmx >> 8;

	point.pan_coarse_dmx = pmsb;
	point.pan_fine_dmx = plsb;
}

DegreeStringData ModelData::CreatePanTiltVCDate() const
{
	int totalLength = 0;
	for (auto const& point : m_pt_values)
	{
		totalLength += point.time_ms;
	}
	int curLen { 0 };

	double prev_pan{ -1.1 };
	double prev_tilt{ -1.1 };
	//data="Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=0.00:0.30;0.85:0.38;1.00:0.44|"
	QString vcPan = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	QString vcTilt = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	for (auto const& point : m_pt_values)
	{
		double const sc_pan = (point.pan + 180.0) / 360.0;//0-1 value
		double const sc_tilt = (point.tilt + 180.0) / 360.0;//0-1 value
		if (abs(prev_pan - sc_pan) > 0.001)
		{
			vcPan += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(sc_pan, 0, 'f', 2);
			prev_pan = sc_pan;
		}
		if (abs(prev_tilt - sc_tilt) > 0.001)
		{
			vcTilt += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(sc_tilt, 0, 'f', 2);
			prev_tilt = sc_tilt;
		}
		curLen += point.time_ms;
	}
	vcPan = vcPan.left(vcPan.count() - 1);
	vcPan += "|";
	vcTilt = vcTilt.left(vcTilt.count() - 1);
	vcTilt += "|";
	return { vcPan, vcTilt };
}

DMXStringData ModelData::CreatePanTiltDMXVCDate() const
{
	int totalLength = 0;
	for (auto const& point : m_pt_values)
	{
		totalLength += point.time_ms;
	}
	int curLen{ 0 };
	uint8_t prev_pan_course{ 255U };
	uint8_t prev_pan_fine{ 255U };
	uint8_t prev_tilt_course{ 255U };
	uint8_t prev_tilt_fine{ 255U };
	//data="Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=0.00:0.30;0.85:0.38;1.00:0.44|"
	QString vcPan_c = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	QString vcPan_f = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	QString vcTilt_c = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	QString vcTilt_f = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	for (auto const& point : m_pt_values)
	{
		bool last = &point != &m_pt_values.back();
		if (abs(prev_pan_course - point.pan_coarse_dmx) > 1 || last)
		{
			vcPan_c += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(point.pan_coarse_dmx / 255.0, 0, 'f', 2);
			prev_pan_course = point.pan_coarse_dmx;
		}

		if (abs(prev_pan_fine - point.pan_fine_dmx) > 1 || last)
		{
			vcPan_f += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(point.pan_fine_dmx / 255.0, 0, 'f', 2);
			prev_pan_fine = point.pan_fine_dmx;
		}

		if (abs(prev_tilt_course - point.tilt_coarse_dmx) > 1 || last)
		{
			vcTilt_c += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(point.tilt_coarse_dmx / 255.0, 0, 'f', 2);
			prev_tilt_course = point.tilt_coarse_dmx;
		}
		
		if (abs(prev_tilt_fine - point.tilt_fine_dmx) > 1 || last)
		{
			vcTilt_f += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(point.tilt_fine_dmx / 255.0, 0, 'f', 2);
			prev_tilt_fine = point.tilt_fine_dmx;
		}
		curLen += point.time_ms;
	}
	vcPan_c = vcPan_c.left(vcPan_c.count() - 1);
	vcPan_c += "|";
	vcTilt_c = vcTilt_c.left(vcTilt_c.count() - 1);
	vcTilt_c += "|";
	vcPan_f = vcPan_f.left(vcPan_f.count() - 1);
	vcPan_f += "|";
	vcTilt_f = vcTilt_f.left(vcTilt_f.count() - 1);
	vcTilt_f += "|";
	return DMXStringData(vcPan_c, vcPan_f, vcTilt_c, vcTilt_f );
}

void ModelData::WriteXMLFile(QString const& xmlFileName) const
{
	auto vc_data = CreatePanTiltVCDate();
	SaveFile("Pan", vc_data.pan, xmlFileName);
	SaveFile("Tilt", vc_data.tilt, xmlFileName);
	auto vc_dmx_data = CreatePanTiltDMXVCDate();
	SaveFile("Pan_Coarse_DMX", vc_dmx_data.pan_coarse, xmlFileName);
	SaveFile("Pan_Fine_DMX", vc_dmx_data.pan_fine, xmlFileName);
	SaveFile("Tilt_Coarse_DMX", vc_dmx_data.tilt_coarse, xmlFileName);
	SaveFile("Tilt_Fine_DMX", vc_dmx_data.tilt_fine, xmlFileName);
	SaveColorFile("Color", xmlFileName);
}

void ModelData::SaveFile(QString const& type, QString const& data, QString const& xmlFileName) const
{
	QFileInfo fileName(xmlFileName);
	QDir dir = fileName.dir();
	QString baseName = fileName.baseName();
	QString baseNameModified = baseName + "_" + type + "." + fileName.completeSuffix();
	QFileInfo fileModified(dir, baseNameModified);
	QString filePathModified = fileModified.filePath();
	QFile xmlFile(filePathModified);

	if (!xmlFile.open(QFile::WriteOnly | QFile::Text))
	{
		xmlFile.close();
		//LogMessage("Failed to Save File", spdlog::level::warn);
		//QMessageBox::warning(this, "Failed to Save File: " + xmlFileName, "Failed to Save File\n" + xmlFileName);
		return;
	}
	QTextStream xmlContent(&xmlFile);
	QDomDocument document;
	document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
	QDomElement valuecurve = document.createElement("valuecurve");
	valuecurve.setAttribute("data", data);
	valuecurve.setAttribute("SourceVersion", "2024.15");
	document.appendChild(valuecurve);
	xmlContent << document.toString();
	xmlFile.close();
}

QString ModelData::CreateColorVC() const
{
	int totalLength = 0;
	for (auto const& point : m_color_values)
	{
		totalLength += point.time_ms;
	}
	int curLen{ 0 };
	//data="Active=TRUE|Id=ID_BUTTON_Palette2|Values=x=0.000^c=green;x=0.500^c=white;x=1.000^c=red|"
	QString vcColor = "Active=TRUE|Id=ID_BUTTON_Palette1|Values=";
	for (auto const& point : m_color_values)
	{
		//totalLength += interval;
		vcColor += QString("x=%1^c=%2;").arg(curLen / (double)totalLength, 0, 'f', 3).arg(point.AsQString());
		curLen += point.time_ms;
	}
	vcColor = vcColor.left(vcColor.count() - 1);
	vcColor += "|";
	return vcColor;
}

void ModelData::SaveColorFile(QString const& type, QString const& xmlFileName) const
{
	QFileInfo fileName(xmlFileName);
	QString const& data = CreateColorVC();
	QDir dir = fileName.dir();
	QString baseName = fileName.baseName();
	QString baseNameModified = baseName + "_" + type + ".vcc";
	QFileInfo fileModified(dir, baseNameModified);
	QString filePathModified = fileModified.filePath();
	QFile xmlFile(filePathModified);

	if (!xmlFile.open(QFile::WriteOnly | QFile::Text))
	{
		xmlFile.close();
		//LogMessage("Failed to Save File", spdlog::level::warn);
		//QMessageBox::warning(this, "Failed to Save File: " + xmlFileName, "Failed to Save File\n" + xmlFileName);
		return;
	}
	QTextStream xmlContent(&xmlFile);
	QDomDocument document;
	document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
	QDomElement valuecurve = document.createElement("colorcurve");
	valuecurve.setAttribute("data", data);
	valuecurve.setAttribute("SourceVersion", "2024.15");
	document.appendChild(valuecurve);
	xmlContent << document.toString();
	xmlFile.close();
}

void ModelData::OpenModelFile(QString const& xmlFileName)
{
	QFile file(xmlFileName);

	if (!file.exists())
	{
		return;
	}

	// If we can't open it, let's show an error message
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return;
	}

	QXmlStreamReader xmlReader(&file);
	QString currentElementName;

	auto SetBoolValue = [](const QXmlStreamAttributes& attributes, QString const& parm, bool& value) {
		if (attributes.hasAttribute(parm))
		{
			bool ok{ false };
			auto const val = attributes.value(parm).toInt(&ok);
			if (ok)
			{
				value = val;
			}
		}
		};
	auto SetIntValue = [](const QXmlStreamAttributes& attributes, QString const& parm, int& value) {
		if (attributes.hasAttribute(parm))
		{
			bool ok{false};
			auto const val = attributes.value(parm).toInt(&ok);
			if (ok)
			{
				value = val;
			}
		}
	};

	auto SetUIntValue = [](const QXmlStreamAttributes& attributes, QString const& parm, uint32_t& value) {
		if (attributes.hasAttribute(parm))
		{
			bool ok{ false };
			auto const val = attributes.value(parm).toUInt(&ok);
			if (ok)
			{
				value = val;
			}
		}
		};

	auto SetUInt8Value = [](const QXmlStreamAttributes& attributes, QString const& parm, uint8_t& value) {
		if (attributes.hasAttribute(parm))
		{
			bool ok{ false };
			auto const val = attributes.value(parm).toUInt(&ok);
			if (ok)
			{
				value = val;
			}
		}
		};

	auto SetDoubleValue = [](const QXmlStreamAttributes& attributes, QString const& parm, double& value) {
		if (attributes.hasAttribute(parm))
		{
			bool ok{ false };
			auto const val = attributes.value(parm).toDouble(&ok);
			if (ok)
			{
				value = val;
			}
		}
		};

	while (!xmlReader.atEnd())
	{
		xmlReader.readNext();
		while (xmlReader.isStartElement())
		{
			currentElementName = xmlReader.name().toString();
			if(currentElementName == "dmxmodel" || currentElementName == "model")
			{
				auto const& attributes =  xmlReader.attributes();
				if (attributes.hasAttribute("DisplayAs")) 
				{
					auto const& type = attributes.value("DisplayAs");
					if (type == "DmxMovingHead")
					{
						SetDoubleValue(attributes, "DmxPanDegOfRot", m_pan->range_of_motion);
					}
					if (type == "DmxMovingHeadAdv")
					{
						//SetDoubleValue(attributes, "DmxPanDegOfRot", m_pan->range_of_motion);
						if (attributes.hasAttribute("DmxColorType"))
						{
							if (m_color) {
								disconnect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData);
							}
							auto const val = attributes.value("DmxColorType");
							if (val == "0")
							{
								auto rgbcolor = QSharedPointer<DmxColorRGB>(new DmxColorRGB);
								SetUIntValue(attributes, "DmxRedChannel", rgbcolor->red_channel);
								SetUIntValue(attributes, "DmxGreenChannel", rgbcolor->green_channel);
								SetUIntValue(attributes, "DmxBlueChannel", rgbcolor->blue_channel);
								SetUIntValue(attributes, "DmxWhiteChannel", rgbcolor->white_channel);
								m_color = /*td::move*/(rgbcolor);
								connect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData, Qt::UniqueConnection);
							}
							else if (val == "1")
							{
								auto wheelcolor = QSharedPointer<DmxColorWheel>(new DmxColorWheel);
								SetUIntValue(attributes, "DmxColorWheelChannel", wheelcolor->wheel_channel);
								SetUIntValue(attributes, "DmxDimmerChannel", wheelcolor->dimmer_channel);
								for (int k =0;k<100;++k) 
								{
									auto wheelColParm = QString("DmxColorWheelColor%1").arg(k);
									auto wheelDMXParm = QString("DmxColorWheelDMX%1").arg(k);
									if (!attributes.hasAttribute(wheelColParm) && !attributes.hasAttribute(wheelDMXParm))
									{
										break;
									}

									auto color = attributes.value(wheelColParm);
									auto dmx = attributes.value(wheelDMXParm).toUInt();
									wheelcolor->colors.emplace_back(QColor(color), dmx);
								}
								//DmxColorWheelColor13="#c0c0c0" DmxColorWheelDMX13="104"
								m_color = /*td::move*/(wheelcolor);
								connect(m_color.get(), &DmxColor::SetChannelData, m_out, &OutputManager::SetData, Qt::UniqueConnection);
							}
							if (m_color) 
							{
								SetUIntValue(attributes, "DmxShutterChannel", m_color->shutter_channel);
								SetUInt8Value(attributes, "DmxShutterOnValue", m_color->shutter_on_value);
							}
						}
						//"DmxColorType"

					}
				}
			}
			if (currentElementName == "PanMotor")
			{
				auto const& attributes = xmlReader.attributes();
				SetUIntValue(attributes, "ChannelCoarse", m_pan->channel_coarse);
				SetUIntValue(attributes, "ChannelFine", m_pan->channel_fine);
				SetIntValue(attributes, "MinLimit", m_pan->min_limit);
				SetIntValue(attributes, "OrientZero", m_pan->orient_zero);
				SetIntValue(attributes, "OrientHome", m_pan->orient_home);
				SetBoolValue(attributes, "Reverse", m_pan->reverse);
				SetDoubleValue(attributes, "RangeOfMotion", m_pan->range_of_motion);
			}
			if (currentElementName == "TiltMotor")
			{
				auto const& attributes = xmlReader.attributes();
				SetUIntValue(attributes, "ChannelCoarse", m_tilt->channel_coarse);
				SetUIntValue(attributes, "ChannelFine", m_tilt->channel_fine);
				SetIntValue(attributes, "MinLimit", m_tilt->min_limit);
				SetIntValue(attributes, "OrientZero", m_tilt->orient_zero);
				SetIntValue(attributes, "OrientHome", m_tilt->orient_home);
				SetBoolValue(attributes, "Reverse", m_tilt->reverse);
				SetDoubleValue(attributes, "RangeOfMotion", m_tilt->range_of_motion);
			}
			xmlReader.readNext();
		}

		if (xmlReader.isEndElement())
		{
			if (xmlReader.name() == "dmx")
			{
				
			}
			continue;
		}

		if (xmlReader.isCharacters() && !xmlReader.isWhitespace())
		{
			QString key = currentElementName;
			QString value = xmlReader.text().toString();

			currentElementName.clear();
		}
	}
}

void ModelData::WriteCmdToPixel(int value, MotorData* motor)
{
	uint8_t lsb = value & 0xFF;
	uint8_t msb = value >> 8;
	
	//int coarse_channel = motor->GetChannelCoarse() - 1;
	//int fine_channel = motor->GetChannelFine() - 1;

	if (motor->channel_coarse > 0) {
		//m_data[motor->channel_coarse -1] = msb;
		//buffer.SetPixel(coarse_channel, 0, msb_c, false, false, true);
		emit SetChannelData(motor->channel_coarse, msb);
	}
	if (motor->channel_fine > 0) {
		//m_data[motor->channel_fine - 1] = lsb;
		//buffer.SetPixel(fine_channel, 0, lsb_c, false, false, true);
		emit SetChannelData(motor->channel_fine, lsb);
	}
}


