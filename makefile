OUT := image
OBJS := main.o lib/gl3w.o
DEPS := $(OBJS:%.o=%.d)

CC := x86_64-w64-mingw32-gcc-posix
CFLAGS := -std=c17

CXX := x86_64-w64-mingw32-g++-posix
CXXFLAGS := -std=c++17

CPPFLAGS := -Iinclude

LDFLAGS := -Llib
LDLIBS := -lglfw3 -lgdi32 -static-libgcc -static-libstdc++ -static -lpthread

DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

$(OUT): $(OBJS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o .$@
	mv .$@ $@

%.o: %.c
	$(COMPILE.c) $(DEPFLAGS) $(OUTPUT_OPTION) $<

%.o: %.cc
	$(COMPILE.cc) $(DEPFLAGS) $(OUTPUT_OPTION) $<

.PHONY: clean
clean:
	rm -f $(DEPS)
	rm -f $(OBJS)
	rm -f $(OUT)

include $(wildcard $(DEPS))

$(OBJS): | .setup

.setup:
	mkdir -p include/GL include/GLFW include/KHR
	mkdir -p lib
	mkdir -p external
	cd external && if [ ! -d gl3w ]; then git clone https://github.com/skaslev/gl3w; fi
	cd external && wget -N https://github.com/glfw/glfw/releases/download/3.3.7/glfw-3.3.7.bin.WIN64.zip
	cd external/gl3w && python3 gl3w_gen.py
	cd external && unzip -n glfw-3.3.7.bin.WIN64.zip
	cp external/gl3w/include/GL/glcorearb.h include/GL/
	cp external/gl3w/include/GL/gl3w.h include/GL/
	cp external/gl3w/include/KHR/khrplatform.h include/KHR/
	cp external/gl3w/src/gl3w.c lib/
	cp external/glfw-3.3.7.bin.WIN64/include/GLFW/glfw3.h include/GLFW/
	cp external/glfw-3.3.7.bin.WIN64/lib-mingw-w64/libglfw3.a lib/
	touch .setup
