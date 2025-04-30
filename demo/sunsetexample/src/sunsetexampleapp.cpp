#include "sunsetexampleapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <imgui/imgui.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::sunsetexampleApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool sunsetexampleApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();


		// Fetch the resource manager
		mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the camera entity
		mCameraEntity = mScene->findEntity("CameraEntity");
		if (!error.check(mCameraEntity != nullptr, "unable to find entity with name: %s", "CameraEntity"))
			return false;

		// Get the Gnomon entity
		mGnomonEntity = mScene->findEntity("GnomonEntity");
		if (!error.check(mGnomonEntity != nullptr, "unable to find entity with name: %s", "GnomonEntity"))
			return false;


		mSunsetEntity = mScene->findEntity("SunsetEntity");
		if (!error.check(mGnomonEntity != nullptr, "unable to find entity with name: %s", "SunsetEntity"))
			return false;



		mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");

		// All done!
		return true;
	}
	
	
	// Update app
	void sunsetexampleApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });


		nap::SunsetCalculatorComponentInstance& sunsetCalculatorComponentInstance = mSunsetEntity->getComponent<SunsetCalculatorComponentInstance>();
		sunsetCalculatorComponentInstance.calculateProp();

		// Select GUI window
		mGuiService->selectWindow(mRenderWindow);


		const auto& theme = mGuiService->getPalette();
		// Draw some gui elements
		ImGui::Begin("Controls");

		// Show all parameters
		mParameterGUI->show(false);

		bool sunIsUp = sunsetCalculatorComponentInstance.istheSunUp();

		// Display some extra info
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(theme.mHighlightColor2, "the sun is %s", sunIsUp ? "Up" : "Down");
		ImGui::TextColored(theme.mHighlightColor3, "%s course purcentage in the %s = %f", sunIsUp ? "Sun" : "Moon", sunIsUp ? "sky" : "night", sunsetCalculatorComponentInstance.getProp());

		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();



	}
	
	
	// Render app
	void sunsetexampleApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get Perspective camera to render with
			auto& perp_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();

			// Add Gnomon
			std::vector<nap::RenderableComponentInstance*> components_to_render
			{
				&mGnomonEntity->getComponent<RenderGnomonComponentInstance>()
			};

			// Render Gnomon
			mRenderService->renderObjects(*mRenderWindow, perp_cam, components_to_render);

			// Render GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
	}
	

	void sunsetexampleApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void sunsetexampleApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// If we pressed escape, quit the loop
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// f is pressed, toggle full-screen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}
		// Add event, so it can be forwarded on update
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int sunsetexampleApp::shutdown()
	{
		return 0;
	}

}
