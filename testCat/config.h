#pragma once

class Config
{
public:
	Config() : 
		showDemoWindow	(false),
		clearColor		(0xFF333333)
	{
	}

	bool	showDemoWindow;	
	uint32	clearColor;
};


