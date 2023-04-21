#pragma once
#include "Module.h"

constexpr auto ROOTPATH = "Source/Shaders/";

class Program;

enum class ProgramType 
{
	DEFAULT,
	SPECULAR,
	HIGHLIGHT,
	SKYBOX,
	SPRITE,
	IRRADIANCE_MAP,
	HDR_TO_CUBEMAP,
	PRE_FILTERED_MAP,
	PROGRAM_TYPE_SIZE
};

class ModuleProgram : public Module
{
public:
	ModuleProgram();
	~ModuleProgram() override;

	bool Start() override;
	bool CleanUp() override;

	void UpdateProgram(const std::string& vtxShaderFileName, 
		const std::string& frgShaderFileName, int programType,
		const std::string programName);

	Program* GetProgram(ProgramType type) const;

private:

	std::unique_ptr<Program> CreateProgram
		(const std::string vtxShaderFileName, 
			const std::string frgShaderFileName,const std::string programName);

	std::string LoadShaderSource(const std::string& shaderFileName);
	unsigned CompileShader(unsigned type, const std::string& source);

	std::vector<std::unique_ptr<Program> > programs;
	std::string rootPath = "Source/Shaders/";
};

inline Program* ModuleProgram::GetProgram(ProgramType type) const
{
	if (programs.empty() || programs.size() 
				> static_cast<int>(ProgramType::PROGRAM_TYPE_SIZE))
	{
		return nullptr;
	}
	else
	{
		return programs[static_cast<int>(type)].get();
	}
}
