#include "data_point.h"


#include "colors.h"

QString ColorDataPoint::AsQString() const
{
	for (auto const& [key,val] : COLOR_NAME_MAP)
	{
		if (key == color)
		{
			return val;
		}
	}
	//rgb(255@ 128@ 0)
	return QString("rgb(%1@ %2@ %3)").arg(color.red()).arg(color.green()).arg(color.blue());
}