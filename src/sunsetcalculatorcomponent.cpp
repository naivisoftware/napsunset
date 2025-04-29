#include "sunsetcalculatorcomponent.h"

#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include "nap/datetime.h"

#include "Sunset.cpp"

std::unique_ptr <SunSet> sunset;

RTTI_BEGIN_CLASS(nap::SunsetCalculatorComponent)
	RTTI_PROPERTY("latitude", &nap::SunsetCalculatorComponent::latitude, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("longitude", &nap::SunsetCalculatorComponent::longitude, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("timezone", &nap::SunsetCalculatorComponent::timezone, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SunsetCalculatorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{   
	SunsetCalculatorComponentInstance::SunsetCalculatorComponentInstance(EntityInstance& entity, Component& resource):
	ComponentInstance(entity, resource)
	{
		sunset = std::make_unique<SunSet>();
		accumulatedTime = 0.0;
		deltaUntilNextCalculation =1000;
	}

	bool SunsetCalculatorComponentInstance::init(utility::ErrorState& errorState)
    {
		currentPropSunUp = 0;
		// 1h extra to the time of the starting of the sun setting down --> the time the night is dark
		offsetTimeSunsettingDown = 1;

		nap::SunsetCalculatorComponent* resource = getComponent<nap::SunsetCalculatorComponent>();

		sunset->setPosition(resource->latitude, resource->longitude, resource->timezone);

		calculateCurrentSunsetState();
        return true;
    }

	void SunsetCalculatorComponentInstance::update(double delta)
	{

		accumulatedTime += delta;
		if (deltaUntilNextCalculation > 0) {
			if (accumulatedTime > deltaUntilNextCalculation) {
				calculateCurrentSunsetState();
			}
		}
		// old method
		/*auto now = getCurrentDateTime();
		int h = (now.getHour()+ offsetTimeSunsettingDown) %24;
		int m = now.getMinute();

		if (currentSunsetHours == h && currentSunsetMinutes == m) {
			
		}*/

	}

	void SunsetCalculatorComponentInstance::calculateCurrentSunsetState() {



		auto now = getCurrentDateTime();

		sunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
		currentSunrise = sunset->calcSunrise();
		currentSunset = sunset->calcSunset();

		currentSunsetHours = static_cast<int>(((currentSunset + offsetTimeSunsettingDown)/ 60)%24);
		currentSunsetMinutes = static_cast<int>(currentSunset) % 60;


		Logger::info("SunsetCalculatorComponentInstance::Sunset sunrise at : %d:%d", static_cast<int>(currentSunrise /60), static_cast<int>(currentSunrise)%60);
		Logger::info("SunsetCalculatorComponentInstance::Sunset sunset at : %d:%d", currentSunsetHours, currentSunsetMinutes);


		int h = now.getHour();
		int m = now.getMinute();

		// milliseconds
		deltaUntilNextCalculation = (currentSunsetHours + offsetTimeSunsettingDown - (h * 60 + m)) * 60 * 1000;
		
	}

	float SunsetCalculatorComponentInstance::getProp() {


		auto now = getCurrentDateTime();
		int h = now.getHour();
		int m = now.getMinute();

		int timePassedSinceMidnight = h * 60 + m;
		int delta_min = static_cast<int>(currentSunset - currentSunrise);
		currentPropSunUp = (timePassedSinceMidnight - static_cast<float>(currentSunrise)) / delta_min;

		return currentPropSunUp;
	}


}
