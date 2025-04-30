#include "sunsetcalculatorcomponent.h"

#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include "nap/datetime.h"

#include "Sunset.cpp"

std::unique_ptr <SunSet> sunset;

RTTI_BEGIN_CLASS(nap::SunsetCalculatorComponent)
	RTTI_PROPERTY("latitude", &nap::SunsetCalculatorComponent::mLatitude, nap::rtti::EPropertyMetaData::Default, "latitude of the location we want to know the sunrise and sundown of")
	RTTI_PROPERTY("longitude", &nap::SunsetCalculatorComponent::mLongitude, nap::rtti::EPropertyMetaData::Default, "longitude of the location we want to know the sunrise and sundown of")
	RTTI_PROPERTY("timezone", &nap::SunsetCalculatorComponent::mTimezone, nap::rtti::EPropertyMetaData::Default, "timezone to return the date in")
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
		mDeltaUntilNextCalculation = 10; // settings this to 10s so to not retrigger the calculation of the sunset until mDeltaUntilNextCalculation is properly set inside calculateCurrentSunsetState
	}

	bool SunsetCalculatorComponentInstance::init(utility::ErrorState& errorState)
    {
		mCurrentPropSun = -1;
		// 1h extra to the time of the starting of the sun setting down --> the time the night is dark
		mOffsetTimeSunsettingDown = 1;

		nap::SunsetCalculatorComponent* resource = getComponent<nap::SunsetCalculatorComponent>();

		sunset->setPosition(resource->mLatitude, resource->mLongitude, resource->mTimezone);

		mDeltaCalculationTimer.start();
		calculateCurrentSunsetState();
        return true;
    }

	void SunsetCalculatorComponentInstance::update(double delta)
	{
		double timePassed = mDeltaCalculationTimer.getElapsedTime();
		if (timePassed > mDeltaUntilNextCalculation) calculateCurrentSunsetState();

	}

	void SunsetCalculatorComponentInstance::calculateCurrentSunsetState() {

		auto now = getCurrentDateTime();

		if (mPreviousSunset == -1) {
			mPreviousSunset = calculatePreviousSunset(now);
			mNextSunrise = calculateNextSunrise(now);
			sunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			mCurrentSunrise = sunset->calcSunrise();
			mCurrentSunset = sunset->calcSunset();
			
		}
		else
		{
			mPreviousSunset = mCurrentSunset;
			mCurrentSunrise = mNextSunrise;

			mNextSunrise = calculateNextSunrise(now);
			sunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			mCurrentSunset = sunset->calcSunset();
		}



		mCurrentSunsetHours = static_cast<int>(mCurrentSunset / 60 + mOffsetTimeSunsettingDown) % 24;
		mCurrentSunsetMinutes = static_cast<int>(mCurrentSunset) % 60;

		std::string minutesLogged = std::to_string((static_cast<int>(mCurrentSunrise) % 60));
		if (minutesLogged.size() < 2) minutesLogged.insert(0,1,'0');

		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunrise at :	%d:%s", static_cast<int>(mCurrentSunrise / 60), minutesLogged.c_str());
		minutesLogged = std::to_string(mCurrentSunsetMinutes);
		if (minutesLogged.size() < 2) minutesLogged.insert(0, 1, '0');
		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunset at  :	%d:%s", mCurrentSunsetHours, minutesLogged.c_str());


		int h = now.getHour();
		int m = now.getMinute();

		// in seconds
		mDeltaUntilNextCalculation = (mCurrentSunset + mOffsetTimeSunsettingDown * 60 - (h * 60 + m)) * 60;

		mDeltaCalculationTimer.reset();
	}


	void SunsetCalculatorComponentInstance::calculateProp() {
		auto now = getCurrentDateTime();
		int h = now.getHour();
		int m = now.getMinute();

		int timePassedSinceMidnight = h * 60 + m;

		mSunIsCurrentlyUp = true;
		if (timePassedSinceMidnight < mCurrentSunrise) mSunIsCurrentlyUp = false;


		int delta_min = static_cast<int>(mCurrentSunset - mCurrentSunrise);

		if (!mSunIsCurrentlyUp) {
			if (h < 12) { // morning
				int timePassedSinceyesterdaysSunDown = h * 60 + m + (24 * 60 - (mPreviousSunset + mOffsetTimeSunsettingDown * 60));
				delta_min = static_cast<int>(mCurrentSunrise - mPreviousSunset + mOffsetTimeSunsettingDown * 60);
				mCurrentPropSun = timePassedSinceyesterdaysSunDown / delta_min;
			}
			else
			{ // evening
				int timePassedSinceSunDown = h * 60 + m + (mCurrentSunset + mOffsetTimeSunsettingDown * 60);
				delta_min = static_cast<int>(mNextSunrise + 24 * 60 - (mCurrentSunset + mOffsetTimeSunsettingDown * 60));
				mCurrentPropSun = timePassedSinceSunDown / delta_min;

			}
		}
		else
		{

			mCurrentPropSun = (timePassedSinceMidnight - static_cast<float>(mCurrentSunrise)) / delta_min;
		}
	}

	double SunsetCalculatorComponentInstance::calculatePreviousSunset(DateTime date) {

		// go back one day using std::chrono
		SystemTimeStamp sysTime = date.getTimeStamp();
		sysTime -= std::chrono::hours(24);
		date.setTimeStamp(sysTime);

		sunset->setCurrentDate(date.getYear(), static_cast<int>(date.getMonth()), date.getDayInTheMonth());
		return sunset->calcSunset();
		
	}

	double SunsetCalculatorComponentInstance::calculateNextSunrise(DateTime date) {

		// go forward one day using std::chrono
		SystemTimeStamp sysTime = date.getTimeStamp();
		sysTime += std::chrono::hours(24);
		date.setTimeStamp(sysTime);

		sunset->setCurrentDate(date.getYear(), static_cast<int>(date.getMonth()), date.getDayInTheMonth());
		return sunset->calcSunrise();

	}

}
