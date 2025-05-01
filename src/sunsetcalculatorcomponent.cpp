#include "sunsetcalculatorcomponent.h"

#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include "nap/datetime.h"

#include "Sunset.cpp"

RTTI_BEGIN_CLASS(nap::SunsetCalculatorComponent)
RTTI_PROPERTY("latitude", &nap::SunsetCalculatorComponent::mLatitude, nap::rtti::EPropertyMetaData::Default, "latitude of the location we want to know the sunrise and sundown of")
RTTI_PROPERTY("longitude", &nap::SunsetCalculatorComponent::mLongitude, nap::rtti::EPropertyMetaData::Default, "longitude of the location we want to know the sunrise and sundown of")
RTTI_PROPERTY("timezone", &nap::SunsetCalculatorComponent::mTimezone, nap::rtti::EPropertyMetaData::Default, "timezone to return the date in")
RTTI_PROPERTY("minutes offset to sundown", &nap::SunsetCalculatorComponent::mMinutesOffsetSunDown, nap::rtti::EPropertyMetaData::Default, "sundown offset from the moment the sun starts to set down");
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SunsetCalculatorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{   
	SunsetCalculatorComponentInstance::SunsetCalculatorComponentInstance(EntityInstance& entity, Component& resource):
	ComponentInstance(entity, resource),
		mSunset(std::make_unique<SunSet>())
	{
	}


	bool SunsetCalculatorComponentInstance::init(utility::ErrorState& errorState)
    {
		nap::SunsetCalculatorComponent* resource = getComponent<nap::SunsetCalculatorComponent>();

		mMinutesOffsetTimeSunsettingDown = resource->mMinutesOffsetSunDown;
		mSunset->setPosition(resource->mLatitude, resource->mLongitude, resource->mTimezone);

		mDeltaCalculationTimer.start();
		calculateCurrentSunsetState();
        return true;
    }

	void SunsetCalculatorComponentInstance::update(double delta)
	{
		calculateProp();
		if (mDeltaCalculationTimer.getElapsedTime() > mDeltaUntilNextCalculation) calculateCurrentSunsetState();

	}

	void SunsetCalculatorComponentInstance::calculateCurrentSunsetState()
	{

		auto now = getCurrentDateTime();

		if (mPreviousSunset == -1)
		{
			mPreviousSunset = calculatePreviousSunset(now);
			mNextSunrise = calculateNextSunrise(now);
			mSunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			mCurrentSunrise = mSunset->calcSunrise();
			mCurrentSunset = mSunset->calcSunset();
			
		}
		else
		{
			mPreviousSunset = mCurrentSunset;
			mCurrentSunrise = mNextSunrise;

			mNextSunrise = calculateNextSunrise(now);
			mSunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			mCurrentSunset = mSunset->calcSunset();
		}

		mCurrentSunsetHours = std::chrono::duration_cast<std::chrono::hours>(std::chrono::minutes(static_cast<int>(mCurrentSunset) + mMinutesOffsetTimeSunsettingDown)).count();

		mCurrentSunsetMinutes = static_cast<int>(mCurrentSunset) % 60;

		std::string minutes_logged = std::to_string((static_cast<int>(mCurrentSunrise) % 60));
		if (minutes_logged.size() < 2) minutes_logged.insert(0,1,'0');

		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunrise at :	%d:%s", static_cast<int>(mCurrentSunrise / 60), minutes_logged.c_str());
		minutes_logged = std::to_string(mCurrentSunsetMinutes);
		if (minutes_logged.size() < 2) minutes_logged.insert(0, 1, '0');
		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunset at  :	%d:%s", mCurrentSunsetHours, minutes_logged.c_str());


		int h = now.getHour();
		int m = now.getMinute();

		// in seconds
		mDeltaUntilNextCalculation = (mCurrentSunset + mMinutesOffsetTimeSunsettingDown - (h * 60 + m)) * 60;

		// time until it's tomorrow + 1s
		const double time_until_tomorrow = static_cast<double>((24 * 60 - (h * 60 + m)) * 60 + 1);

		// check if we should recalculate when the day changes (00:00) or at the next sunset.
		if (mDeltaUntilNextCalculation > time_until_tomorrow)mDeltaUntilNextCalculation = time_until_tomorrow;

		mDeltaCalculationTimer.reset();
	}


	void SunsetCalculatorComponentInstance::calculateProp()
	{
		auto now = getCurrentDateTime();
		int h = now.getHour();
		int m = now.getMinute();

		const double time_passed_since_midnight = static_cast<const double> (h * 60 + m);

		mSunIsCurrentlyUp = true;
		if (time_passed_since_midnight < mCurrentSunrise) mSunIsCurrentlyUp = false;


		double delta_min = static_cast<double>(mCurrentSunset - mCurrentSunrise);

		if (!mSunIsCurrentlyUp)
		{
			if (h < 12)
			{ // morning
				int time_passed_since_yesterdays_sundown = h * 60 + m + (24 * 60 - (mPreviousSunset + mMinutesOffsetTimeSunsettingDown));		///< in minutes
				delta_min = static_cast<double>(mCurrentSunrise - mPreviousSunset + mMinutesOffsetTimeSunsettingDown);
				mCurrentPropSun = time_passed_since_yesterdays_sundown / delta_min;
			}
			else
			{ // evening
				double time_passed_since_sundown = static_cast<double> (h * 60 + m + mMinutesOffsetTimeSunsettingDown) + mCurrentSunset;		///< in minutes
				delta_min = static_cast<double>(mNextSunrise + 24 * 60 - (mCurrentSunset + mMinutesOffsetTimeSunsettingDown));
				mCurrentPropSun = time_passed_since_sundown / delta_min;

			}
		}
		else
		{
			mCurrentPropSun = (time_passed_since_midnight - static_cast<float>(mCurrentSunrise)) / delta_min;
		}
	}

	double SunsetCalculatorComponentInstance::calculatePreviousSunset(DateTime date)
	{

		// go back one day using std::chrono
		SystemTimeStamp sysTime = date.getTimeStamp();
		sysTime -= std::chrono::hours(24);
		date.setTimeStamp(sysTime);

		mSunset->setCurrentDate(date.getYear(), static_cast<int>(date.getMonth()), date.getDayInTheMonth());
		return mSunset->calcSunset();
		
	}

	double SunsetCalculatorComponentInstance::calculateNextSunrise(DateTime date)
	{

		// go forward one day using std::chrono
		SystemTimeStamp sysTime = date.getTimeStamp();
		sysTime += std::chrono::hours(24);
		date.setTimeStamp(sysTime);

		mSunset->setCurrentDate(date.getYear(), static_cast<int>(date.getMonth()), date.getDayInTheMonth());
		return mSunset->calcSunrise();

	}

}
