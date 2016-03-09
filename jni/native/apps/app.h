#ifndef APP_H
#define APP_H

#include "global.h"

class NativeApp;

struct InitParams
{
	std::string baseDir;
	// unsigned int videoWidth;
	// unsigned int videoHeight;
};

namespace App {
	enum Type
	{
		APP_TYPE_STUB    = 0,
		APP_TYPE_FLUID   = 1
	};

	void create(int appType, const InitParams& params);

	NativeApp* getInstance();
	Type getType();

} // namespace App

#endif /* APP_H */
