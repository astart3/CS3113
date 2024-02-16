#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>

#include <iostream>

SDL_Window* g_displayWindow;
//datatype for where the game is displayed
bool g_gameIsRunning = true;
//boolean checking whether game is running
SDL_Event event;

// Some constants necessary to get started
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

// The background colours may change in the course of our games, so they can also be variables
const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	g_displayWindow = SDL_CreateWindow("Hello, World!",
										SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(g_displayWindow); //creates OPENGL context
	SDL_GL_MakeCurrent(g_displayWindow, context); //sets OPENGL context to current window 

	#ifdef _WINDOWS
		glewInit();
	#endif

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY); //sets background color

	while (g_gameIsRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				g_gameIsRunning = false;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);  // Quite simply: clear the space in memory holding our colours
		SDL_GL_SwapWindow(g_displayWindow); // Update a window with whatever OpenGL is rendering
	}

	SDL_Quit();
	return 0;
}