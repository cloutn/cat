#pragma once

#include "scl/type.h"
#include "scl/string.h"

namespace scl {
	class vector2i;
	class vector2;
	class vector3;
	class vector4;
	class matrix;
}

namespace imguiex {


void labelText		(const char* const name, const char* valueFormat, ...); 
void inputText		(const char* const label, char* text, const int textCapacity);
void inputFloat		(const char* const label, float& v);
void inputFloat2	(const char* const label, scl::vector2& v);
void inputFloat3	(const char* const label, scl::vector3& v);
void inputFloat4	(const char* const label, scl::vector4& v);
void inputMatrix4	(const char* const label, scl::matrix& v);
void inputDouble	(const char* const label, double& v);
void checkbox		(const char* const label, bool& v);
void inputInt2		(const char* const label, scl::vector2i& v);
void inputHex		(const char* const label, uint32& v);
void inputColorFloat(const char* const label, scl::vector4& v);
void inputColorInt	(const char* const label, uint32& v);

scl::string256 leftLable(const char* const name);
#define LEFT_LABEL(s) leftLable(s).c_str()


} // namespace cat



