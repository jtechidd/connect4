CXX = g++
CXXFLAGS = -g -Wall -Wextra -Wl,--no-undefined,-rpath,./build -O2 -std=c++17 -fPIC -I./include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -DSPDLOG_COMPILED_LIB
LDFLAGS = -L./build -luv -lprotobuf -lspdlog -lfmt -lGL -lSDL3 -ldl

PROTOS = src/message.proto
COMPILED_PROTOS = \
	$(PROTOS:src/%.proto=include/%.pb.h) \
	$(PROTOS:.proto=.pb.cc)
IMGUI_DIR = ./third_party/imgui
IMGUI_SRCS = \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
LIB_IMGUI = build/libimgui.so
LIB_SRCS = \
	src/common.cpp \
	src/server.cpp \
	src/server_session.cpp \
	src/server_message_handler.cpp \
	src/ring_buffer.cpp \
	src/game.cpp \
	src/client.cpp \
	src/client_message_handler.cpp \
	src/client_ui.cpp \
	src/client_ui_boilerplate.cpp \
	$(COMPILED_PROTOS)
LIB = build/libc4.so
SERVER_SRC = src/server_main.cpp
SERVER = build/server
CLIENT_SRC = src/client_main.cpp
CLIENT = build/client

$(COMPILED_PROTOS): $(PROTOS)
	protoc --proto_path=./src --cpp_out=. $(PROTOS)
	mv $(PROTOS:src/%.proto=%.pb.h) include
	mv $(PROTOS:src/%.proto=%.pb.cc) src

$(LIB_IMGUI): $(IMGUI_SRCS)
	$(CXX) $(CXXFLAGS) -shared $(IMGUI_SRCS) -o $(LIB_IMGUI) $(LDFLAGS)

$(LIB): $(LIB_SRCS) $(LIB_IMGUI)
	$(CXX) $(CXXFLAGS) -shared $(LIB_SRCS) -o $(LIB) $(LDFLAGS) -limgui

$(SERVER): $(LIB) $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o $(SERVER) $(LDFLAGS) -lc4
run_server: $(SERVER)
	$(SERVER)

$(CLIENT): $(LIB) $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o $(CLIENT) $(LDFLAGS) -lc4
run_client: $(CLIENT)
	$(CLIENT)

all: $(LIB_IMGUI) $(LIB) $(SERVER) $(CLIENT)

.PHONY: clean
clean:
	rm -rf build/* \
	rm -r $(COMPILED_PROTOS)

.PHONY: dependencies
dependencies:
	sudo apt-get update
	sudo apt-get install -y \
		make \
		build-essential \
		libspdlog-dev \
		libuv1-dev \
		protobuf-compiler \
		libsdl3-dev \
		mesa-utils \
		libgl1-mesa-dev \
		libglu1-mesa-dev