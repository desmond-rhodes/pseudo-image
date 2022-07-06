#include <iostream>
#include <vector>
#include <string>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>

int pseudo_image(std::vector<std::string> const&);

int main(int argc, char* argv[]) {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	try {
		std::vector<std::string> args(argv, argv+argc);
		return pseudo_image(args);
	}
	catch (std::exception const& e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
	catch (...) {
		std::cerr << "Uncaught exception of unknown type was encountered.\n";
		return -1;
	}
}

std::string file_content(std::string const&);
GLuint shader_compile(GLenum, std::string const&);
GLuint shader_link(std::vector<GLuint> const&);

int pseudo_image(std::vector<std::string> const& args) {
	struct {
		bool renew(size_t w, size_t h) {
			if (data == nullptr) {
				delete[] data;
				this->w = 8;
				this->h = 8;
				data = new GLuint[this->w*this->h] {
					0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
					0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000,
					0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
					0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000,
					0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
					0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000,
					0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff,
					0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000, 0xff0000ff, 0xffff0000
				};
				return true;
			}
			return false;
		}
		GLuint* data {nullptr};
		size_t w;
		size_t h;
	}
	image;

	struct glfwHandle {
		glfwHandle() {
			if (!glfwInit())
				throw std::runtime_error("glfwHandle");
		}
		~glfwHandle() {
			glfwTerminate();
		}
	}
	use_glfw;

	struct winfo_t {
		int w {960};
		int h {720};
	}
	winfo;

	GLFWwindow* window;
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		window = glfwCreateWindow(winfo.w, winfo.h, "Hello World", nullptr, nullptr);
		if (!window)
			return -1;
		glfwSetWindowUserPointer(window, static_cast<void*>(&winfo));
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
			auto winfo {static_cast<winfo_t*>(glfwGetWindowUserPointer(window))};
			winfo->w = w;
			winfo->h = h;
			glViewport(0, 0, w, h);
		});
		glfwMakeContextCurrent(window);
		if (gl3wInit())
			return -1;
		std::cout
			<< "OpenGL " << glGetString(GL_VERSION)
			<< ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION)
			<< '\n' << std::flush;
	}

	GLuint shader;
	{
		auto const vert_src {file_content("shader.vert")};
		auto const frag_src {file_content("shader.frag")};
		auto const vert_obj {shader_compile(GL_VERTEX_SHADER  , vert_src)};
		auto const frag_obj {shader_compile(GL_FRAGMENT_SHADER, frag_src)};
		shader = shader_link({vert_obj, frag_obj});
		glDeleteShader(vert_obj);
		glDeleteShader(frag_obj);
	}
	glUseProgram(shader);

	auto const uniform_tex {glGetUniformLocation(shader, "tex")};

	GLuint sam;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	{
		glCreateSamplers(1, &sam);
		glSamplerParameteri(sam, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sam, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindSampler(uniform_tex, sam);

		GLuint const bindingindex {0};

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		auto const attrib_position {glGetAttribLocation(shader, "vPosition")};
		glEnableVertexAttribArray(attrib_position);
		glVertexAttribFormat(attrib_position, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(attrib_position, bindingindex);

		auto const attrib_texcoord {glGetAttribLocation(shader, "vTexCoord")};
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribFormat(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat));
		glVertexAttribBinding(attrib_texcoord, bindingindex);

		GLfloat vertex[] {-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f};
		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(vertex), vertex, 0);
		glBindVertexBuffer(bindingindex, vbo, 0, 4*sizeof(GLfloat));

		GLuint index[] {0, 1, 2, 0, 3, 2};
		glCreateBuffers(1, &ebo);
		glNamedBufferStorage(ebo, sizeof(index), index, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	struct {
		void resize(GLsizei w, GLsizei h) {
			this->w = w;
			this->h = h;
			glDeleteTextures(1, &name);
			glCreateTextures(GL_TEXTURE_2D, 1, &name);
			glTextureStorage2D(name, 1, GL_RGBA8, w, h);
		}
		void bind(GLuint uniform) {
			glBindTextureUnit(uniform, name);
		}
		void data(size_t w, size_t h, GLuint const* image) {
			glTextureSubImage2D(name, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);
		}
		GLuint name {0};
		GLsizei w;
		GLsizei h;
	}
	texture;

	std::chrono::microseconds const r_limit {16666}; /* 60Hz */
	auto r_last {std::chrono::steady_clock::now()};

	while (!glfwWindowShouldClose(window)) {
		if (image.renew(winfo.w, winfo.h)) {
			if (image.w != texture.w || image.h != texture.h) {
				texture.resize(image.w, image.h);
				texture.bind(uniform_tex);
			}
			texture.data(image.w, image.h, image.data);
		}
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_for(r_limit - (std::chrono::steady_clock::now() - r_last));
		r_last = std::chrono::steady_clock::now();
	}

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
	auto const obj {glCreateShader(type)};
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
	auto const pro {glCreateProgram()};
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
