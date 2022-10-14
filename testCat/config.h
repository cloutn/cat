#pragma once

#include "scl/vector.h"

namespace game {

class Config
{
public:
	Config();

	scl::vector4 clearColorf();

	bool	showDemoWindow;	
	bool	showDeviceInfoWindow;
	uint32	clearColor;
};

}
