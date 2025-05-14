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
	 * Calculates local sunset and sunrise for a given lat and longitude.
	 * Listen to the 'mSunStateChanged, 'mSunUp' or 'mSunDown' signals to receive sunrise and sunset events.
	 */
    class NAPAPI SunsetCalculatorComponent: public Component
    {
        RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SunsetCalculatorComponent, SunsetCalculatorComponentInstance)

		public:
			double mLatitude = 0;					///< Property: 'Latitude' set to use 0	(Greenwich)	->(nul island)
			double mLongitude = 0;					///< Property: 'Longitude' set to use 0(equator)	->(nul island)
			int mTimezone = 1;						///< Property: 'Timezone' timezone, excluding daylight savings
    		double mSunriseOffset = 0.0;			///< Property: 'SunriseOffset' sunrise offset in minutes
    		double mSunsetOffset = 0.0;				///< Property: 'SunsetOffset' sunset offset in minutes
    };


	/**
	 * Calculates **local** sunset and sunrise for a given lat and longitude, including offsets.
	 * Listen to 'mSunStateChanged, 'mSunUp' or 'mSunDown' signals to receive sunrise and sunset events.
	 *
	 * Note that this component uses the systems local time to check if the sun is up or down,
	 * not the time deducted from the given lon and latitude -> which it cannot do.
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
		 * @return current sun state (up or down)
		 */
		EState getState() const							{ return mState; }

		/**
		 * @return local sunset time
		 */
		const DateTime& getSunSet() const				{ return mSunset; }

		/**
		 * @return local sunrise time
		 */
		const DateTime& getSunRise() const				{ return mSunRise; }

		/**
		 * @return latitude
		 */
		double getLatitude() const						{ return mLatitude; }

		/**
		 * @return longitude
		 */
		double getLongitude() const						{ return mLongitude; }

		/**
		 * @return bool `true` if the sun is up (daytime), `false` if down (nighttime).
		 */
		bool isUp() const								{ return mState == EState::Up; }

		/**
		 * Listen to this signal to get notified on sunset / sunrise
		 */
		Signal<EState> mSunStateChanged;

		/**
		 * Listen to this signal to get notified of sunrise
		 */
		Signal<> mSunUp;

		/**
		 * Listen to this signal to get notifief of sunset
		 */
		Signal<> mSunDown;

	private:
		EState mState = EState::Unknown;				///< Current daylight status (true = sun is above horizon)
		std::unique_ptr<SunSet> mModel;					///< unique ptr to the sunset class
		EDay mDay = EDay::Unknown;						///< current day

		SystemTimeStamp mSunRiseStamp;					///< Sunrise timestamp
		DateTime mSunRise;								///< Sunrise date-time=
		double mSunriseOffset = 0.0;					///< Sunrise offset in minutes

		SystemTimeStamp mSunSetStamp;					///< Sunset timestamp
		DateTime mSunset;								///< Sunset date-time
		double mSunsetOffset = 0.0;						///< Sunset offset in minutes

		int mTimezone = 0;								///< Location timezone
		double mLatitude = 0;							///< Location latitude
		double mLongitude = 0;							///< Location longitude
	};
}
