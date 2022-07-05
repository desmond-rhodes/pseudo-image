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

	GLuint point_ib {0};
	GLuint color_ib {1};
	GLuint texco_ib {2};

	GLuint format_ao;
	{
		glGenVertexArrays(1, &format_ao);
		glBindVertexArray(format_ao);

		GLint const shader_pos {glGetAttribLocation(shader, "vPosition")};
		glEnableVertexAttribArray(shader_pos);
		glVertexAttribFormat(shader_pos, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(shader_pos, point_ib);

		GLint const shader_col {glGetAttribLocation(shader, "vColor")};
		glEnableVertexAttribArray(shader_col);
		glVertexAttribFormat(shader_col, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(shader_col, color_ib);

		GLint const shader_tco {glGetAttribLocation(shader, "vTexCoord")};
		glEnableVertexAttribArray(shader_tco);
		glVertexAttribFormat(shader_tco, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(shader_tco, texco_ib);

		glBindVertexArray(0);
	}

	GLuint textu_bo;
	{
		std::vector<GLubyte> const textu {
			0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
			0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
			0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
			0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
			0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
			0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
			0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
			0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff
		};
		glCreateBuffers(1, &textu_bo);
		glNamedBufferStorage(textu_bo, textu.size() * sizeof(textu[0]), textu.data(), 0);
	}

	GLuint texture;
	{
		glCreateTextures(GL_TEXTURE_2D, /* 0 */ 1, &texture); /* don't work with level 0 */
		glTextureStorage2D(texture, 1, GL_R8, 8, 8);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, textu_bo);
		glTextureSubImage2D(texture, 0, 0, 0, 8, 8, GL_RED, GL_UNSIGNED_BYTE, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	GLuint sampler;
	{
		glCreateSamplers(1, &sampler);
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLsizei point_stride;
	GLuint point_bo;
	{
		std::vector<GLfloat> const point {
			-0.5f,  0.5f,
			 0.5f,  0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f
		};
		glCreateBuffers(1, &point_bo);
		glNamedBufferStorage(point_bo, point.size() * sizeof(point[0]), point.data(), 0);
		point_stride = 2 * sizeof(point[0]);
	}

	GLsizei color_stride;
	GLuint color_bo;
	{
		std::vector<GLfloat> const color {
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 1.0f
		};
		glCreateBuffers(1, &color_bo);
		glNamedBufferStorage(color_bo, color.size() * sizeof(color[0]), color.data(), 0);
		color_stride = 3 * sizeof(color[0]);
	}

	GLsizei texco_stride;
	GLuint texco_bo;
	{
		std::vector<GLfloat> const texco {
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f
		};
		glCreateBuffers(1, &texco_bo);
		glNamedBufferStorage(texco_bo, texco.size() * sizeof(texco[0]), texco.data(), 0);
		texco_stride = 2 * sizeof(texco[0]);
	}

	GLsizei index_count;
	GLuint index_bo;
	{
		std::vector<GLuint> const index {
			0, 1, 2,
			0, 3, 2
		};
		glCreateBuffers(1, &index_bo);
		glNamedBufferStorage(index_bo, index.size() * sizeof(index[0]), index.data(), 0);
		index_count = index.size();
	}

	std::cout << std::flush;

	std::vector<GLfloat> const fill {0.0f, 0.0f, 0.0f, 0.0f};

	glBindTextureUnit(uniform_tex, texture); /* crash when texture() don't exist */
	glBindSampler(uniform_tex, sampler);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, std::vector({GL_RED, GL_RED, GL_RED, GL_ONE}).data());

	glBindVertexArray(format_ao);
	glBindVertexBuffer(point_ib, point_bo, 0, point_stride);
	glBindVertexBuffer(color_ib, color_bo, 0, color_stride);
	glBindVertexBuffer(texco_ib, texco_bo, 0, texco_stride);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_bo);

	std::chrono::microseconds const r_limit {16666}; /* 60Hz */
	auto r_last {std::chrono::steady_clock::now()};

	while (!glfwWindowShouldClose(window)) {
		glClearBufferfv(GL_COLOR, 0, fill.data());
		glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

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
