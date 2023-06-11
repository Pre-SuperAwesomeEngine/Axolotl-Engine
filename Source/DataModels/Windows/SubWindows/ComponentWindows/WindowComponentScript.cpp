#include "WindowComponentScript.h"

#include "Application.h"
#include "Scene/Scene.h"

#include "FileSystem/ModuleFileSystem.h"
#include "Modules/ModuleScene.h"

#include "Components/ComponentScript.h"

#include "IScript.h"
#include "Math/float3.h"
#include "ScriptFactory.h"

#include "Auxiliar/Reflection/VectorField.h"

WindowComponentScript::WindowComponentScript(ComponentScript* component) :
	ComponentWindow("SCRIPT", component),
	windowUID(UniqueID::GenerateUID())
{
}

WindowComponentScript::~WindowComponentScript()
{
}

std::string WindowComponentScript::DrawStringField(std::string value, std::string name)
{
	if (ImGui::InputText(name.c_str(), value.data(), 24))
	{
		return value;
	}
	return value;
}

float WindowComponentScript::DrawFloatField(float value, std::string name)
{
	if (ImGui::DragFloat(name.c_str(), &value, 0.05f, -50.0f, 50.0f, "%.2f"))
	{
		return value;
	}
	return value;
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
		if (ImGui::ListBox(
				finalLabel.c_str(), &currentItem, constructors.data(), static_cast<int>(constructors.size()), 5))
		{
			ChangeScript(script, constructors[currentItem]);
			ENGINE_LOG("%s SELECTED, drawing its contents.", script->GetConstructName().c_str());
		}

		label = "Create Script##";
		finalLabel = label + thisID;
		if (ImGui::Button(finalLabel.c_str()))
		{
			ImGui::OpenPopup("Create new script");
		}

		ImGui::SetNextWindowSize(ImVec2(280, 75));

		if (ImGui::BeginPopupModal(
				"Create new script", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
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
		ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
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

		script->SetScript(nullptr);			  // This deletes the script itself
		script->SetConstuctor(std::string()); // And this makes it so it's also deleted from the serialization
	}

	for (std::size_t index = 0; TypeFieldPair enumAndMember : scriptObject->GetFields())
	{
		ValidFieldType member = enumAndMember.second;
		//DrawField(member, enumAndMember.first);
		switch (enumAndMember.first)
		{
			case FieldType::FLOAT:
			{
				Field<float> floatField = std::get<Field<float>>(member);
				float value = floatField.getter();

				label = floatField.name;
				finalLabel = label + separator + thisID;

				floatField.setter(DrawFloatField(value, finalLabel.c_str()));
				break;
			}

			case FieldType::VECTOR3:
			{
				Field<float3> float3Field = std::get<Field<float3>>(member);
				float3 value = float3Field.getter();
				if (ImGui::DragFloat3(
						float3Field.name.c_str(), (&value[2], &value[1], &value[0]), 0.05f, -50.0f, 50.0f, "%.2f"))
				{
					float3Field.setter(value);
				}
				break;
			}

			case FieldType::VECTOR:
			{
				VectorField vectorField = std::get<VectorField>(member);
				std::vector<std::any> vectorValue = vectorField.getter();

				for (const auto& elem : vectorValue)
				{
					if (elem.type() == typeid(float)) 
					{
						float floatValue = std::any_cast<float>(elem);
						std::vector<float> value;
						for (const auto& elem : vectorValue) {
							try {
								float floatValue = std::any_cast<float>(elem);
								value.push_back(floatValue);
							}
							catch (const std::bad_any_cast&) {
							}
						}
						for (int i = 0; i < value.size(); i++)
						{
							vectorValue[i] = DrawFloatField(value[i], (vectorField.name + std::to_string(i)));

							vectorField.setter(vectorValue);
						}
					}
					else if (elem.type() == typeid(std::string))
					{
						std::string stringValue = std::any_cast<std::string>(elem);
						std::vector<std::string> value;
						for (const auto& elem : vectorValue) {
							try {
								std::string stringValue = std::any_cast<std::string>(elem);
								value.push_back(stringValue);
							}
							catch (const std::bad_any_cast&) {
							}
						}
						for (int i = 0; i < value.size(); i++) 
						{
							vectorValue[i] = std::string(DrawStringField(value[i], (vectorField.name + std::to_string(i)).c_str()).data());
							
							vectorField.setter(vectorValue);
						}
							
					}

					else if (elem.type() == typeid(GameObject*))
					{
						GameObject* gameObjectValue = std::any_cast<GameObject*>(elem);
						std::vector<GameObject*> value;
						for (const auto& elem : vectorValue) {
							try {
								GameObject* gameObjectValue = std::any_cast<GameObject*>(elem);
								value.push_back(gameObjectValue);
							}
							catch (const std::bad_any_cast&) {
							}
						}
						for (int i = 0; i < value.size(); i++)
						{
							//DO THE GAMEOBJECT IMGUI
						}

					}
					// Add more type checks for other supported types

					// ...
					break;
				}

				break;
			}


			case FieldType::STRING:
			{
				Field<std::string> stringField = std::get<Field<std::string>>(member);
				std::string value = stringField.getter();

				label = stringField.name;
				finalLabel = label + separator + thisID;

				stringField.setter(DrawStringField(value, finalLabel.c_str()));
				
				break;
			}

			case FieldType::GAMEOBJECT:
			{
				Field<GameObject*> gameObjectField = std::get<Field<GameObject*>>(member);
				GameObject* value = gameObjectField.getter();

				std::string gameObjectSlot = "Drag a GameObject here";
				if (value != nullptr)
				{
					gameObjectSlot = value->GetName().c_str();
				}

				label = gameObjectSlot;
				finalLabel = label + separator + thisID;
				ImGui::Button(finalLabel.c_str(), ImVec2(208.0f, 20.0f));

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY"))
					{
						UID draggedGameObjectID = *(UID*) payload->Data;
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

				finalLabel = "Remove GO##" + thisID + std::to_string(index);
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
		++index;
	}
}

void WindowComponentScript::ChangeScript(ComponentScript* newScript, const char* selectedScript)
{
	newScript->SetConstuctor(selectedScript);
	IScript* Iscript = App->GetScriptFactory()->ConstructScript(selectedScript);
	Iscript->SetOwner(component->GetOwner());
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
		// reset the input string contents
		strcpy_s(name, "NewScript");
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

	const ModuleFileSystem* fileSystem = App->GetModule<ModuleFileSystem>();

	// Both header and source have the same name, so only checking the header is enough
	if (fileSystem->Exists(scriptHeaderPath.c_str()))
	{
		ENGINE_LOG("That name is already in use, please use a different one");
		return;
	}

	char* headerBuffer = nullptr;
	char* sourceBuffer = nullptr;

	unsigned int headerBufferSize = fileSystem->Load("Source/PreMades/TemplateHeaderScript", headerBuffer);
	unsigned int sourceBufferSize = fileSystem->Load("Source/PreMades/TemplateSourceScript", sourceBuffer);

	std::string headerString(headerBuffer, headerBufferSize);
	delete headerBuffer;
	std::string sourceString(sourceBuffer, sourceBufferSize);
	delete sourceBuffer;

	ReplaceSubstringsInString(headerString, "{0}", scriptName);
	ReplaceSubstringsInString(sourceString, "{0}", scriptName);

	fileSystem->Save(scriptHeaderPath.c_str(), headerString.c_str(), headerString.size());
	fileSystem->Save(scriptSourcePath.c_str(), sourceString.c_str(), sourceString.size());

	ENGINE_LOG("New script %s created", scriptName.c_str());
}

void WindowComponentScript::ReplaceSubstringsInString(std::string& stringToReplace,
													  const std::string& from,
													  const std::string& to)
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