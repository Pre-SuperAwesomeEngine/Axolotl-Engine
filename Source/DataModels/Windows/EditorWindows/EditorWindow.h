#pragma once

#include "Windows/Window.h"

#include "imgui.h"

class EditorWindow : public Window
{
public:
	~EditorWindow();

	void Draw(bool& enabled) override;

	inline bool IsFocused() const
	{
		return focused;
	}

protected:
	EditorWindow(const std::string& name);
	virtual void DrawWindowContents() = 0;

	ImGuiWindowFlags flags = ImGuiWindowFlags_None;

private:
	bool focused = false;
};

