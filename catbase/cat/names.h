#pragma once

namespace cat {

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
