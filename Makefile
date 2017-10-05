.PHONY: all info clean

# Use gcc as default compiler and linker
# May be changed by passing arguments to make
ifeq ($(origin CC), default)
    CC=g++
endif
ifeq ($(origin LD), default)
    LD=g++
endif

# "../" should point to the directory that contains the "bitbuffers" subdirectory
CFLAGS  = -I../ -O0 -g 

SOURCES = $(shell ls *.cpp)

OBJECTS = $(foreach o, $(SOURCES:.cpp=.o), build/$o)

all: $(OBJECTS)

build/%.o: %.cpp %.h
	@echo "\033[01;32m=> Compiling '$<' ...\033[00;00m"
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@
	@echo ""

info:
	@echo "Compiler is \"$(CC)\" defined by $(origin CC)"
	@echo "Linker is \"$(LD)\" defined by $(origin LD)"
	@echo "$(OBJECTS)"

clean:
	rm -f $(OBJECTS)




