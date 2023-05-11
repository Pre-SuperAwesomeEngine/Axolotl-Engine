#include "WindowComponentScript.h"

#include "Application.h"
#include "Scene/Scene.h"

#include "Modules/ModuleScene.h"
#include "FileSystem/ModuleFileSystem.h"

#include "Components/ComponentScript.h"

#include "ScriptFactory.h"
#include "IScript.h"

WindowComponentScript::WindowComponentScript(ComponentScript* component) :
	ComponentWindow("SCRIPT", component), windowUID(UniqueID::GenerateUID())
{
}


WindowComponentScript::~WindowComponentScript()
{
}

void WindowComponentScript::DrawWindowContents()
{
	DrawEnableAndDeleteComponent();

	ImGui::Text("");

	std::vector<const char*> constructors = App->GetScriptFactory()->GetConstructors();
	ComponentScript* script = static_cast<ComponentScript*>(component);

	if (!script)
	{
		return;
	}

	std::string label = "Select Script";
	std::string separator = "##";
	std::string thisID = std::to_string(windowUID);
	std::string finalLabel = label + separator + thisID;

	const IScript* scriptObject = script->GetScript();

	if (!scriptObject)
	{
		if (ImGui::ListBox(finalLabel.c_str(), &current_item, constructors.data(), static_cast<int>(constructors.size()), 5))
		{
			ChangeScript(script, constructors[current_item]);
			ENGINE_LOG("%s SELECTED, drawing its contents.", script->GetConstructName().c_str());
		}

		label = "Create Script##";
		finalLabel = label + thisID;
		if (ImGui::Button(finalLabel.c_str()))
		{
			ImGui::OpenPopup("Create new script");
		}

		ImGui::SetNextWindowSize(ImVec2(280, 75));

		if (ImGui::BeginPopupModal("Create new script", nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
		{
			OpenCreateNewScriptPopUp();
			ImGui::EndPopup();
		}
		return;
	}

	ImGui::Separator();

	std::string scriptName = script->GetConstructName().c_str();
	std::string scriptExtension = ".cpp:";
	std::string fullScriptName = scriptName + scriptExtension;
	ImGui::Text(fullScriptName.c_str());

	if (ImGui::GetWindowWidth() > static_cast<float>(fullScriptName.size()) * 13.0f)
	{
		ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);
	}

	else
	{
		ImGui::SameLine();
	}

	label = "Remove Script##";
	finalLabel = label + thisID;
	if (ImGui::Button(finalLabel.c_str()))
	{
		ENGINE_LOG("%s REMOVED, showing list of available scripts.", script->GetConstructName().c_str());

		script->SetScript(nullptr); // This deletes the script itself
		script->SetConstuctor("");	// And this makes that it is also deleted from the serialization
	}

	for (TypeFieldPair enumAndMember : scriptObject->GetFields())
	{
		ValidFieldType member = enumAndMember.second;
		switch (enumAndMember.first)
		{
			case FieldType::FLOAT:
			{
				Field<float> floatField = std::get<Field<float>>(member);
				float value = floatField.getter();

				label = floatField.name;
				finalLabel = label + separator + thisID;
				if (ImGui::DragFloat(finalLabel.c_str(), &value, 0.05f, -50.0f, 50.0f, "%.2f"))
				{
					floatField.setter(value);
				}
				break;
			}

			case FieldType::STRING:
			{
				Field<std::string> stringField = std::get<Field<std::string>>(member);
				std::string value = stringField.getter();

				label = stringField.name;
				finalLabel = label + separator + thisID;
				if (ImGui::InputText(finalLabel.c_str(), value.data(), 24))
				{
					stringField.setter(value);
				}
				break;
			}

			case FieldType::GAMEOBJECT:
			{
				Field<GameObject*> gameObjectField = std::get<Field<GameObject*>>(member);
				const GameObject* value = gameObjectField.getter();

				std::string gameObjectSlot = "Drag a GameObject here";
				if (value != nullptr)
				{
					gameObjectSlot = value->GetName().c_str();
				}

				label = gameObjectSlot;
				finalLabel = label + separator + thisID;
				ImGui::BeginDisabled();
				ImGui::Button(finalLabel.c_str(), ImVec2(208.0f, 20.0f));
				ImGui::EndDisabled();

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY"))
					{
						UID draggedGameObjectID = *(static_cast<UID*>(payload->Data));
						GameObject* draggedGameObject =
							App->GetModule<ModuleScene>()->GetLoadedScene()->SearchGameObjectByID(draggedGameObjectID);

						if (draggedGameObject)
						{
							gameObjectField.setter(draggedGameObject);
						}
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine(0.0f, 3.0f);
				ImGui::Text(gameObjectField.name.c_str());
				ImGui::SameLine();

				label = "Remove GO##";
				finalLabel = label + thisID;
				if (ImGui::Button(finalLabel.c_str()))
				{
					gameObjectField.setter(nullptr);
				}

				break;
			}

			case FieldType::BOOLEAN:
			{
				Field<bool> booleanField = std::get<Field<bool>>(member);
				bool value = booleanField.getter();

				label = booleanField.name;
				finalLabel = label + separator + thisID;
				if (ImGui::Checkbox(finalLabel.c_str(), &value))
				{
					booleanField.setter(value);
				}
				break;
			}

			default:
				break;
		}
	}
}

void WindowComponentScript::ChangeScript(ComponentScript* newScript, const char* selectedScript)
{
	newScript->SetConstuctor(selectedScript);
	IScript* Iscript = App->GetScriptFactory()->ConstructScript(selectedScript);
	Iscript->SetGameObject(component->GetOwner());
	Iscript->SetApplication(App.get());
	newScript->SetScript(Iscript);
}

void WindowComponentScript::OpenCreateNewScriptPopUp()
{
	static char name[FILENAME_MAX] = "NewScript";
	ImGui::InputText("Script Name", name, IM_ARRAYSIZE(name));
	ImGui::NewLine();

	ImGui::SameLine(ImGui::GetWindowWidth() - 120);
	if (ImGui::Button("Save", ImVec2(50, 20)))
	{
		AddNewScriptToProject(name);
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine(ImGui::GetWindowWidth() - 60);
	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}
}

void WindowComponentScript::AddNewScriptToProject(const std::string& scriptName)
{
	std::string scriptsPath = "Scripts/";

	std::string scriptHeaderPath = scriptsPath + scriptName + ".h";
	std::string scriptSourcePath = scriptsPath + scriptName + ".cpp";

	// Both header and source have the same name, so only checking the header is enough
	if (App->GetModule<ModuleFileSystem>()->Exists(scriptHeaderPath.c_str()))
	{
		ENGINE_LOG("That name is already in use, please use a different one");
		return;
	}

	ENGINE_LOG("New script %s created", scriptName.c_str());

	char* headerBuffer = nullptr;
	char* sourceBuffer = nullptr;

	App->GetModule<ModuleFileSystem>()->Load("Source/PreMades/TemplateHeaderScript", headerBuffer);
	App->GetModule<ModuleFileSystem>()->Load("Source/PreMades/TemplateSourceScript", sourceBuffer);

	std::string headerBufferAsString = headerBuffer;
	std::string sourceBufferAsString = sourceBuffer;

	ReplaceSubstringsInString(headerBufferAsString, "{0}", scriptName);
	ReplaceSubstringsInString(sourceBufferAsString, "{0}", scriptName);

	headerBuffer = headerBufferAsString.data();
	sourceBuffer = sourceBufferAsString.data();

	App->GetModule<ModuleFileSystem>()->Save(scriptHeaderPath.c_str(), headerBuffer, headerBufferAsString.size());
	App->GetModule<ModuleFileSystem>()->Save(scriptSourcePath.c_str(), sourceBuffer, sourceBufferAsString.size());
}

void WindowComponentScript::ReplaceSubstringsInString(std::string& stringToReplace,
	const std::string& from, const std::string& to)
{
	if (from.empty())
	{
		return;
	}

	size_t startPos = 0;
	while ((startPos = stringToReplace.find(from, startPos)) != std::string::npos)
	{
		stringToReplace.replace(startPos, from.length(), to);
		startPos += to.length();
	}
}