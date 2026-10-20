#include <SFML/Network.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>

constexpr size_t color_count = 15;
constexpr const char* res = "\e[0m";
constexpr const char* colors[color_count] = {
   "\e[38;2;255;128;255m", "\e[38;2;255;128;0m",   "\e[38;2;255;255;0m",
   "\e[38;2;128;255;0m",   "\e[38;2;0;255;0m",     "\e[38;2;0;255;255m",
   "\e[38;2;0;0;255m",     "\e[38;2;128;0;255m",   "\e[38;2;139;69;19m",
   "\e[38;2;84;57;44m",    "\e[38;2;0;0;0m",       "\e[38;2;64;64;64m",
   "\e[38;2;128;128;128m", "\e[38;2;192;192;192m", "\e[38;2;255;255;255m"
};

void flush() {
   std::cout << std::flush;
}

void clear_line(int lines = 1) {
   for (int i = 0; i < lines; ++i) {
      printf("\r\e[K\e[F");
   }
   printf("\r\e[K");
   flush();
}

std::string get_valid_username(const char *prompt) {
   printf("%s", prompt);
   std::string username;
   std::getline(std::cin >> std::ws, username);

   while (username.size() > 24 || username.size() < 3) {
      clear_line();
      printf("Invalid username length, try again: ");
      std::getline(std::cin >> std::ws, username);
   }
   return username;
}

void send_messages(sf::TcpSocket& socket, const std::string& username, std::atomic<bool> &quit, std::mutex &mutex) {
   while (true) {
      printf("Input your message ('exit' to exit): ");
      std::string message;
      std::getline(std::cin >> std::ws, message);
      clear_line();

      if (message == "exit") {
         quit = true;
         printf("Exiting...\n");
         return;
      }

      std::string full_message = username + ": " + message;
      printf("%s\n", full_message.c_str());

      sf::Packet packet;
      packet << full_message;
      std::lock_guard<std::mutex> lock (mutex);

      if (socket.send(packet) != sf::Socket::Status::Done) {
         socket.disconnect();
         printf("Could not send data!\n");
         exit(EXIT_FAILURE);
      }
   }
}

void receive_messages(sf::TcpSocket& socket, std::atomic<bool> &quit, std::mutex &mutex) {
   while (!quit) {
      sf::Packet received;
      std::lock_guard<std::mutex> lock (mutex);

      if (socket.receive(received) == sf::Socket::Status::Done) {
         std::string message;
         received >> message;

         clear_line(0);
         printf("%s\n", message.c_str());
         printf("Input your message ('exit' to exit): ");
         flush();
      }
   }
}

int main() {
   sf::IpAddress IPAddress;
   unsigned short port = 0;

   do {
      printf("Input server's IP: ");
      std::string IP;
      std::getline(std::cin >> std::ws, IP);
      IPAddress = sf::IpAddress(IP);
      if (IPAddress.toInteger() == 0) {
         printf("Invalid IP address format, try again. ");
      }
   }
   while (IPAddress.toInteger() == 0);

   printf("Input server's port: ");
   std::cin >> std::ws >> port;

   sf::TcpSocket socket;
   if (socket.connect(IPAddress, port) != sf::Socket::Status::Done) {
      printf("Could not connect to %s:%u.\n", IPAddress.toString().c_str(), port);
      exit(EXIT_FAILURE);
   }

   socket.setBlocking(false);
   printf("Connected to server!\n");

   srand(time(nullptr));
   std::string username = get_valid_username("Please input your username: ");
   username = colors[rand() % color_count] + username + res;

   sf::Packet startingPacket;
   startingPacket << username + " connected.";

   if (socket.send(startingPacket) != sf::Socket::Status::Done) {
      socket.disconnect();
      printf("Could not send data!\n");
      exit(EXIT_FAILURE);
   }

   std::atomic<bool> quit;
   std::mutex mutex;
   std::thread send(send_messages, std::ref(socket), username, std::ref(quit), std::ref(mutex));
   std::thread receive(receive_messages, std::ref(socket), std::ref(quit), std::ref(mutex));

   if (send.joinable()) {
      send.join();
   }

   if (receive.joinable()) {
      receive.join();
   }

   sf::Packet disconnectPacket;
   disconnectPacket << username + " disconnected.";
   socket.send(disconnectPacket);

   printf("Disconnected.\n");
   socket.disconnect();
}
