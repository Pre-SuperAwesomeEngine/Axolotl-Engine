#include "Application.h"
#include "CameraSample.h"
#include "Components/Component.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentScript.h"

#include "debugdraw.h"

REGISTERCLASS(CameraSample);

CameraSample::CameraSample() : Script(), 
	position(float3::zero),
	influenceRadius(1.0f),
	positionOffset(float3::zero)
{
	REGISTER_FIELD(influenceRadius, float);
	REGISTER_FIELD(positionOffset, float3);
}

void CameraSample::Start()
{
	position = owner->GetComponent<ComponentTransform>()->GetGlobalPosition();
}

void CameraSample::PreUpdate(float deltaTime)
{
#ifdef ENGINE
	dd::sphere(position, dd::colors::Yellow, influenceRadius);
#endif // ENGINE
}



