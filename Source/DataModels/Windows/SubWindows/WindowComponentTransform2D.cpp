#include "ComponentWindows/WindowComponentTransform2D.h"

#include "ComponentTransform2D.h"

#include "Application.h"


WindowComponentTransform2D::WindowComponentTransform2D(ComponentTransform2D * component) :
	ComponentWindow("Transform 2D", component)
{
}

WindowComponentTransform2D::~WindowComponentTransform2D()
{
}

void WindowComponentTransform2D::DrawWindowContents()
{
	ComponentTransform2D* asTransform = static_cast<ComponentTransform2D*>(component);

	if (asTransform)
	{
		/*
		currentTranslation = asTransform->GetPosition();
		currentRotation = asTransform->GetRotationXYZ();
		currentScale = asTransform->GetScale();

		currentDragSpeed = 0.025f;

		translationModified = false;
		rotationModified = false;
		scaleModified = false;

		DrawTransformTable();

		UpdateComponentTransform();
		*/
	}
}


void WindowComponentTransform2D::DrawTransformTable()
{
	/*
	if (ImGui::BeginTable("TransformTable", 2))
	{
		ImGui::TableNextColumn();
		ImGui::Text("Translation"); ImGui::SameLine();

		ImGui::TableNextColumn();
		ImGui::Text("x:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##XTrans", &currentTranslation.x, currentDragSpeed,
			std::numeric_limits<float>::min(), std::numeric_limits<float>::min()))
		{
			translationModified = true;
		}
		ImGui::PopStyleVar(); ImGui::SameLine();

		ImGui::Text("y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##YTrans", &currentTranslation.y, currentDragSpeed,
			std::numeric_limits<float>::min(), std::numeric_limits<float>::min()))
		{
			translationModified = true;
		}
		ImGui::PopStyleVar(); ImGui::SameLine();

		ImGui::Text("z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##ZTrans", &currentTranslation.z, currentDragSpeed,
			std::numeric_limits<float>::min(), std::numeric_limits<float>::min()))
		{
			translationModified = true;
		}
		ImGui::PopStyleVar();

		ImGui::TableNextColumn();
		ImGui::Text("Rotation"); ImGui::SameLine();

		ImGui::TableNextColumn();
		ImGui::Text("x:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##XRot", &currentRotation.x, currentDragSpeed,
			std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), "%0.3f"))
		{
			rotationModified = true;
		}
		ImGui::PopStyleVar(); ImGui::SameLine();

		ImGui::Text("y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##YRot", &currentRotation.y, currentDragSpeed,
			std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), "%0.3f"))
		{
			rotationModified = true;
		}
		ImGui::PopStyleVar(); ImGui::SameLine();

		ImGui::Text("z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##ZRot", &currentRotation.z, currentDragSpeed,
			std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), "%0.3f"))
		{
			rotationModified = true;
		}
		ImGui::PopStyleVar();

		ImGui::TableNextColumn();
		ImGui::Text("Scale"); ImGui::SameLine();

		ImGui::TableNextColumn();
		ImGui::Text("x:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##XScale", &currentScale.x, currentDragSpeed,
			0.0001f, std::numeric_limits<float>::max()))
		{
			scaleModified = true;
		}
		ImGui::PopStyleVar(); ImGui::SameLine();

		ImGui::Text("y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##YScale", &currentScale.y, currentDragSpeed,
			0.0001f, std::numeric_limits<float>::max()))
		{
			scaleModified = true;
		}
		ImGui::PopStyleVar(); ImGui::SameLine();

		ImGui::Text("z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
		if (ImGui::DragFloat("##ZScale", &currentScale.z, currentDragSpeed,
			0.0001f, std::numeric_limits<float>::max()))
		{
			scaleModified = true;
		}
		ImGui::PopStyleVar();

		ImGui::EndTable();
	}
		*/
}