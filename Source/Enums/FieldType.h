#pragma once

#include <any>
#include <vector>

class GameObject;
class StateMachine;
namespace math
{
class float3;
}

enum class FieldType
{
	FLOAT,
	STRING,
	GAMEOBJECT,
	FLOAT3,
	VECTOR,
	BOOLEAN,
	STATEMACHINE
};

template<typename T>
struct TypeToEnum
{
};

template<>
struct TypeToEnum<float>
{
	const static FieldType value = FieldType::FLOAT;
};

template<>
struct TypeToEnum<math::float3>
{
	const static FieldType value = FieldType::FLOAT3;
};

template<>
struct TypeToEnum<std::vector<std::any>>
{
	const static FieldType value = FieldType::VECTOR;
};

template<>
struct TypeToEnum<std::string>
{
	const static FieldType value = FieldType::STRING;
};

template<>
struct TypeToEnum<GameObject*>
{
	const static FieldType value = FieldType::GAMEOBJECT;
};

template<>
struct TypeToEnum<bool>
{
	const static FieldType value = FieldType::BOOLEAN;
};

template<>
struct TypeToEnum<StateMachine*>
{
	const static FieldType value = FieldType::STATEMACHINE;
};
