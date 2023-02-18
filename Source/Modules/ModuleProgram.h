#pragma once
#include "Module.h"

class Program;

enum class ProgramType {
	MESHSHADER,
	SKYBOX,
	PROGRAM_TYPE_SIZE
};

class ModuleProgram : public Module
{
public:
	ModuleProgram();
	~ModuleProgram() override;

	bool Start() override;

	void UpdateProgram(std::string& vtxShaderFileName, std::string& frgShaderFileName, int programType,
		std::string programName);

	bool CleanUp() override;

	Program* GetProgram(ProgramType type) const;

private:
	std::unique_ptr<Program> CreateProgram(std::string vtxShaderFileName, std::string frgShaderFileName,
		std::string programName);

	char* LoadShaderSource(const char* shaderFileName);
	unsigned CompileShader(unsigned type, const char* source);

	std::vector<std::unique_ptr<Program> > Programs;
	std::string RootPath = "Lib/Shaders/";
};

inline Program* ModuleProgram::GetProgram(ProgramType type) const
{
	return Programs[(int)type].get();
}


