#pragma once
#include "EditorWindow.h"
#include "Windows/SubWindows/SubWindow.h"
#include "ModuleScene.h"
#include <vector>
#include <memory>

class GameObject;

class WindowEditorControl : public EditorWindow
{
public:
	WindowEditorControl();
	~WindowEditorControl();

protected:
	void DrawWindowContents() override;

	ImVec2 GetStartingSize() const override;
};

inline ImVec2 WindowEditorControl::GetStartingSize() const
{
	return ImVec2(900, 250);
}
