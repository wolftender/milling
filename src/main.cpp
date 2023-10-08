#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "app.hpp"

int main(int argc, char** argv) {
	// initialize glfw
	if (!glfwInit()) {
		std::cerr << "fatal: failed to initialize glfw!" << std::endl;
		return 1;
	}

	std::unique_ptr<mini::application> app = nullptr;

	try {
		app = std::make_unique<mini::application>();
	} catch (const std::exception& e) {
		std::cerr << "failed to init app: " << e.what() << std::endl;
		return 1;
	}

	app->message_loop();

	// cleanup glfw
	glfwTerminate();
	return 0;
}