CXX = g++
CXXFLAGS = -g -Wall -Wextra -Wl,--no-undefined,-rpath,./build -O2 -std=c++17 -fPIC -I./include \
	$(shell pkg-config --cflags spdlog) \
	$(shell pkg-config --cflags Qt6Widgets)
LDFLAGS = -L./build -luv -lprotobuf -lspdlog -lfmt $(shell pkg-config --libs Qt6Widgets)

PROTOS = src/message.proto
COMPILED_PROTOS = \
	$(PROTOS:src/%.proto=include/%.pb.h) \
	$(PROTOS:.proto=.pb.cc)
LIB_SRCS = \
	src/common.cpp \
	src/server.cpp \
	src/server_client_connection.cpp \
	src/server_message_handler.cpp \
	src/ring_buffer.cpp \
	src/game.cpp \
	src/client.cpp \
	src/client_message_handler.cpp \
	src/client_ui.cpp \
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

$(LIB): $(LIB_SRCS) $(LIB_IMGUI)
	$(CXX) $(CXXFLAGS) -shared $(LIB_SRCS) -o $(LIB) $(LDFLAGS)

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
		bear \
		libspdlog-dev \
		libuv1-dev \
		protobuf-compiler \
		qt6-base-dev \
		qt6-declarative-dev \
		libgl1-mesa-dev