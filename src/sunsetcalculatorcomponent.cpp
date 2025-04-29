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
		deltaUntilNextCalculation = -1;
	}

	bool SunsetCalculatorComponentInstance::init(utility::ErrorState& errorState)
    {
		currentPropSun = -1;
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
	}

	void SunsetCalculatorComponentInstance::calculateCurrentSunsetState() {

		auto now = getCurrentDateTime();

		if (previousSunset == -1) {
			previousSunset = calculatePreviousSunset(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			
		}
		else {
			previousSunset = currentSunset;
		}

		sunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());

		currentSunrise = sunset->calcSunrise();
		currentSunset = sunset->calcSunset();

		currentSunsetHours = static_cast<int>(currentSunset / 60 + offsetTimeSunsettingDown) % 24;
		currentSunsetMinutes = static_cast<int>(currentSunset) % 60;

		std::string minutesLogged = std::to_string((static_cast<int>(currentSunrise) % 60));
		if (minutesLogged.size() < 2)minutesLogged.insert(0,1,'0');

		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunrise at :	%d:%s", static_cast<int>(currentSunrise / 60), minutesLogged.c_str());
		minutesLogged = std::to_string(currentSunsetMinutes);
		if (minutesLogged.size() < 2)minutesLogged.insert(0, 1, '0');
		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunset at  :	%d:%s", currentSunsetHours, minutesLogged.c_str());


		int h = now.getHour();
		int m = now.getMinute();

		// in seconds
		deltaUntilNextCalculation = (currentSunset + offsetTimeSunsettingDown*60 - (h * 60 + m)) * 60;
		deltaUntilNextCalculation = 10;
		accumulatedTime = 0;
	}

	float SunsetCalculatorComponentInstance::getProp() {


		auto now = getCurrentDateTime();
		int h = now.getHour();
		int m = now.getMinute();

		int timePassedSinceMidnight = h * 60 + m;

		sunIsCurrentlyUp = true;
		if (timePassedSinceMidnight < currentSunrise)sunIsCurrentlyUp = false;


		int delta_min = static_cast<int>(currentSunset - currentSunrise);

		if (!sunIsCurrentlyUp) {
			int timePassedSinceSunDown = h * 60 + m + (24 * 60 - (previousSunset + offsetTimeSunsettingDown * 60));
			delta_min = static_cast<int>(currentSunrise - previousSunset + offsetTimeSunsettingDown * 60);
			currentPropSun = timePassedSinceSunDown / delta_min;
		}
		else {

			currentPropSun = timePassedSinceMidnight - static_cast<float>(currentSunrise) / delta_min;
		}

		return currentPropSun;
	}


	double SunsetCalculatorComponentInstance::calculatePreviousSunset(int year, int month, int day) {
		// go back one day
		
		if (day > 1) {
			day -= 1;
		}
		else {
			if (month > 1) {
				month -= 1;
			}
			else {
				year -= 1;
			}
		}

		sunset->setCurrentDate(year, month, day);
		return sunset->calcSunset();
		
	}

}
