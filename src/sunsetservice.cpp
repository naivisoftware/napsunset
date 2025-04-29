#include <nap/logger.h>

#include "sunsetservice.h"
#include "sunsetcalculatorcomponent.h"

//#include "sunset.cpp"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SunsetService)
    RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{

	bool SunsetService::init(nap::utility::ErrorState& errorState)
	{
		Logger::info("Initializing SunsetService");

		return true;
	}

    void SunsetService::update(double deltaTime)
	{

    }
}
