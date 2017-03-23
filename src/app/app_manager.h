#pragma once

#include "window.h"

class AppManager{
public:
	AppManager();
	~AppManager();

	bool initialize();

private:
	WindowApp *window_;
};