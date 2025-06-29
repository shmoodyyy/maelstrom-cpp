SRC_DIR = ./src
SRC = $(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJ_DIR = ./obj
OBJ = $(patsubst %.cpp,%.o,$(SRC))
INCLUDE = -I$(SRC_DIR) -I${GLIBC_INCLUDE}
CFLAGS = -Wall -pedantic
CXXFLAGS = -std=c++23 -fno-exceptions
OUT_DIR = ./bin
OUT = $(OUT_DIR)/node.run

.PHONY: build
build: $(OUT)

$(OUT): $(OBJ)
	mkdir -p $(OUT_DIR)
	mkdir -p $(OBJ_DIR)
	g++ $(OPTFLAGS) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) -o $(OUT) $(OBJ)

src/%.o: src/%.cpp
	g++ $(OPTFLAGS) $(LD_FLAGS) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

.PHONY: clean
clean:
	-rm -f $(shell find -type f -iregex ".*\.o" 2>/dev/null || echo "")
	-rm -rf $(OUT_DIR) 2>/dev/null || true
	-rm -f $(OUT)
