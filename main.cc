#include <iostream>
#include <vector>
#include <string>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>

std::string file_content(std::string const&);
GLuint shader_compile(GLenum, std::string const&);
GLuint shader_link(std::vector<GLuint> const&);

int main(int argc, char* argv[]) {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::vector<std::string> args(argv, argv+argc);

	for (auto const& i : args)
		std::cout << i << ' ';
	std::cout << '\n';

	if (!glfwInit())
		return -1;

	GLFWwindow* window;
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(960, 720, "Hello World", nullptr, nullptr);

		if (!window) {
			glfwTerminate();
			return -1;
		}

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			glViewport(0, 0, width, height);
		});
	}
	glfwMakeContextCurrent(window);
	{
		if (gl3wInit())
			return -1;

		if (!gl3wIsSupported(4, 5))
			return -1;

		std::cout
			<< "OpenGL " << glGetString(GL_VERSION)
			<< ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION)
			<< '\n';
	}

	GLuint shader;
	try {
		std::string const vert_src {file_content("shader.vert")};
		std::string const frag_src {file_content("shader.frag")};
		GLuint const vert_obj {shader_compile(GL_VERTEX_SHADER  , vert_src)};
		GLuint const frag_obj {shader_compile(GL_FRAGMENT_SHADER, frag_src)};
		shader = shader_link({vert_obj, frag_obj});
		glDeleteShader(vert_obj);
		glDeleteShader(frag_obj);
	}
	catch (std::exception const& e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
	glUseProgram(shader);

	GLint const uniform_tex {glGetUniformLocation(shader, "tex")};

	GLuint vertex_ib {0};

	GLuint format_ao;
	{
		glGenVertexArrays(1, &format_ao);
		glBindVertexArray(format_ao);

		GLint const shader_pos {glGetAttribLocation(shader, "vPosition")};
		glEnableVertexAttribArray(shader_pos);
		glVertexAttribFormat(shader_pos, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(shader_pos, vertex_ib);

		GLint const shader_tco {glGetAttribLocation(shader, "vTexCoord")};
		glEnableVertexAttribArray(shader_tco);
		glVertexAttribFormat(shader_tco, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat));
		glVertexAttribBinding(shader_tco, vertex_ib);

		glBindVertexArray(0);
	}

	GLuint texture;
	{
		std::vector<GLuint> const textu {
			0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
			0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000,
			0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
			0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000,
			0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
			0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000,
			0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
			0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000
		};
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);
		glTextureStorage2D(texture, 1, GL_RGBA8, 8, 8);
		glTextureSubImage2D(texture, 0, 0, 0, 8, 8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, textu.data());
	}

	GLuint sampler;
	{
		glCreateSamplers(1, &sampler);
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLsizei vertex_stride;
	GLuint vertex_bo;
	{
		std::vector<GLfloat> const vertex {
			-1.0f,  1.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 0.0f,
			 1.0f, -1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 1.0f
		};
		glCreateBuffers(1, &vertex_bo);
		glNamedBufferStorage(vertex_bo, vertex.size() * sizeof(vertex[0]), vertex.data(), 0);
		vertex_stride = 4 * sizeof(vertex[0]);
	}

	GLenum index_m;
	GLenum index_t;
	GLsizei index_count;
	GLuint index_bo;
	{
		std::vector<GLuint> const index {
			0, 1, 2,
			0, 3, 2
		};
		glCreateBuffers(1, &index_bo);
		glNamedBufferStorage(index_bo, index.size() * sizeof(index[0]), index.data(), 0);
		index_m = GL_TRIANGLES;
		index_t = GL_UNSIGNED_INT;
		index_count = index.size();
	}

	std::cout << std::flush;

	std::vector<GLfloat> const fill {0.0f, 0.0f, 0.0f, 0.0f};

	glBindTextureUnit(uniform_tex, texture);
	glBindSampler(uniform_tex, sampler);

	glBindVertexArray(format_ao);
	glBindVertexBuffer(vertex_ib, vertex_bo, 0, vertex_stride);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_bo);

	std::chrono::microseconds const r_limit {16666}; /* 60Hz */
	auto r_last {std::chrono::steady_clock::now()};

	while (!glfwWindowShouldClose(window)) {
		glClearBufferfv(GL_COLOR, 0, fill.data());
		glDrawElements(index_m, index_count, index_t, nullptr);

		glfwSwapBuffers(window);

		glfwPollEvents();

		std::this_thread::sleep_for(r_limit - (std::chrono::steady_clock::now() - r_last));
		r_last = std::chrono::steady_clock::now();
	}

	glfwTerminate();

	std::cout << "Hello, world!\n";
	return 0;
}

std::string file_content(std::string const& name) {
	std::ifstream file;
	file.open(name, std::ifstream::in);
	file.seekg(0, std::ios_base::end);
	std::string content(file.tellg(), '\0');
	file.seekg(0, std::ios_base::beg);
	file.read(content.data(), content.size());
	file.close();
	return content;
}

GLuint shader_compile(GLenum type, std::string const& src) {
	GLuint obj {glCreateShader(type)};
	if (!obj)
		throw std::runtime_error("shader_compile");
	GLchar const* v_src[] {src.data()};
	glShaderSource(obj, 1, v_src, nullptr);
	glCompileShader(obj);
	GLint status;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
		std::string log(length, '\0');
		glGetShaderInfoLog(obj, length, nullptr, log.data());
		std::cerr << log << '\n';
		glDeleteShader(obj);
		throw std::runtime_error("shader_compile");
	}
	return obj;
}

GLuint shader_link(std::vector<GLuint> const& obj) {
	GLuint pro {glCreateProgram()};
	if (!pro)
		throw std::runtime_error("shader_link");
	for (auto const& i : obj)
		glAttachShader(pro, i);
	glLinkProgram(pro);
	GLint status;
	glGetProgramiv(pro, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetProgramiv(pro, GL_INFO_LOG_LENGTH, &length);
		std::string log(length, '\0');
		glGetProgramInfoLog(pro, length, nullptr, log.data());
		std::cerr << log << '\n';
		glDeleteProgram(pro);
		throw std::runtime_error("shader_link");
	}
	for (auto const& i : obj)
		glDetachShader(pro, i);
	return pro;
}
