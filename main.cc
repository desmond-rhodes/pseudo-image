#include <iostream>
#include <vector>
#include <string>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

int main(int argc, char* argv[]) {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::vector<std::string> args(argv, argv+argc);

	for (auto const& i : args)
		std::cout << i << ' ';
	std::cout << '\n';

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	GLFWwindow* window = glfwCreateWindow(960, 720, "Hello World", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (gl3wInit())
		return -1;

	if (!gl3wIsSupported(4, 5))
		return -1;

	std::cout
		<< "OpenGL " << glGetString(GL_VERSION)
		<< ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION)
		<< '\n';

	std::cout << std::flush;

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();

	std::cout << "Hello, world!\n";
	return 0;
}
