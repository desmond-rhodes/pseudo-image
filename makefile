OUT := image
OBJS := main.o

CXXFLAGS := -std=c++17

DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

$(OUT): $(OBJS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.o: %.cc
	$(COMPILE.cc) $(DEPFLAGS) $(OUTPUT_OPTION) $<

.PHONY: clean
clean:
	rm -f *.d
	rm -f *.o
	rm -f $(OUT)

include $(wildcard $(OBJS:%.o=%.d))
