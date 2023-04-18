#pragma once
#include "Module.h"

class EditorWindow;
class WindowMainMenu;
class WindowDebug;
class WindowScene;
class WindowInspector;
class Resource;
class GameObject;

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

	const WindowScene* GetScene() const;
		  WindowMainMenu* GetMainMenu() const;
	const WindowDebug* GetDebugOptions() const;

	bool IsSceneFocused() const;
	void SetResourceOnInspector(const std::weak_ptr<Resource>& resource) const;
	void RefreshInspector() const;
	std::pair<int, int> GetAvailableRegion();

private:
	void CopyAnObject();
	void PasteAnObject();
	void CutAnObject();
	void DuplicateAnObject();

	std::vector<std::unique_ptr<EditorWindow> > windows;
	std::unique_ptr<WindowMainMenu> mainMenu = nullptr;
	std::unique_ptr<WindowDebug> debugOptions = nullptr;

	WindowInspector* inspector;
	WindowScene* scene;
	bool windowResized;

	GameObject* copyObject;
};

inline void ModuleEditor::Resized()
{
	windowResized = true;
}

inline const WindowScene* ModuleEditor::GetScene() const
{
	return scene;
}

inline WindowMainMenu* ModuleEditor::GetMainMenu() const
{
	return mainMenu.get();
}

inline const WindowDebug* ModuleEditor::GetDebugOptions() const
{
	return debugOptions.get();
}


