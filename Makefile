.PHONY: all info clean library test

# Use gcc as default compiler and linker
# May be changed by passing arguments to make
ifeq ($(origin CC), default)
    CC=g++
endif
ifeq ($(origin LD), default)
    LD=g++
endif

CFLAGS  = -O0 -g

SOURCES = $(shell ls *.cpp)

OBJECTS = $(foreach o, $(SOURCES:.cpp=.o), build/$o)

USES = bitbuffers-cpp propertynode-cpp

CFLAGS += $(foreach u, $(USES), -I./$u/)

all: library

build/%.o: %.cpp %.h
	@echo "\033[01;32m=> Compiling '$<' ...\033[00;00m"
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@
	@echo ""

library: $(OBJECTS)
	@echo "\033[01;33m==> Creating static library '$@':\033[00;00m"
	@mkdir -p build/
	ar rcs build/libgmt-cpp.a $(OBJECTS)
	@echo ""

test: library
	@echo "\033[01;33m==> Compiling tests ...\033[00;00m"
	$(MAKE) -C test

info:
	@echo "Compiler is \"$(CC)\" defined by $(origin CC)"
	@echo "Linker is \"$(LD)\" defined by $(origin LD)"
	@echo "$(OBJECTS)"

clean:
	rm -rf build/




