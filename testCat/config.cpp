#include "./config.h"

#include "cat/color.h"
#include "cat/yaml.h"

#include "scl/file.h"

namespace game {

Config::Config()
{
}

scl::vector4 Config::getClearColorf()
{
	scl::vector4 color;
	cat::argb_to_float(clearColor, color.a, color.r, color.g, color.b);
	return color;
}

void Config::load(const char* const filename)
{
	if (!scl::file::exists(filename))
		return;

	yaml::document doc;
	yaml::node root = doc.load(filename);
	showDemoWindow = root["showDemoWindow"].to_bool();
	showDeviceInfoWindow = root["showDeviceInfoWindow"].to_bool();
	clearColor = root["clearColor"].to_uint();
	screenSize = root["screenSize"].to_vector2i();
	screenPos = root["screenPos"].to_vector2i();
}

void Config::save(const char* const filename)
{
	yaml::document doc;
	yaml::node root = doc.root().set_map();

	root.add("showDemoWindow", showDemoWindow);
	root.add("showDeviceInfoWindow", showDeviceInfoWindow);
	root.add("clearColor", ryml::fmt::hex(clearColor));
	root.add("screenSize", screenSize);
	root.add("screenPos", screenPos);

	doc.save(filename);
}

} // namespace game

