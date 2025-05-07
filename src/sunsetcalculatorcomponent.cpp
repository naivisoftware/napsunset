/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sunsetcalculatorcomponent.h"

#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/datetime.h>
#include <sunset.h>


RTTI_BEGIN_CLASS(nap::SunsetCalculatorComponent)
RTTI_PROPERTY("Latitude", &nap::SunsetCalculatorComponent::mLatitude, nap::rtti::EPropertyMetaData::Default, "latitude of the location we want to know the sunrise and sundown of")
RTTI_PROPERTY("Longitude", &nap::SunsetCalculatorComponent::mLongitude, nap::rtti::EPropertyMetaData::Default, "longitude of the location we want to know the sunrise and sundown of")
RTTI_PROPERTY("TimeZone", &nap::SunsetCalculatorComponent::mTimezone, nap::rtti::EPropertyMetaData::Default, "timezone to return the date in")
RTTI_PROPERTY("Minutes Offset Sun Phase", &nap::SunsetCalculatorComponent::mMinutesOffsetSunPhase, nap::rtti::EPropertyMetaData::Default, "minutes offset from the moment the sun starts transitionning ([-] in the morning ");
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
		auto* resource = getComponent<nap::SunsetCalculatorComponent>();
		mMinutesOffsetSunPhaseChange = resource->mMinutesOffsetSunPhase;
		mSunset->setPosition(resource->mLatitude, resource->mLongitude, resource->mTimezone);

		mDeltaCalculationTimer.start();
		calculateCurrentSunsetState();

        return true;
    }


	void SunsetCalculatorComponentInstance::update(double delta)
	{
		// Update sunset state when time has passed
		auto time_passed = mDeltaCalculationTimer.getSeconds().count();


		if (time_passed > mDeltaUntilNextCalculation)
			calculateCurrentSunsetState();

		// Update sun proportions every minute or when clock changes
		auto calc_diff = std::chrono::duration_cast<nap::Minutes>(getCurrentTime() - mCalcStamp);
		if (math::abs<int>(calc_diff.count()) > 0)
			calculateProp();
	}


	void SunsetCalculatorComponentInstance::calculateCurrentSunsetState()
	{
		auto now = getCurrentDateTime();
		if (mPreviousSunset == -1)
		{

			mPreviousSunset = calculatePreviousSunset(now) - static_cast<double>(mMinutesOffsetSunPhaseChange);
			mNextSunrise = calculateNextSunrise(now) + static_cast<double>(mMinutesOffsetSunPhaseChange);
			mSunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			mCurrentSunrise = mSunset->calcSunrise() + static_cast<double>(mMinutesOffsetSunPhaseChange);
			mCurrentSunset = mSunset->calcSunset() - static_cast<double>(mMinutesOffsetSunPhaseChange);
			
		}
		else
		{
			mPreviousSunset = mCurrentSunset;
			mCurrentSunrise = mNextSunrise;
			mNextSunrise = calculateNextSunrise(now);
			mSunset->setCurrentDate(now.getYear(), static_cast<int>(now.getMonth()), now.getDayInTheMonth());
			mCurrentSunset = mSunset->calcSunset();
		}

		mCurrentSunsetHours = std::chrono::duration_cast<std::chrono::hours>(std::chrono::minutes(static_cast<int>(mCurrentSunset))).count();
		mCurrentSunsetMinutes = static_cast<int>(mCurrentSunset) % 60;

		int minutes_logged = static_cast<int>(mCurrentSunrise) % 60;
		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunrise at: %.2d:%.2d", static_cast<int>(mCurrentSunrise / 60), minutes_logged);
		Logger::info("SunsetCalculatorComponentInstance::calculateCurrentSunsetState sunset at:	%.2d:%.2d", mCurrentSunsetHours, mCurrentSunsetMinutes);

		int h = now.getHour();
		int m = now.getMinute();

		// in seconds
		mDeltaUntilNextCalculation = (mCurrentSunset - (h * 60 + m)) * 60;

		// time until it's tomorrow + 1s
		auto time_until_tomorrow = static_cast<double>((24 * 60 - (h * 60 + m)) * 60 + 1);

		// check if we should recalculate when the day changes (00:00) or at the next sunset.
		if (mDeltaUntilNextCalculation > time_until_tomorrow)
			mDeltaUntilNextCalculation = time_until_tomorrow;

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
		auto current_sun_state = time_passed_since_midnight < mCurrentSunrise || time_passed_since_midnight > mCurrentSunset ?
			EState::Down : EState::Up;

		auto delta_min = static_cast<double>(mCurrentSunset - mCurrentSunrise);
		switch(current_sun_state)
		{
			case EState::Down:
			{
				if (h < 12)
				{
					// morning
					auto time_passed_since_yesterdays_sunset = static_cast<double>(h * 60 + m + 24 * 60) - mPreviousSunset;		///< in minutes
					mTimeUntilNextSunchange = static_cast<int>(mCurrentSunrise) - (h * 60 + m);
					delta_min = static_cast<double>(mCurrentSunrise + 24 * 60) - mPreviousSunset;
					mCurrentPropSun = time_passed_since_yesterdays_sunset / delta_min;
				}
				else
				{
					// evening
					auto time_passed_since_sunset = static_cast<double>(h * 60 + m) - mCurrentSunset;		///< in minutes
					mTimeUntilNextSunchange = static_cast<int>(mNextSunrise) + 24 * 60 - (h * 60 + m);
					delta_min = static_cast<double>(mNextSunrise + 24 * 60) - mCurrentSunset;
					mCurrentPropSun = time_passed_since_sunset / delta_min;

				}
				break;
			}
			case EState::Up:
			{
				mTimeUntilNextSunchange = static_cast<int>(mCurrentSunset) - (h * 60 + m);
				mCurrentPropSun = (time_passed_since_midnight - static_cast<float>(mCurrentSunrise)) / delta_min;
				break;
			}
			default:
			{
				assert(false);
				break;
			}
		}

		// Notify listeners if state changed
		if (current_sun_state != mSunState)
		{
			mSunState = current_sun_state;
			mSunStateChanged.trigger(current_sun_state);
		}

		// Store time reference
		mCalcStamp = getCurrentTime();
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
