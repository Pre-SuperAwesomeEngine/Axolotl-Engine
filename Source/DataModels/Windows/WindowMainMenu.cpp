#include "WindowMainMenu.h"

#include "imgui.h"
#include "SDL.h"

const std::string WindowMainMenu::repositoryLink = "https://github.com/Pre-SuperAwesomeEngine/Engine";
bool WindowMainMenu::defaultEnabled = true;

WindowMainMenu::WindowMainMenu(const std::vector< std::shared_ptr<EditorWindow> >& editorWindows) : Window("Main Menu")
{
	about = std::make_unique<WindowAbout>();
	browser = std::make_unique<WindowFileBrowser>();
	nWindows = editorWindows.size();
	for (std::shared_ptr<EditorWindow> window : editorWindows)
	{
		windowNames.push_back(window->GetName());
		windowsEnabled.push_back(true);
	}
}

WindowMainMenu::~WindowMainMenu()
{
}

void WindowMainMenu::Draw(bool& enabled)
{
	if (ImGui::BeginMainMenuBar())
	{
		DrawWindowsMenu();
		DrawAbout();
		//DrawFileDialog();
		DrawFileBrowser();
		DrawGithubLink();
		DrawExit();
	}
	ImGui::EndMainMenuBar();
}

void WindowMainMenu::DrawWindowsMenu()
{
	if (ImGui::BeginMenu("Windows"))
	{
		for (int i = 0; i < nWindows; ++i)
		{
			bool windowEnabled = IsWindowEnabled(i);
			ImGui::MenuItem(windowNames[i].c_str(), NULL, &windowEnabled);
			SetWindowEnabled(i, windowEnabled);
		}
		ImGui::EndMenu();
	}
}

void WindowMainMenu::DrawAbout()
{
	if (ImGui::MenuItem("About"))
	{
		showAbout = !showAbout;
	}		
	about->Draw(showAbout);
}

/*void WindowMainMenu::DrawFileDialog()
{
	if (ImGui::MenuItem("File dialog"))
	{
		showFileDialog = !showFileDialog;
	}		
	importer->Draw(showFileDialog);
}*/

void WindowMainMenu::DrawFileBrowser()
{
	if (ImGui::MenuItem("File browser"))
	{
		showFileBrowser = !showFileBrowser;
	}
	browser->Draw(showFileBrowser);
}

void WindowMainMenu::DrawGithubLink() const
{
	if (ImGui::MenuItem("GitHub Link"))
	{
		ShellExecute(NULL, "open", repositoryLink.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
}

void WindowMainMenu::DrawExit() const
{
	if (ImGui::MenuItem("Exit"))
	{
		//to make it easier in terms of coupling between classes,
		//just push an SDL_QuitEvent to the event queue
		SDL_Event quitEvent;
		quitEvent.type = SDL_QUIT;
		SDL_PushEvent(&quitEvent);
	}
}
