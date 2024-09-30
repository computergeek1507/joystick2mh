#pragma once

#include <QString>
#include <QColor>
#include <map>

static const std::vector<std::pair<QColor, QString> > COLOR_NAME_MAP = {
	{QColor(Qt::white), "white"},
	{QColor(Qt::black), "black"},
	{QColor(Qt::blue), "blue"},
	{QColor(Qt::cyan), "cyan"},
	{QColor(Qt::green), "green"},
	{QColor(Qt::magenta), "magenta"},
	{QColor(Qt::red), "red"},
	{QColor(Qt::yellow), "yellow"},
};