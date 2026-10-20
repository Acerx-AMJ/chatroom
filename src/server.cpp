#include <SFML/Network.hpp>
#include <memory>

int main() {
   sf::UdpSocket socket;
   if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
      printf("Could not bind to port %d!\n", socket.getLocalPort());
      return EXIT_FAILURE;
   }

   printf("Server listening on %s:%d...\n", sf::IpAddress::getLocalAddress().toString().c_str(), socket.getLocalPort());
   sf::TcpListener listener;

   if (listener.listen(socket.getLocalPort()) != sf::Socket::Status::Done) {
      printf("Could not connect listener!\n");
      return EXIT_FAILURE;
   }

   sf::SocketSelector selector;
   selector.add(listener);

   std::vector<std::unique_ptr<sf::TcpSocket>> clients;

   while (true)
   {
      if (selector.wait())
      {
         if (selector.isReady(listener))
         {
            auto socket = std::make_unique<sf::TcpSocket>();
            if (listener.accept(*socket) != sf::Socket::Status::Done)
               throw std::runtime_error("Could not accept socket!");

            selector.add(*socket);
            clients.push_back(std::move(socket));
         }

         for (auto it = clients.begin(); it != clients.end();)
         {
            if (selector.isReady(**it))
            {
               sf::Packet packet;

               if ((*it)->receive(packet) != sf::Socket::Status::Done)
               {
                  selector.remove(**it);
                  it = clients.erase(it);
                  continue;
               }

               std::string data;
               packet >> data;
               printf("%s\n", data.c_str());

               sf::Packet echoPacket;
               echoPacket << data;

               for (auto& socket : clients)
               {
                  if (socket.get() == it->get())
                     continue;
                  
                  if (socket->send(echoPacket) != sf::Socket::Status::Done)
                     printf("Failed to echo user's message!\n");
               }
            }
            ++it;
         }
      }
   }
   printf("Server disconnected.\n");

   for (auto& socket : clients)
      socket->disconnect();
}