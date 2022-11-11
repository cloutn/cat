#pragma once

#include "scl/vector.h"

namespace game {

class Config
{
public:
	Config();

	scl::vector4	getClearColorf();

	void load(const char* const filename);
	void save(const char* const filename);

	bool			showDemoWindow			= false;
	bool			showDeviceInfoWindow	= false;
	bool			showConfigWindow		= false;
	uint32			clearColor				= 0xFF333333;
	scl::vector2i	screenSize				= { 1280, 800 };
	scl::vector2i	screenPos				= { 100, 100 };

}; // class Config


} // namespace game



