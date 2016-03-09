#include "app.h"

#include "native_app.h"
#include "apps/fluids/fluids_app.h"

namespace App {
	std::unique_ptr<NativeApp> instance;
	App::Type type;

	void create(int appType, const InitParams& params)
	{
		switch (appType) {
			case APP_TYPE_STUB:
				instance.reset(new NativeApp(params));
				break;

			case APP_TYPE_FLUID:
				instance.reset(new FluidMechanics(params));
				break;

				// TODO: other types...

			default:
				throw std::runtime_error("Unknown application type: " + Utility::toString(appType));
		}

		type = Type(appType);
	}

	NativeApp* getInstance()
	{
		return instance.get();
	}

	Type getType()
	{
		return type;
	}

} // namespace App
