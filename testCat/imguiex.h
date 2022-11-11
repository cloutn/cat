#pragma once

#include "scl/type.h"
#include "scl/string.h"

namespace scl {
	class vector2i;
	class vector4;
}

namespace imguiex {


void labelText		(const char* const name, const char* valueFormat, ...); 
void inputDouble	(const char* const label, double& v);
void checkbox		(const char* const label, bool& v);
void inputInt2		(const char* const label, scl::vector2i& v);
void inputHex		(const char* const label, uint32& v);
void inputColorFloat(const char* const label, scl::vector4& v);
void inputColorInt	(const char* const label, uint32& v);

string256 leftLable(const char* const name);
#define LEFT_LABEL(s) leftLable(s).c_str()


} // namespace cat



