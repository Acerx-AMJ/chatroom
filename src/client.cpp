#include <SFML/Network.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>

inline constexpr size_t color_count = 16u;
inline constexpr const char* res = "\033[0m";
inline constexpr const char* colors[color_count] = {
   "\033[38;2;255;128;255m", "\033[38;2;255;128;0m",   "\033[38;2;255;255;0m",
   "\033[38;2;128;255;0m",   "\033[38;2;0;255;0m",     "\033[38;2;0;255;255m",
   "\033[38;2;0;0;255m",     "\033[38;2;128;0;255m",   "\033[38;2;139;69;19m",
   "\033[38;2;84;57;44m",    "\033[38;2;0;0;0m",       "\033[38;2;64;64;64m",
   "\033[38;2;128;128;128m", "\033[38;2;192;192;192m", "\033[38;2;255;255;255m"
};

inline void clear_line(int lines = 1)
{
   for (int i = 0; i <= lines; ++i)
   {
      std::cout << "\r\033[K";
      if (i < lines)
         std::cout << "\033[F";
   }
   std::cout << std::flush;
}

inline std::string get_valid_username(const std::string& prompt)
{
   std::cout << prompt;

   std::string username;
   std::getline(std::cin >> std::ws, username);

   while (username.size() > 24 || username.size() < 3)
   {
      clear_line();
      std::cout << "Invalid username length, try again: ";
      std::getline(std::cin >> std::ws, username);
   }
   return username;
}

std::atomic<bool> quit = false;
std::mutex mutex;

void send_messages(sf::TcpSocket& socket, const std::string& username);
void receive_messages(sf::TcpSocket& socket);

int main() {
   srand(time(nullptr));

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
   if (socket.connect(IPAddress, port) != sf::Socket::Status::Done)
      throw std::runtime_error("Could not connect to the given port!");

   socket.setBlocking(false);
   std::cout << "Connected to server!\n";

   std::string username = get_valid_username("Please input your username: ");
   username = colors[rand() % color_count] + username + res;

   sf::Packet startingPacket;
   startingPacket << username + " connected.";

   if (socket.send(startingPacket) != sf::Socket::Status::Done)
   {
      socket.disconnect();
      throw std::runtime_error("Could not send data!");
   }

   std::thread send(send_messages, std::ref(socket), username);
   std::thread receive(receive_messages, std::ref(socket));

   if (send.joinable())
      send.join();

   if (receive.joinable())
      receive.join();

   sf::Packet disconnectPacket;
   disconnectPacket << username + " disconnected.";
   socket.send(disconnectPacket);

   std::cout << "Disconnected." << std::endl;
   socket.disconnect();
}

void send_messages(sf::TcpSocket& socket, const std::string& username)
{
   while (true)
   {
      std::cout << "Input your message ('exit' to exit): ";

      std::string message;
      std::getline(std::cin >> std::ws, message);
      clear_line();

      if (message == "exit")
      {
         quit = true;
         std::cout << "Exiting..." << std::endl;
         return;
      }

      std::string full_message = username + ": " + message;
      std::cout << full_message << std::endl;

      sf::Packet packet;
      packet << full_message;

      std::lock_guard<std::mutex> lock (mutex);

      if (socket.send(packet) != sf::Socket::Status::Done)
      {
         socket.disconnect();
         throw std::runtime_error("Could not send data!");
      }
   }
}

void receive_messages(sf::TcpSocket& socket)
{
   while (true)
   {
      if (quit)
         return;

      sf::Packet received;

      std::lock_guard<std::mutex> lock (mutex);

      if (socket.receive(received) == sf::Socket::Status::Done)
      {
         std::string message;
         received >> message;

         clear_line(0);
         std::cout << message << std::endl;
         std::cout << "Input your message ('exit' to exit): " << std::flush;
      }
   }
}
