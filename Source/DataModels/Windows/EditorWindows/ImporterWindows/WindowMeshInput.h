#pragma once
#include "Windows/EditorWindows/WindowFileBrowser.h"

class ComponentMeshRenderer;

class WindowMeshInput : public WindowFileBrowser
{
public:
	WindowMeshInput(ComponentMeshRenderer* componentMesh);
	~WindowMeshInput() override;

	void DoThisIfOk() override;

private:
	ComponentMeshRenderer* componentMesh;
};
