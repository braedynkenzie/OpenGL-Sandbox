#include "Globals.h"
#include <GLFW\glfw3.h>

// Initialize all global variables (declared in Globals.h)
unsigned int SCREEN_WIDTH = 960;
unsigned int SCREEN_HEIGHT = 540;

float deltaTime = 0.0f; 
float lastFrameTime = 0.0f; 
float startTime = glfwGetTime();

float lastCursorX = SCREEN_WIDTH / 2.0f;
float lastCursorY = SCREEN_HEIGHT / 2.0f;

float yaw = 0.0f;
float pitch = 0.0f;
bool firstMouseCapture = true; 