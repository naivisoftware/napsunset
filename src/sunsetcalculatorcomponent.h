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
		* Returns a normalised proportion of the time passed :
		* [1](now - sun rose up) / (sun setting down - sun rose up)
		* [2](now - previous sun setting down) / (sun rose up - previous sun setting down)
		* [1] if the time passed is between sun rose up and sun setting down (day time)
		* [2] if the time passed is between previous sun setting down and sun rose up (night time)
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

		float currentPropSun;
		bool sunIsCurrentlyUp;

		// deltaUntilNextCalculation the new delta until the calculation of the sunset needs to be done
		long float deltaUntilNextCalculation;

		long float accumulatedTime;



	};
}
