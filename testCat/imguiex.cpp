#include "./imguiex.h"

#include "imgui/imgui.h"

#include "scl/vector.h"
#include "scl/string.h"
#include "cat/color.h"

//#include <string>

namespace imguiex {

using scl::string128;
using scl::string256;

static string256 _label(const char* const label, const char* const id = NULL)
{
	float width = ImGui::CalcItemWidth();

	float x = ImGui::GetCursorPosX();
	ImGui::Text(label); 
	ImGui::SameLine(); 
	ImGui::SetCursorPosX(x + width * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::SetNextItemWidth(-1);

	string256 labelID = "##";
	labelID += label;
	if (NULL != id)
		labelID += id;
	//labelID.format_append("%d", id);

	return labelID;
}

void labelText(const char* const name, const char* valueFormat, ...) 
{ 
	_label(name);

	va_list arg;
	va_start(arg, valueFormat);
	ImGui::TextV(valueFormat, arg);
	va_end(arg);
}

void inputText(const char* const label, char* text, const int textCapacity)
{
	ImGui::InputText(_label(label).c_str(), text, textCapacity);
}

void inputFloat(const char* const label, float& v)
{
	ImGui::InputFloat(_label(label).c_str(), &v);
}

void inputFloat2(const char* const label, scl::vector2& v)
{
	ImGui::InputFloat2(_label(label).c_str(), &v.x);
}

void inputFloat3(const char* const label, scl::vector3& v)
{
	ImGui::InputFloat3(_label(label).c_str(), &v.x);
}

void inputFloat4(const char* const label, scl::vector4& v)
{
	ImGui::InputFloat4(_label(label).c_str(), &v.x);
}

void inputMatrix4(const char* const label, scl::matrix& v)
{
	ImGui::InputFloat4(_label(label, "0").c_str(), v.m[0]);
	for (int i = 1; i < 4; ++i)
	{
		string128 labelRow;
		labelRow.format("%s%d", label, i);
		ImGui::InputFloat4(_label("", labelRow.c_str()).c_str(), v.m[i]);
	}
}

void inputDouble(const char* const label, double& v)
{
	ImGui::InputDouble(_label(label).c_str(), &v);
}

void checkbox(const char* const label, bool& v)
{
	ImGui::Checkbox(_label(label).c_str(), &v);
}

void inputInt2(const char* const label, scl::vector2i& v)
{
	ImGui::InputInt2(_label(label).c_str(), &v.x);
}

void inputHex(const char* const label, uint32& v)
{
	ImGui::InputInt(_label(label).c_str(), (int*)&v, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
}
       

void inputColorFloat(const char* const label, scl::vector4& v)
{
	ImGuiColorEditFlags flags = 0;	//ImGuiColorEditFlags_AlphaPreview;
	ImGui::ColorEdit3(_label(label).c_str(), (float*)&v, flags);
}

void inputColorInt(const char* const label, uint32& v)
{
	scl::vector4 color;
	cat::argb_to_float(v, color.a, color.r, color.g, color.b);
	inputColorFloat(label, color);
	v = cat::float_to_argb(color.a, color.r, color.g, color.b);
}


scl::string256 leftLable(const char* const name)
{
	return _label(name);
}

} // namespace cat



