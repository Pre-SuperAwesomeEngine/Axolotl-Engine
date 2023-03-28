#include "WindowMainMenu.h"

#include "Application.h"
#include "ModuleScene.h"
#include "DataModels/Scene/Scene.h"

#include "SDL.h"

const std::string WindowMainMenu::repositoryLink = "https://github.com/Horizons-Games/Axolotl-Engine";
bool WindowMainMenu::defaultEnabled = true;

WindowMainMenu::WindowMainMenu(const std::vector< std::unique_ptr<EditorWindow> >& editorWindows) :
	Window("Main Menu"), showAbout(false), openPopup(false), isSaving(false), action(Actions::NONE), about(std::make_unique<WindowAbout>()),
	loadScene(std::make_unique<WindowLoadScene>()), saveScene(std::make_unique<WindowSaveScene>())
{
	for (const std::unique_ptr<EditorWindow>& window : editorWindows)
	{
		std::pair<std::string, bool> windowNameAndEnabled;
		if (window->GetName() == "Configuration") windowNameAndEnabled = std::make_pair(window->GetName(), false);
		else windowNameAndEnabled = std::make_pair(window->GetName(), defaultEnabled);
		windowNamesAndEnabled.push_back(windowNameAndEnabled);
	}
}

WindowMainMenu::~WindowMainMenu()
{
}

void WindowMainMenu::Draw(bool& enabled)
{
	if (openPopup) DrawPopup();
	else if (!isSaving && action != Actions::NONE) {
		if (action == Actions::NEW_SCENE)
		{
			CreateNewScene();
			action = Actions::NONE;
		}
		else if (action == Actions::EXIT) Exit();
	}
	if (isSaving) saveScene->SaveAsWindow(isSaving);
	if (ImGui::BeginMainMenuBar())
	{
		DrawFileMenu();
		DrawWindowMenu();
		DrawBuildGameMenu();
		DrawHelpMenu();
	}
	ImGui::EndMainMenuBar();
}

void WindowMainMenu::Exit()
{
	//to make it easier in terms of coupling between classes,
	//just push an SDL_QuitEvent to the event queue
	SDL_Event quitEvent;
	quitEvent.type = SDL_QUIT;
	SDL_PushEvent(&quitEvent);
}

void WindowMainMenu::CreateNewScene()
{
	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	scene->InitNewEmptyScene();
	App->scene->SetLoadedScene(std::move(scene));
}

void WindowMainMenu::DrawPopup()
{
	ImGui::OpenPopup("Are you sure?");
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Are you sure?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to save the scene?\nAll your changes will be lost if you don't save them.");
		ImGui::Separator();
		std::string filePathName = App->scene->GetLoadedScene()->GetRoot()->GetName();
		if (ImGui::Button("Save scene", ImVec2(120, 0)))
		{
			if (filePathName != "New Scene") App->scene->SaveSceneToJson(filePathName + SCENE_EXTENSION);
			else isSaving = true;
			ImGui::CloseCurrentPopup();
			openPopup = false;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Close without saving", ImVec2(240, 0)))
		{
			isSaving = false;
			openPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void WindowMainMenu::DrawFileMenu()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::Button(ICON_IGFD_FILE " New Scene"))
		{
			openPopup = true;
			action = Actions::NEW_SCENE;
		}
		loadScene->DrawWindowContents();
		if (ImGui::Button(ICON_IGFD_SAVE " Save Scene"))
		{
			std::string filePathName = App->scene->GetLoadedScene()->GetRoot()->GetName();
			// We should find a way to check if the scene has already been saved
			// Using "New Scene" is a patch
			if (filePathName != "New Scene") App->scene->SaveSceneToJson(filePathName + SCENE_EXTENSION);
			else isSaving = true;
		}
		saveScene->DrawWindowContents();
		if (ImGui::MenuItem("Exit"))
		{
			openPopup = true;
			action = Actions::EXIT;
		}
		ImGui::EndMenu();
	}
}

void WindowMainMenu::DrawWindowMenu()
{
	if (ImGui::BeginMenu("Window"))
	{
		for (std::pair<std::string, bool>& windowNameAndEnabled : windowNamesAndEnabled)
		{
			ImGui::MenuItem(windowNameAndEnabled.first.c_str(), NULL, &windowNameAndEnabled.second);
		}
		ImGui::EndMenu();
	}
}

void WindowMainMenu::DrawHelpMenu()
{
	if (ImGui::BeginMenu("Help"))
	{
		ImGui::MenuItem("About Axolotl", NULL, &showAbout);
		if (ImGui::MenuItem("GitHub Link"))
		{
			ShellExecute(NULL, "open", repositoryLink.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		ImGui::EndMenu();
	}
	about->Draw(showAbout);
}

void WindowMainMenu::DrawBuildGameMenu()
{
	if (ImGui::BeginMenu("Build"))
	{
		if (ImGui::MenuItem("Debug"))
		{
			BuildGame(GameBuildType::DEBUG_GAME);
		}
		if (ImGui::MenuItem("Release"))
		{
			BuildGame(GameBuildType::RELEASE_GAME);
		}
		ImGui::EndMenu();
	}
}

void WindowMainMenu::BuildGame(GameBuildType buildType)
{
	if (!system(nullptr))
	{
		ENGINE_LOG("No command processor available, not building");
		return;
	}
	std::string buildConfig;
	switch (buildType)
	{
	case WindowMainMenu::GameBuildType::DEBUG_GAME:
		ENGINE_LOG("Building DebugGame...");
		buildConfig = "DebugGame";
		break;
	case WindowMainMenu::GameBuildType::RELEASE_GAME:
		ENGINE_LOG("Building ReleaseGame...");
		buildConfig = "ReleaseGame";
		break;
	default:
		assert(false && "Unknown build type, never should arrive here");
		break;
	}
	std::string msbuildPath = "\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\2019"
		"\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe\"";
	std::string solutionPath = "..\\Source\\Engine.sln";
	std::string configurationParameter = "/p:Configuration=" + buildConfig;
	std::string platformParameter = "/p:Platform=x64";

	std::string command = msbuildPath + " " + solutionPath + " " + configurationParameter + " " + platformParameter;
	
	int result = system(command.c_str());
	ENGINE_LOG("Build result: %d", result);
}
