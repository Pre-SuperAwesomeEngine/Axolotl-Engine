#pragma once

#include "EditorWindow.h"

#include "FileSystem/UniqueID.h"
// TODO: REMOVE
#include "Windows/EditorWindows/ImporterWindows/WindowLoadScene.h"
#include "ImporterWindows/WindowSaveScene.h"
// --
#include <memory>

class Model;
class GameObject;
class Component;
class ComponentCamera;
enum class LightType;

class WindowInspector : public EditorWindow
{
public:
	WindowInspector();
	~WindowInspector();

protected:
	void DrawWindowContents() override;

	ImVec2 GetStartingSize() const override;

private:
	void DrawChangeActiveComponentContent(int labelNum, const std::shared_ptr<Component>& component);
	bool DrawDeleteComponentContent(int labelNum, const std::shared_ptr<Component>& component);
	void DrawTextureTable();
	bool MousePosIsInWindow();
	bool WindowRightClick();

	void AddComponentMeshRenderer();
	void AddComponentMaterial();
	void AddComponentLight(LightType type);

	// TODO: REMOVE
	bool showSaveScene = true;
	bool showLoadScene = true;
	void DrawButtomsSaveAndLoad();
	std::unique_ptr<WindowLoadScene> loadScene;
	std::unique_ptr<WindowSaveScene> saveScene;
	// --
};

inline ImVec2 WindowInspector::GetStartingSize() const
{
	return ImVec2(900, 250);
}
