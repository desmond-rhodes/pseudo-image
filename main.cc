struct winfo_t {
	char const* title {"Pseudo Image"};
	int w {1280};
	int h {960};
}
winfo;

#include <GL/gl3w.h>
#include <new>

class {
private:
	GLuint fragment(int x, int y) {
		GLuint color {0x000000};

		/* x-axis */
		if (y == 0)
			color = 0x333333;
		/* x-axis 10 unit */
		if (x != 0 && x %  10 == 0 && std::abs(y)  <= 8)
			color = 0x333333;
		/* x-axis 100 unit */
		if (x != 0 && x % 100 == 0 && std::abs(y) <= 10)
			color = 0x777777;

		/* y-axis */
		if (x == 0)
			color = 0x333333;
		/* y-axis 10 unit */
		if (y != 0 && y %  10 == 0 && std::abs(x) <=  8)
			color = 0x333333;
		/* y-axis 100 unit */
		if (y != 0 && y % 100 == 0 && std::abs(x) <= 10)
			color = 0x777777;

		/* line */
		if (std::abs(y+40 - 0.5*x) <= 2)
			color = 0xffffff;
		/* circle */
		if (std::abs(x*x+y*y - 325*325) <= 1300)
			color = 0xffffff;
		/* parabola */
		if (std::abs(x-1 + 0.0025*y*y) <= 3)
			color = 0xffffff;
		/* polynomial */
		if (std::abs(y - 0.00000001*(x+700)*(x+500)*(x-200)*(x-600)) <= 10)
			color = 0xffffff;

		return color | 0xff000000;
	}

public:
	GLuint* data {nullptr};
	size_t w;
	size_t h;

	bool renew(size_t w, size_t h) {
		if (data == nullptr || w != this->w || h != this->h) {
			auto allocate {new (std::nothrow) GLuint[w*h]};
			if (!allocate)
				return false;
			delete[] data;

			data = allocate;
			this->w = w;
			this->h = h;

			auto y {static_cast<int>(h)/-2};
			size_t j {0};
			while (j < h) {
				auto x {static_cast<int>(w)/-2};
				size_t i {0};
				while (i < w) {
					data[j*w+i] = fragment(x, y);
					x += 1;
					i += 1;
				}
				y += 1;
				j += 1;
			}

			return true;
		}
		return false;
	}
}
image;

#include <optional>
#include <ostream>
#include <fstream>

char* file_content(char const* const filename, std::ostream& os) {
	std::ifstream file;
	file.open(filename, std::ifstream::in);
	if (file.fail()) {
		os << "Unable to open file " << filename << "\n";
		return nullptr;
	}
	file.seekg(0, std::ios_base::end);
	auto const size {static_cast<size_t>(file.tellg())};
	auto content {new (std::nothrow) char[size+1]()};
	if (!content) {
		os << "Unable to allocate buffer of size " << size << " bytes to load file " << filename << ".\n";
		return nullptr;
	}
	file.seekg(0, std::ios_base::beg);
	file.read(content, size);
	if (file.fail()) {
		delete[] content;
		os << "Unable to read file " << filename << "\n";
		return nullptr;
	}
	return content;
}

GLuint shader_compile(GLenum type, char const* const src, std::ostream& os) {
	auto const obj {glCreateShader(type)};
	if (!obj)
		return 0;
	glShaderSource(obj, 1, &src, nullptr);
	glCompileShader(obj);
	GLint status;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
		auto log {new (std::nothrow) char[length+1]};
		if (!log) {
			os << "Unable to allocate buffer of size " << length+1 << " bytes to load error log from compiling shader.\n";
			return 0;
		}
		glGetShaderInfoLog(obj, length, nullptr, log);
		os << log << '\n';
		delete[] log;
		glDeleteShader(obj);
		return 0;
	}
	return obj;
}

GLuint shader_link(size_t n, GLuint const obj[], std::ostream& os) {
	auto const pro {glCreateProgram()};
	if (!pro)
		return 0;
	for (size_t i {0}; i < n; ++i)
		glAttachShader(pro, obj[i]);
	glLinkProgram(pro);
	GLint status;
	glGetProgramiv(pro, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetProgramiv(pro, GL_INFO_LOG_LENGTH, &length);
		auto log {new (std::nothrow) char[length+1]};
		if (!log) {
			os << "Unable to allocate buffer of size " << length+1 << " bytes to load error log from linking shader.\n";
			return 0;
		}
		glGetProgramInfoLog(pro, length, nullptr, log);
		os << log << '\n';
		delete[] log;
		glDeleteProgram(pro);
		return 0;
	}
	for (size_t i {0}; i < n; ++i)
		glDetachShader(pro, obj[i]);
	return pro;
}

#include <functional>

class cleanup {
public:
	cleanup(std::function<void()> p) : f {p} {}
	~cleanup() { f(); }
private:
	std::function<void()> f;
};

#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	if (!glfwInit())
		return -1;
	cleanup c_glfw {[]{ glfwTerminate(); }};

	GLFWwindow* window;
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		window = glfwCreateWindow(winfo.w, winfo.h, winfo.title, nullptr, nullptr);
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
		std::cout << "OpenGL " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
	}

	GLuint sha;
	{
		size_t constexpr n {2};

		char const* const filename[] {"shader.vert", "shader.frag"};
		GLenum const type[] {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

		GLuint obj[n];
		for (size_t i {0}; i < n; ++i) {
			auto const src {file_content(filename[i], std::cerr)};
			if (!src)
				return -1;
			auto const o {shader_compile(type[i], src, std::cerr)};
			if (!o)
				return -1;
			obj[i] = o;
			delete[] src;
		}

		sha = shader_link(2, obj, std::cerr);
		if (!sha)
			return -1;

		for (size_t i {0}; i < n; ++i)
			glDeleteShader(obj[i]);
	}
	glUseProgram(sha);

	auto const uniform_tex {glGetUniformLocation(sha, "tex")};

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

		auto const attrib_position {glGetAttribLocation(sha, "vPosition")};
		glEnableVertexAttribArray(attrib_position);
		glVertexAttribFormat(attrib_position, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(attrib_position, bindingindex);

		auto const attrib_texcoord {glGetAttribLocation(sha, "vTexCoord")};
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
			glDeleteTextures(1, &id);
			glCreateTextures(GL_TEXTURE_2D, 1, &id);
			glTextureStorage2D(id, 1, GL_RGBA8, w, h);
		}
		void bind(GLuint uniform) {
			glBindTextureUnit(uniform, id);
		}
		void data(size_t w, size_t h, GLuint const* image) {
			glTextureSubImage2D(id, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);
		}
		GLuint id {0};
		GLsizei w {0};
		GLsizei h {0};
	}
	texture;

	std::chrono::microseconds const r_limit {16666}; /* 60Hz */
	auto r_last {std::chrono::steady_clock::now()};

	std::cout << std::flush;

	while (!glfwWindowShouldClose(window)) {
		if (image.renew(winfo.w, winfo.h)) {
			if (image.w != static_cast<size_t>(texture.w) || image.h != static_cast<size_t>(texture.h)) {
				texture.resize(image.w, image.h);
				texture.bind(uniform_tex);
			}
			texture.data(image.w, image.h, image.data);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glfwSwapBuffers(window);
		}
		glfwPollEvents();
		std::this_thread::sleep_for(r_limit - (std::chrono::steady_clock::now() - r_last));
		r_last = std::chrono::steady_clock::now();
	}

	return 0;
}
