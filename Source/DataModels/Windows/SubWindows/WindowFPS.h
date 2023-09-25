#pragma once

#include "SubWindow.h"

class WindowFPS : public SubWindow
{
public:
	WindowFPS();
	~WindowFPS() override;
	
	int GetFps() const;

protected:
	void DrawWindowContents() override;

private:
	int fpsCaptures;
	int timeCaptures;
	int currentFpsIndex;
	int currentTimeIndex;
	std::vector<float> fpsHist;
	std::vector<float> timeHist;
};

inline int WindowFPS::GetFps() const
{
	return currentFpsIndex;
};
