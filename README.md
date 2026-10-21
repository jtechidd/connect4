# Connect4

<img width="323" height="256" alt="image" src="https://github.com/user-attachments/assets/2e3378c9-369c-47db-8f1b-d745bcd225bd" />

---

Connect4 is a 2-player game where each turn a player will insert a token into a column slot; whoever makes a 4-in-a-row first will be the winner. This project provides game server and game client executables where the server will handle the matchmaking and in-game authoritative logic, and the client will be the UI, representational, and user-facing.

The implementation uses only C++, with `libuv` as an asynchronous framework. Networking in C/C++ could be done by traditional socket programming. However, I don't want to deal with that much boilerplate and would like to abstract the cross-platform modern polling things (epoll/kqueue/iofs). Also, `libuv` could provide other batteries-included asynchronous operations such as files, timers, etc.

Since this project is at the same complexity as TCP socket programming, the messaging format is agnostic. This project is using `protobuf` for messaging since it can serialize data into a byte format, and the data can be much smaller compared to JSON or another human-readable format. However, TCP does not frame the data by default. There will be some basic messaging design and internal usage of a ring buffer, which helps buffer and consume the data stream.

The server is run in a single thread; with the power of `libuv`, it will manage to wake up the process only when an event is ready. This could benefit a lot in serving a large scale of clients. The client itself has 2 threads, where the main thread runs the `libqt` UI main loop, and another runs the `libuv` main loop. I hope that the communication between these two will be safe (I have tried utilizing `QMetaObject::invokeMethod` for `libqt` and using `uv_async_send` for `libuv` as cross-thread operations)

**Final disclaimer**: This is provided as a fun project for demonstrating an implementation of a basic TCP game server with C++. This is not a production-ready application, and there are a lot of things that can be improved. Any ideas, issues, or pull requests that are raised to help improve this are welcome.

---

## How to run this project?

I have implemented this on an Ubuntu-like Linux distro. To run this project, first run `make dependencies` to install dependencies, and then run `make all` to build everything. You can find all built executables in the `./build` folder. Since this project is not cross-platform, any support for `cmake` or for making this project cross-platform is welcome.
