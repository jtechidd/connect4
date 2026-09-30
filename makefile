CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -fPIC -I./include -DSPDLOG_COMPILED_LIB -Wl,-rpath,./build
LDFLAGS = -L./build -luv -lprotobuf -lspdlog -lc4

PROTOS = src/message.proto

COMPILED_PROTOS = \
	$(PROTOS:src/%.proto=include/%.pb.h) \
	$(PROTOS:.proto=.pb.cc) \

LIB_SRCS = \
	src/common.cpp \
	src/server.cpp \
	src/server_session.cpp \
	src/server_message_handler.cpp \
	src/ring_buffer.cpp \
	src/game.cpp \
	src/client.cpp \
	src/client_message_handler.cpp \
	$(COMPILED_PROTOS)

LIB = build/libc4.so

SERVER = build/server
SERVER_SRC = src/server_main.cpp

CLIENT = build/client
CLIENT_SRC = src/client_main.cpp

$(COMPILED_PROTOS): $(PROTOS)
	protoc --proto_path=./src --cpp_out=. $(PROTOS)
	mv $(PROTOS:src/%.proto=%.pb.h) include
	mv $(PROTOS:src/%.proto=%.pb.cc) src

$(LIB): $(LIB_SRCS)
	$(CXX) $(CXXFLAGS) -shared $(LIB_SRCS) -o $(LIB) $(LDFLAGS)

$(SERVER): $(LIB) $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $(SERVER) ./src/server_main.cpp $(LDFLAGS)

run_server: $(SERVER)
	$(SERVER)

$(CLIENT): $(LIB) $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o ./build/client ./src/client_main.cpp $(LDFLAGS)

run_client: $(CLIENT)
	./build/client

all: $(LIB) $(SERVER) $(CLIENT)

.PHONY: clean
clean:
	rm -rf build/* \
	rm -r $(COMPILED_PROTOS)

.PHONY: dependencies
dependencies:
	sudo apt-get update
	sudo apt-get install -y make build-essential libspdlog-dev libuv1-dev protobuf-compiler