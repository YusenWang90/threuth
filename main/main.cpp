#include "renderer.hpp"

OpenGLRenderer renderer;

float frameTime = 0.0f;

int main() try
{
	renderer.startUp();

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

	// TODO: 

	renderer.loadResources();

	OGL_CHECKPOINT_ALWAYS();

	Timer timer;

	// Main loop
	while(!glfwWindowShouldClose(renderer.getWindow()))
	{
		timer.Reset();

		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize(renderer.getWindow(), &nwidth, &nheight);

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( renderer.getWindow(), &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, renderer.getWindowWidth(), renderer.getWindowHeight());
		}

		// Update state
		renderer.updateTreeRotation();

		renderer.updateConstantMovement();

		//TODO: update state
		renderer.updateInput(frameTime);

		renderer.updateUniforms();

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
		renderer.drawScene();

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers(renderer.getWindow());

		frameTime = timer.Elapsed();
	}

	// Cleanup.
	//TODO: additional cleanup
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}

