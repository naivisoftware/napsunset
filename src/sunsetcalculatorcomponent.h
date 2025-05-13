/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <nap/datetime.h>
#include <nap/timer.h>
#include <nap/signalslot.h>
#include <mathutils.h>

// Forward declare thirdparty-sunset
class SunSet;

namespace nap
{
	class SunsetCalculatorComponentInstance;

	/**
	 * Calculates sunset and sunrise for a given lat and longitude
	 */
    class NAPAPI SunsetCalculatorComponent: public Component
    {
        RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SunsetCalculatorComponent, SunsetCalculatorComponentInstance)

		public:

			// Geographic coordinates and timezone for calculating sun position
			double mLatitude = 0;					///<  Property: 'latitude' set to use 0	(Greenwich)	->(nul island)
			double mLongitude = 0;					///<  Property: 'longitude' set to use 0(equator)	->(nul island)
			int mTimezone = 2;						///<  Property: 'timezone' set to use 2 (Europe's timezone)
    };


	/**
	 * Calculates sunset and sunrise for a given lat and longitude
	 */
	class NAPAPI SunsetCalculatorComponentInstance : public ComponentInstance
	{

		RTTI_ENABLE(ComponentInstance)
	public:

		enum class EState : int8
		{
			Unknown		= -1,	//< Current sunset state is unknown
			Down		= 0,	//< Current sunset state is down 
			Up			= 1		//< Current sunset state is up
		};

		/**
		 * @param entity the entity this component belongs to.
		 * @param resource the resource this instance was created from.
		 */
		SunsetCalculatorComponentInstance(EntityInstance& entity, Component& resource);

		// Destructor
		~SunsetCalculatorComponentInstance() override;

		/**
		* Initialises the sunset
		* sunset gets its location(latitude/longitude/timezone) only here, and nowhere else.
		*/
		bool init(utility::ErrorState& erroState) override;

		/**
		 * Waits until the time has come to recalculate.
		 */
		void update(double deltaTime) override;

		/**
		 * @brief Checks whether the sun is currently above the horizon.
		 * @return bool `true` if the sun is up (daytime), `false` if down (nighttime).
		 */
		bool isUp() const								{ return mState == EState::Up; }

		/**
		 * @return current sun state (up or down)
		 */
		EState getState() const							{ return mState; }

		/**
		 * @return sunset time for current day
		 */
		const DateTime& getSunSet() const				{ return mSunset; }

		/**
		 * @return sunrise time for current day
		 */
		const DateTime& getSunRise() const				{ return mSunRise; }

		/**
		 * Listen to this signal to get notified on sunset / sunrise
		 */
		Signal<EState> mSunStateChanged;

	private:
		EState mState = EState::Unknown;				///< Current daylight status (true = sun is above horizon)
		std::unique_ptr<SunSet> mModel;					///< unique ptr to the sunset class
		EDay mDay = EDay::Unknown;						///< current day

		double mSunRiseMinute = 0;						///< Minutes past midnight for sunrise
		SystemTimeStamp mSunRiseStamp;					///< Sunrise timestamp
		DateTime mSunRise;								///< Sunrise date-time

		double mSunSetMinute =  0;						///< Minutes past midnight for sunset
		SystemTimeStamp mSunSetStamp;					///< Sunset timestamp
		DateTime mSunset;								///< Sunset date-time

		int mTimezone = 0;								///< Location timezone
		double mLatitude = 0;							///< Location latitude
		double mLongitude = 0;							///< Location longitude
	};
}
