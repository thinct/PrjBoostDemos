#include <stdlib.h>
#include <iostream>

#include "CommuniteThreader.hpp"

using namespace std;

int main(int argc, char* argv) {
  try {
    CommuniteThreader server(6688);

    server.RegistActionFunc(
        "Action", [](boost::asio::ip::tcp::socket* socket,
                         std::vector<std::string> args) {
          // Handle the action here
          std::cout << "Received someAction: ";
          for (const std::string& arg : args) {
            std::cout << arg << ", ";
          }
          std::cout << std::endl;

          // Example: Echo back the received data
          std::string response = "Server received: ";
          for (const std::string& arg : args) {
            response += arg + CMD_SPLIT_OPR;
          }
          response += CMD_SPLIT_WITH_END_OPR;
          boost::asio::write(*socket, boost::asio::buffer(response));

          return 0;
        });

    server.RegistActionFunc("Quit", [&](boost::asio::ip::tcp::socket* socket,
                                        std::vector<std::string> args) {
      server.Quit();
      return 0;
    });
    // Add more action handlers as needed...

    server.RunServer();
  } catch (exception& e) {
    cout << e.what() << endl;
  }
  system("pause");
  return 0;
}