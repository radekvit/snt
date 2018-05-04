CXX=g++-7.2
APPNAME=snt
INCLUDE=include
SRC=src
CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -I. -I $(INCLUDE) -I $(SRC) -I $(LIBINCLUDE)
OBJ=obj
DOC = docs
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(SRC)/*.hpp) $(wildcard $(INCLUDE)/*.h)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

.PHONY: all format clean debug build test pack doc run libbuild cleanall

all: deploy

init:
	git submodule init
	git submodule update

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o $@ $(LDLIBS)

$(OBJ)/asm_generator.o: src/intrinsics.asm
$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS) $(LIBHEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -r $(OBJFILES) $(APPNAME)

format:
	-clang-format -style=file -i $(SRC)/*.cpp $(SRC)/*.hpp $(INCLUDE)/*.h

test:
	$(MAKE) -C test test

run: all
	./$(APPNAME)

pack: clean
	zip -r xvitra00.zip dplc $(LIBDIR) $(SRC) $(INCLUDE) Makefile README.txt examples
