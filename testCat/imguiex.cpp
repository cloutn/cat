#include "./imguiex.h"

#include "imgui/imgui.h"

#include "scl/vector.h"
#include "scl/string.h"
#include "cat/color.h"

//#include <string>

namespace imguiex {

static string256 _label(const char* const label)
{
	float width = ImGui::CalcItemWidth();

	float x = ImGui::GetCursorPosX();
	ImGui::Text(label); 
	ImGui::SameLine(); 
	ImGui::SetCursorPosX(x + width * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::SetNextItemWidth(-1);

	string256 labelID = "##";
	labelID += label;

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


string256 leftLable(const char* const name)
{
	return _label(name);
}

} // namespace cat



