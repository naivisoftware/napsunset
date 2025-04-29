#include "sunsetservice.h"

#include <component.h>
#include "nap/datetime.h"



namespace nap
{
	class SunsetCalculatorComponentInstance;
	class SunsetService;

    class NAPAPI SunsetCalculatorComponent: public Component
    {
        RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SunsetCalculatorComponent, SunsetCalculatorComponentInstance)
        public:
            
            double latitude = 0, longitude = 0;
			int timezone=2;
    };


	class NAPAPI SunsetCalculatorComponentInstance : public ComponentInstance {

		friend class SunsetService;
		RTTI_ENABLE(ComponentInstance)
	public:

		/**
		 * @param entity the entity this component belongs to.
		 * @param resource the resource this instance was created from.
		 */
		SunsetCalculatorComponentInstance(EntityInstance& entity, Component& resource);
		~SunsetCalculatorComponentInstance() = default;

		virtual bool init(utility::ErrorState& erroState) override;

		void update(double deltaTime) override;

		/*
		* Returns a normalised proportion:
		* positive if the sun is up
		* negative if the sun is down
		* of the time left until sundown
		*/
		float getProp();

		bool istheSunUp() {
			return sunIsCurrentlyUp;
		}

	private:
		void calculateCurrentSunsetState();
		double calculatePreviousSunset(int year, int month, int day);

		// in minutes
		double currentSunrise=-1, currentSunset=-1;
		double previousSunset = -1;
		int currentSunsetHours, currentSunsetMinutes;
		int offsetTimeSunsettingDown;

		float currentPropSunUp;
		bool sunIsCurrentlyUp;

		// deltaUntilNextCalculation the new delta until the calculation of the sunset needs to be done
		long float deltaUntilNextCalculation;

		long float accumulatedTime;



	};
}
