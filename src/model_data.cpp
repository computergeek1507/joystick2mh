#include "model_data.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>
#include <QXmlStreamReader>

ModelData::ModelData(QSettings* sett) 
{
	memset(m_data, 0x00, sizeof(m_data));
	m_pan = std::make_unique< MotorData>();
	m_tilt = std::make_unique< MotorData>();
	ReadSettings(sett);
}

void ModelData::ReadSettings(QSettings* sett)
{
	sett->beginGroup("tilt");
	m_tilt->channel_coarse = sett->value("channel_coarse", 0).toInt();
	m_tilt->channel_fine = sett->value("channel_fine", 0).toInt();
	m_tilt->min_limit = sett->value("min_limit", -180).toInt();
	m_tilt->max_limit = sett->value("max_limit", 180).toInt();
	m_tilt->range_of_motion = sett->value("range_of_motion", 150.0).toFloat();
	m_tilt->orient_zero = sett->value("orient_zero", 0).toInt();
	m_tilt->orient_home = sett->value("orient_home", 90).toInt();
	m_tilt->reverse = sett->value("reverse", false).toBool();
	sett->endGroup();

	sett->beginGroup("pan");
	m_pan->channel_coarse = sett->value("channel_coarse", 0).toInt();
	m_pan->channel_fine = sett->value("channel_fine", 0).toInt();
	m_pan->min_limit = sett->value("min_limit", -180).toInt();
	m_pan->max_limit = sett->value("max_limit", 180).toInt();
	m_pan->range_of_motion = sett->value("range_of_motion", 540.0).toFloat();
	m_pan->orient_zero = sett->value("orient_zero", 0).toInt();
	m_pan->orient_home = sett->value("orient_home", 90).toInt();
	m_pan->reverse = sett->value("reverse", false).toBool();
	sett->endGroup();
}

void ModelData::SaveSettings(QSettings* sett) 
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
}

void ModelData::ClearData()
{
	m_wheel_values.clear();
	m_rgb_values.clear();
	m_pt_values.clear();
	m_last_color = QColor(Qt::black);
}

//-1.0 to 1.0
void ModelData::AddPanTilt(int time_ms, double pan, double tilt)
{
	auto& pt = m_pt_values.emplace_back(time_ms, pan, tilt);
	CalcPanTiltDMX(pt);
}

void ModelData::AddColor(int time_ms)
{
	//auto& pt = m_pt_values.emplace_back(time_ms, pan, tilt);
	//auto& pt = m_pt_values.emplace_back(time_ms, pan, tilt);
	//CalcPanTiltDMX(pt);
}

void ModelData::ChangeColor(QColor color) {m_last_color = color;}

void ModelData::CalcPanTiltDMX(PTDataPoint& point)
{
	point.tilt_dmx = m_tilt->ConvertPostoCmd(point.tilt * 180.0);

	point.pan_dmx = m_pan->ConvertPostoCmd(point.pan * 180.0);
	WriteCmdToPixel(point.tilt_dmx, m_tilt.get());
	WriteCmdToPixel(point.pan_dmx, m_pan.get());
}

std::tuple<QString,QString> ModelData::CreatePanTiltVCDate() const
{
	int totalLength = 0;
	for (auto const& point : m_pt_values)
	{
		totalLength += point.time_ms;
	}
	int curLen { 0 };
	//data="Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=0.00:0.30;0.85:0.38;1.00:0.44|"
	QString vcPan = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	QString vcTilt = "Active=TRUE|Id=ID_VALUECURVE_XVC|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=";
	for (auto const& point : m_pt_values)
	{
		//totalLength += interval;
		vcPan += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(point.pan, 0, 'f', 2);

		vcTilt += QString("%1:%2;").arg(curLen / (double)totalLength, 0, 'f', 2).arg(point.tilt, 0, 'f', 2);
		curLen += point.time_ms;
	}
	vcPan = vcPan.left(vcPan.count() - 1);
	vcPan += "|";
	vcTilt = vcTilt.left(vcTilt.count() - 1);
	vcTilt += "|";
	return { vcPan, vcTilt };
}

void ModelData::WriteXMLFile(QString const& xmlFileName) const
{
	auto vc_data = CreatePanTiltVCDate();
	SaveFile("Pan", std::get<0>(vc_data), xmlFileName);
	SaveFile("Tilt", std::get<1>(vc_data), xmlFileName);
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
			if(currentElementName == "dmxmodel")
			{
				auto const& attributes =  xmlReader.attributes();
				if (attributes.hasAttribute("DisplayAs")) 
				{
					auto const& type = attributes.value("DisplayAs");
					if (type == "DmxMovingHead")
					{
						SetDoubleValue(attributes, "DmxPanDegOfRot", m_pan->range_of_motion);
					}
				}
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

	if (motor->channel_coarse >= 0) {
		m_data[motor->channel_coarse -1] = msb;
		//buffer.SetPixel(coarse_channel, 0, msb_c, false, false, true);
		emit SetChannelData(motor->channel_coarse, msb);
	}
	if (motor->channel_fine >= 0) {
		m_data[motor->channel_fine - 1] = lsb;
		//buffer.SetPixel(fine_channel, 0, lsb_c, false, false, true);
		emit SetChannelData(motor->channel_fine, lsb);
	}
}

int MotorData::ConvertPostoCmd(float position) const
{
	/*

	int channel_coarse{ 0 };
	int channel_fine{ 0 };
	int min_value{ 0 };
	int max_value{ 65535 };
	int min_limit{ -180 };
	int max_limit{ 180 };
	float range_of_motion{180.0f};
	int orient_zero { 0 };
	int orient_home{ 0 };
	float slew_limit{ 0.0F };
	bool reverse{false};
	bool upside_down{false};
	int rev{ 1 };
	*/
	int rev{ 1 };
	if (reverse) {
		rev = -1;
	}
	else {
		rev = 1;
	}
	float limited_pos = position;
	if (limited_pos > max_limit) {
		limited_pos = max_limit;
	}
	else if (limited_pos < min_limit) {
		limited_pos = min_limit;
	}

	//if (upside_down) {
	//	limited_pos = -1.0f * limited_pos;
	//}

	float goto_home = (float)max_value * (float)orient_home / range_of_motion;
	float amount_to_move = (float)max_value * limited_pos / range_of_motion * rev;
	float cmd = goto_home + amount_to_move;
	float full_spin = (float)max_value * 360.0 / range_of_motion;

	if (cmd < 0) {
		if (cmd + full_spin < max_value) {
			cmd += full_spin;
		}
		else {
			cmd = 0; // tbd....figure out which limit is closer to desired target
		}
	}
	else if (cmd > max_value) {
		if (cmd - full_spin >= 0.0f) {
			cmd -= full_spin;
		}
		else {
			cmd = max_value; // tbd....figure out which limit is closer to desired target
		}
	}
	return cmd;
}
