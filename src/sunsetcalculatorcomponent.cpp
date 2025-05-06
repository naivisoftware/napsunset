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
	SunsetCalculatorComponentInstance::SunsetCalculatorComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource),
		mSunset(std::make_unique<SunSet>())
	{ }



	// this is needed for the PIMPL (Pointer To Implementation) to work with the unique_ptr to Sunset in the header
	SunsetCalculatorComponentInstance::~SunsetCalculatorComponentInstance() { }


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
		const long time_passed_since_calculations = static_cast<long>(mDeltaCalculationTimer.getElapsedTime());
		if(time_passed_since_calculations%60==0)calculateProp();

		if (time_passed_since_calculations > mDeltaUntilNextCalculation) calculateCurrentSunsetState();

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

		// let's calculate the proportion right now, so
		// [1] on init it appears right away
		// [2] when the day or sunset change they appear right away
		calculateProp();

	}


	void SunsetCalculatorComponentInstance::calculateProp()
	{
		auto now = getCurrentDateTime();
		int h = now.getHour();
		int m = now.getMinute();

		auto time_passed_since_midnight = static_cast<double> (h * 60 + m);

		mSunIsCurrentlyUp = true;

		bool sun_up_in_the_sky = true;
		if (time_passed_since_midnight < mCurrentSunrise || time_passed_since_midnight > mCurrentSunset) sun_up_in_the_sky = false;
		
		if (sun_up_in_the_sky != mSunIsCurrentlyUp)
		{
			mSunIsCurrentlyUp = sun_up_in_the_sky;
			mSunIsUp.trigger(mSunIsCurrentlyUp);
		}


		auto delta_min = static_cast<double>(mCurrentSunset - mCurrentSunrise);

		if (!mSunIsCurrentlyUp)
		{
			if (h < 12)
			{ // morning
				int time_passed_since_yesterdays_sunset = h * 60 + m + 24 * 60 - (static_cast<int>(mPreviousSunset) + mMinutesOffsetTimeSunsettingDown);		///< in minutes
				mTimeUntilNextSunchange = static_cast<int>(mCurrentSunrise + h * 60 + m);
				delta_min = static_cast<double>(mCurrentSunrise + 24 * 60 - (mPreviousSunset + mMinutesOffsetTimeSunsettingDown));
				mCurrentPropSun = time_passed_since_yesterdays_sunset / delta_min;
			}
			else
			{ // evening
				auto time_passed_since_sunset = static_cast<double>(h * 60 + m - (mCurrentSunset + mMinutesOffsetTimeSunsettingDown));		///< in minutes
				mTimeUntilNextSunchange = static_cast<int>(mNextSunrise + 24 * 60 -(h * 60 + m));
				delta_min = static_cast<double>(mNextSunrise + 24 * 60 - (mCurrentSunset + mMinutesOffsetTimeSunsettingDown));
				mCurrentPropSun = time_passed_since_sunset / delta_min;

			}
		}
		else
		{
			mTimeUntilNextSunchange = static_cast<int>(mCurrentSunset + mMinutesOffsetTimeSunsettingDown) - (h * 60 + m);
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
