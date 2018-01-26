.PHONY: all info clean

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

all: test

test: $(SOURCES) ../build/libgmt-cpp.a
	@echo "\033[01;32m=> Compiling '$<' ...\033[00;00m"
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $(SOURCES) -lgmt-cpp -L../build -I../ -o $@
	@echo ""

info:
	@echo "Compiler is \"$(CC)\" defined by $(origin CC)"
	@echo "Linker is \"$(LD)\" defined by $(origin LD)"

clean:
	rm -f test




