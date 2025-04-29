#pragma once

#include <nap/service.h>
#include <nap/signalslot.h>

namespace nap
{

	class SunsetCalculatorComponentInstance;

    class NAPAPI SunsetService : public Service
    {
		friend class SunsetCalculatorComponentInstance;
        RTTI_ENABLE(Service)
    public:
		SunsetService(ServiceConfiguration* configuration) :Service(configuration) {}
        
        virtual bool init(nap::utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

	private:
		SunsetCalculatorComponentInstance* mSunsetCalculator;


    };
}
