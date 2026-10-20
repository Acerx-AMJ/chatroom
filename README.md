# Chatroom
Terminal chatroom that you can chat with your friends in. Works in a LAN. Includes both the server and the client applications.

## Building
This project depends on C++17 and [SFML](https://www.sfml-dev.org/). The recommended way to build the project is using CMake. It will build both the server and the client executables:
```bash
cmake -B build
cmake --build build
```
Server will be located in `build/server` and the client in `build/client`. Or you can compile using your favorite compiler:
```bash
g++ -std=c++17 src/client.cpp -o client -lsfml-system -lsfml-network
g++ -std=c++17 src/server.cpp -o server -lsfml-system -lsfml-network
```

## Usage
First run the server:
```
./build/server
Server listening on 192.168.8.50:38698...
```

Server will tell your private IP and on which port it is listening. Open up as many clients as you wish to and input the IP and port when asked:
```
./build/client
Input server's IP: 192.168.8.50
Input server's port: 38698
Connected to server!
```
Input an username and you'll be good to go. The server will tell you when someone connects, disconnects or sends a message.

## Credits
This project was made by Daniel Vishnevsky and is licensed under MIT License. It was made in February of 2025 and republished as an effort to make all of my projects open-source.
