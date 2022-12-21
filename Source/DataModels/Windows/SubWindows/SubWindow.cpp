#include "SubWindow.h"

#include "imgui.h"

bool SubWindow::defaultEnabled = true;

SubWindow::SubWindow(const std::string& name) : Window(name)
{
}

SubWindow::~SubWindow()
{
}

void SubWindow::Draw(bool& enabled)
{
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		DrawWindowContents();
	}
}
