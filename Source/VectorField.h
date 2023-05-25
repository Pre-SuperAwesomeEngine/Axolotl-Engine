#pragma once

#include <vector>
#include <any>
#include <type_traits>

struct VectorField : public Field<std::vector<std::any>>
{
public:
	FieldType innerType;

	VectorField(const std::string& name,
		const std::function<std::vector<std::any>(void)>& getter,
		const std::function<void(const std::vector<std::any>&)>& setter,
		FieldType innerType) :
		Field<std::vector<std::any>>(name, getter, setter),
		innerType(innerType)
	{
	}
};
