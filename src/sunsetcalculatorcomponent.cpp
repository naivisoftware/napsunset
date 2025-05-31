/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sunsetcalculatorcomponent.h"

#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/datetime.h>
#include <sunset.h>

RTTI_BEGIN_ENUM(nap::SunsetCalculatorComponentInstance::EState)
	RTTI_ENUM_VALUE(nap::SunsetCalculatorComponentInstance::EState::Down,		"Down"),
	RTTI_ENUM_VALUE(nap::SunsetCalculatorComponentInstance::EState::Up,			"Up"),
	RTTI_ENUM_VALUE(nap::SunsetCalculatorComponentInstance::EState::Unknown,	"Unknown")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::SunsetCalculatorComponent)
	RTTI_PROPERTY("Latitude", &nap::SunsetCalculatorComponent::mLatitude, nap::rtti::EPropertyMetaData::Default, "Latitude of the location we want to know the sunrise and sundown of")
	RTTI_PROPERTY("Longitude", &nap::SunsetCalculatorComponent::mLongitude, nap::rtti::EPropertyMetaData::Default, "Longitude of the location we want to know the sunrise and sundown of")
	RTTI_PROPERTY("TimeZone", &nap::SunsetCalculatorComponent::mTimezone, nap::rtti::EPropertyMetaData::Default, "Timezone at Longitude")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SunsetCalculatorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{   
	SunsetCalculatorComponentInstance::SunsetCalculatorComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource),
		mModel(std::make_unique<SunSet>())
	{ }


	// this is needed for the PIMPL (Pointer To Implementation) to work with the unique_ptr to Sunset in the header
	SunsetCalculatorComponentInstance::~SunsetCalculatorComponentInstance() { }


	bool SunsetCalculatorComponentInstance::init(utility::ErrorState& errorState)
    {
		// Set position
		auto* resource = getComponent<nap::SunsetCalculatorComponent>();
		mTimezone = resource->mTimezone;
		mLongitude = resource->mLongitude;
		mLatitude = resource->mLatitude;

		// Initialize model
		mModel->setPosition(mLatitude, mLongitude, mTimezone);

		// Compute
		update(0.0);
        return true;
    }


	void SunsetCalculatorComponentInstance::update(double delta)
	{
		// Get current date-time
		auto date_time = getCurrentDateTime();

		// If day changed, update sunset / sunrise information
		if (mDay != date_time.getDay())
		{
			// Get null (midnight) for current date/time
			auto null_time = createTimestamp(date_time.getYear(), static_cast<int>(date_time.getMonth()), date_time.getDayInTheMonth(), 0, 0, 0);

			// Update current date
			mModel->setCurrentDate(date_time.getYear(), static_cast<int>(date_time.getMonth()), date_time.getDayInTheMonth());

			// Compute sunrise
			static constexpr double mms = 60.0 * 1000.0;
			mSunRiseMinute = mModel->calcSunrise();
			mSunRiseStamp = null_time + Milliseconds(static_cast<int64>(mSunRiseMinute * mms));
			mSunRise = DateTime(mSunRiseStamp, DateTime::ConversionMode::Local);

			// Compute sunset
			mSunSetMinute = mModel->calcSunset();
			mSunSetStamp = null_time + Milliseconds(static_cast<int64>(mSunSetMinute * mms));
			mSunset = DateTime(mSunSetStamp, DateTime::ConversionMode::Local);

			// Store computed day
			mDay = date_time.getDay();
		}

		// Check if we need to notify listeners
		const auto& current = date_time.getTimeStamp();
		auto current_state = current > mSunRiseStamp && current < mSunSetStamp ?
			EState::Up : EState::Down;

		// Notify listeners
		if (current_state != mState)
		{
			mState = current_state;
			mSunStateChanged(mState);
		}
	}
}
