// Local Includes
#include "sunsetexampleservice.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::sunsetexampleService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	bool sunsetexampleService::init(nap::utility::ErrorState& errorState)
	{
		//Logger::info("Initializing sunsetexampleService");
		return true;
	}


	void sunsetexampleService::update(double deltaTime)
	{
	}
	

	void sunsetexampleService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}
	

	void sunsetexampleService::shutdown()
	{
	}
}
