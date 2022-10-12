#pragma once

class Config
{
public:
	Config() : 
		showDemoWindow	(false),
		showDeviceInfoWindow(false),
		clearColor		(0xFF333333)
	{
	}

	bool	showDemoWindow;	
	bool	showDeviceInfoWindow;
	uint32	clearColor;
};


