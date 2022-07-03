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
