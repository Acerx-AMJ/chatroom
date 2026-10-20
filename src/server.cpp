#include <SFML/Network.hpp>
#include <memory>

int main() {
   sf::UdpSocket socket;
   if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
      printf("Could not bind to port %d!\n", socket.getLocalPort());
      exit(EXIT_FAILURE);
   }

   printf("Server listening on %s:%d...\n", sf::IpAddress::getLocalAddress().toString().c_str(), socket.getLocalPort());
   sf::TcpListener listener;

   if (listener.listen(socket.getLocalPort()) != sf::Socket::Status::Done) {
      printf("Could not connect listener!\n");
      exit(EXIT_FAILURE);
   }

   sf::SocketSelector selector;
   selector.add(listener);

   std::vector<std::unique_ptr<sf::TcpSocket>> clients;

   while (true) {
      if (!selector.wait()) {
         continue;
      }
   
      if (selector.isReady(listener)) {
         auto socket = std::make_unique<sf::TcpSocket>();
         if (listener.accept(*socket) != sf::Socket::Status::Done) {
            printf("Could not accept socket!\n");
            exit(EXIT_FAILURE);
         }

         selector.add(*socket);
         clients.push_back(std::move(socket));
      }

      for (auto it = clients.begin(); it != clients.end();) {
         if (!selector.isReady(**it)) {
            it = std::next(it);
            continue;
         }
         sf::Packet packet;

         if ((*it)->receive(packet) != sf::Socket::Status::Done) {
            selector.remove(**it);
            it = clients.erase(it);
            continue;
         }

         std::string data;
         packet >> data;
         printf("%s\n", data.c_str());

         sf::Packet echoPacket;
         echoPacket << data;

         for (std::unique_ptr<sf::TcpSocket> &socket: clients) {
            if (socket.get() == it->get()) {
               continue;
            }

            if (socket->send(echoPacket) != sf::Socket::Status::Done) {
               printf("Failed to echo user's message!\n");
            }
         }
         it = std::next(it);
      }
   }

   printf("Server disconnected.\n");
   for (std::unique_ptr<sf::TcpSocket> &socket: clients) {
      socket->disconnect();
   }
}