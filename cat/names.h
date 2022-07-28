#pragma once

namespace cat {

//#ifdef near
//#undef near
//#endif
//
//#ifdef far
//#undef far
//#endif
//
//#ifdef min 
//#undef min 
//#endif
//
//#ifdef max
//#undef max
//#endif

typedef const char* const pcs;
#define _cs(name_var) pcs name_var = #name_var;

class Names
{
public:
	_cs(scale)
	_cs(translation)
	_cs(rotation)
};

extern const Names names;

} // namespace ui
