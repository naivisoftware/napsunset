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
			int mMinutesOffsetSunDown = 0;			///<  Property: 'minutesOffsetSunDown' offset from the moment the sun start setting down to the moment we consider the sun to be completely down - set to 0
    };


	/**
	 * Calculates sunset and sunrise for a given lat and longitude
	 */
	class NAPAPI SunsetCalculatorComponentInstance : public ComponentInstance
	{

		RTTI_ENABLE(ComponentInstance)
	public:

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
		 * @brief Calculates sun position proportions and daylight status.
		 *
		 * This method computes:
		 * 1. The proportion (mCurrentPropSun) of the sun's current position
		 *    relative to its total daytime course (when the sun is up).
		 * 2. The proportion (mCurrentPropSun) of the sun's current position
		 *    relative to its total nighttime course (when the sun is down).
		 * 3. Updates mSunIsCurrentlyUp to indicate whether the sun is currently
		 *    above the horizon (true) or below it (false).
		 *
		 * @note The same variable (mCurrentPropSun) stores different proportions
		 *       depending on whether it's day or night.
		 */
		void calculateProp();

		/**
		 * @brief Gets the current sun position proportion.
		 *
		 * This method returns the proportion of the sun's progress through its daily cycle:
		 * - During daytime: Ratio of current sun position to total daytime duration
		 * - During nighttime: Ratio of current sun position to total nighttime duration
		 *
		 * @return a float value in range [0.0, 1.0] representing:
		 *               - Sun's progress through daytime when sun is up
		 *               - Sun's progress through nighttime when sun is down
		 *
		 */
		float getProp() {return mCurrentPropSun;}

		/**
		 * @brief Checks whether the sun is currently above the horizon.
		 * @return bool `true` if the sun is up (daytime), `false` if down (nighttime).
		 */
		bool isUp() {return mSunIsCurrentlyUp;}

		/**
		* @return an int value (in minutes) corresponding to the time until the next sunrise or sunset
		*/
		int getTimeUntilNextSunCourseChange() {return mTimeUntilNextSunchange;}

		Signal<bool> mSunIsUp;

	private:

		/**
		* Calculates the time of the sunrise and sunset for today at the given location.
		*/
		void calculateCurrentSunsetState();

		/**
		* Calculates the time of the sunrise and sunset for yesterday at the given location.
		*/
		double calculatePreviousSunset(DateTime date);

		/**
		* Calculates the time of the sunrise and sunset for tomorrow at the given location.
		*/
		double calculateNextSunrise(DateTime date);


		double mCurrentSunrise = -1;					///< Today's sunrise time in minutes from midnight  
		double mCurrentSunset = -1;						///< Today's sunset time in minutes from midnight  
		double mPreviousSunset = -1;					///< Yesterday's sunset time in minutes from midnight
		double mNextSunrise = -1;						///< Tomorrow's sunrise time in minutes from midnight  

		int mCurrentSunsetHours;						///< Today's sunset hour component (0-23)  
		int mCurrentSunsetMinutes;						///< Today's sunset minute component (0-59)  
		int mMinutesOffsetTimeSunsettingDown;			///< Additional offset (in minutes) after sunset until sun is completely down  : 1h extra to the time of the starting of the sun setting down --> the time the night is dark

		float mCurrentPropSun = -1;						///< Sun's progress proportion (0.0-1.0): daytime progress when sun is up, nighttime progress when sun is down
		int mTimeUntilNextSunchange = -1;				///< Current time in minutes until the sun is goin down, or setting up				
		bool mSunIsCurrentlyUp;							///< Current daylight status (true = sun is above horizon)  
		long mDeltaUntilNextCalculation = 0;			///< Time remaining (s) until next sunset/sunrise calculation  

		nap::SystemTimer mDeltaCalculationTimer;        ///< Timer tracking interval until next required calculation (at next sunset. Settings this to 10s so to not retrigger the calculation of the sunset until mDeltaUntilNextCalculation is properly set inside calculateCurrentSunsetState
		std::unique_ptr<SunSet> mSunset = nullptr;		///< unique ptr to the sunset class
		nap::SystemTimeStamp mCalcStamp;
	};
}
