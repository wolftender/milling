CC := g++
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
EXECUTABLE := $(BIN_DIR)/program

IMGUI_SRC_DIR := libs/imgui
IMGUI_OBJ_DIR := obj/imgui

SRC := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/scenes/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC)) $(OBJ_DIR)/glad.o $(OBJ_DIR)/lodepng.o

IMGUI_SRC := $(wildcard $(IMGUI_SRC_DIR)/*.cpp)
IMGUI_OBJ := $(patsubst $(IMGUI_SRC_DIR)/%.cpp, $(IMGUI_OBJ_DIR)/%.o, $(IMGUI_SRC))

NFD_INC_DIR := libs/nativefiledialog/src/include
NFD_LIB_DIR := libs/nativefiledialog/build/lib/Release/x64

CPPFLAGS := -Iinc -I$(NFD_INC_DIR) -Ilibs/lodepng `pkg-config --cflags glfw3` -Ilibs/glad/include -Ilibs/imgui -Ilibs/implot --std=c++20
CFLAGS := -Wall -g
LDFLAGS :=
LDLIBS := `pkg-config --libs glfw3` `pkg-config --libs gtk+-3.0` -L$(NFD_LIB_DIR) -lnfd -ldl -lpthread

all: $(EXECUTABLE)
.PHONY: all

$(EXECUTABLE): $(OBJ) $(IMGUI_OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

$(IMGUI_OBJ_DIR):
	mkdir -p $@

$(IMPLOT_OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/glad.o: libs/glad/src/glad.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/lodepng.o: libs/lodepng/lodepng.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(IMGUI_OBJ_DIR)/%.o: $(IMGUI_SRC_DIR)/%.cpp | $(IMGUI_OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) -rv $(EXECUTABLE) $(OBJ_DIR)

-include $(OBJ:.o=.d)
