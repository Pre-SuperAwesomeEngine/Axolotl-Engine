#pragma once
#include "Module.h"

class EditorWindow;
class WindowMainMenu;
class WindowScene;

class ModuleEditor : public Module
{
public:
	ModuleEditor();
	~ModuleEditor() override;

	bool Init() override;
	bool Start() override;
	bool CleanUp() override;

	update_status PreUpdate() override;
	update_status Update() override;
	update_status PostUpdate() override;

	void Resized();

	bool IsSceneFocused() const;

private:
	std::vector<std::string> lines;
	std::vector<std::shared_ptr<EditorWindow> > windows;
	std::unique_ptr<WindowMainMenu> mainMenu = nullptr;
	std::shared_ptr<WindowScene> scene = nullptr;

	bool windowResized = false;
};

