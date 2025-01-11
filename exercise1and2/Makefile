all: main main-debug

CXX = clang++
override CXXFLAGS += -g -Wmost -Werror -I/usr/include/gegl-0.4 -I./OpenCL-Wrapper/src -I/home/users8/acgl/s0248735/Documents/abschluss/OpenCL-Wrapper/src/OpenCL/include
LDFLAGS = -L/usr/lib64
LDLIBS = /usr/lib64/libOpenCL.so.1

# List source files explicitly, excluding OpenCL-Wrapper/src/main.cpp
SRCS = ./Grid.cpp ./OpenCL-Wrapper/src/kernel.cpp ./CLI.cpp
HEADERS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.h' -print)

KERNEL_SRC = /home/users8/acgl/s0248735/Documents/abschluss/game_of_life.cl
KERNEL_DEST = .

main: $(SRCS) $(HEADERS) $(KERNEL_DEST)/game_of_life.cl Main.cpp
	$(CXX) $(CXXFLAGS) -O3 -fno-tree-vectorize $(SRCS) Main.cpp -o "$@" $(LDFLAGS) $(LDLIBS)

main-debug: $(SRCS) $(HEADERS) $(KERNEL_DEST)/game_of_life.cl Main.cpp
	$(CXX) $(CXXFLAGS) -U_FORTIFY_SOURCE -O0 $(SRCS) Main.cpp -o "$@" $(LDFLAGS) $(LDLIBS)

$(KERNEL_DEST)/game_of_life.cl: $(KERNEL_SRC)
	cp $(KERNEL_SRC) $(KERNEL_DEST)

clean:
	rm -f main main-debug $(KERNEL_DEST)/game_of_life.cl
