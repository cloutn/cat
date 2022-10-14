#include "./config.h"

#include "cat/color.h"

namespace game {

Config::Config() :
	showDemoWindow(false),
	showDeviceInfoWindow(false),
	clearColor(0xFF333333)
{

}

scl::vector4 Config::clearColorf()
{
	scl::vector4 color;
	cat::argb_to_float(clearColor, color.a, color.r, color.g, color.b);
	return color;
}

}

