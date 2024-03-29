CXX=g++-7.3
APPNAME=snt
TESTER=tester
INCLUDE=include
SRC=src
CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -I. -I $(INCLUDE) -I $(SRC)
OBJ=obj
DOC = docs
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(SRC)/*.hpp) $(wildcard $(INCLUDE)/*.h)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

.PHONY: all format clean debug build test pack doc run libbuild cleanall

all: deploy $(TESTER)

$(TESTER): checksln.cpp
	$(CXX) -std=c++17 -O3 checksln.cpp -o $(TESTER)

init:
	git submodule init
	git submodule update

build: $(APPNAME)

debug: CXXFLAGS+=-g -pg -O0
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
	zip -r xvitra00.zip $(SRC) $(INCLUDE) Makefile xvitra00.pdf checksln.cpp tests.zip
