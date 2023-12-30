#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#define CMD_SPLIT_OPR          ";"
#define CMD_SPLIT_LISTOPR      ","
#define CMD_SPLIT_WITH_END_OPR "\n"

#define QDbgLn std::cout << __LINE__ << std::endl

class CommuniteThreader {
public:
    using CallBackFunc = std::function<long(boost::asio::ip::tcp::socket*, std::vector<std::string>)>;

    CommuniteThreader(unsigned int uPort)
        : m_uPort(uPort), m_ioService(), m_acceptor(m_ioService), m_socket(m_ioService)
    {
    }

    void RegistActionFunc(std::string strActionName, CallBackFunc func)
    {
        m_funcReg[strActionName] = func;
    }

    void Quit(unsigned int timeout = 5000 /*ms*/)
    {
        m_ioService.stop();
        m_ioThread.join();
    }

    void CloseServer()
    {
        m_socket.close();
        m_acceptor.close();
        m_ioService.stop();
        m_ioThread.join();
    }

    void RunServer()
    {
        try {
            m_acceptor.open(boost::asio::ip::tcp::v4());
            m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_uPort));
            m_acceptor.listen();

            std::cout << "Server started on port " << m_uPort << std::endl;

            m_ioThread = std::thread([this]() {
                m_ioService.run();
            });

            StartAccept();
        } catch (const std::exception& e) {
            std::cerr << "Exception in RunServer: " << e.what() << std::endl;
        }
    }

private:
    void StartAccept()
    {
        m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "New connection..." << std::endl;
                HandleClient();
            } else {
                std::cerr << "Error in StartAccept: " << ec.message() << std::endl;
            }
        });
    }

    void HandleClient()
    {
        boost::asio::async_write(m_socket, boost::asio::buffer("server ready..."),
            [this](const boost::system::error_code& ec, size_t /*bytes_written*/) {
                if (!ec) {
                    std::cout << "Server ready message sent..." << std::endl;
                    StartRead();
                } else {
                    std::cerr << "Error in HandleClient: " << ec.message() << std::endl;
                }
            });
    }

    void StartRead()
    {
        boost::asio::async_read_until(m_socket, m_receiveBuffer, CMD_SPLIT_WITH_END_OPR,
            [this](const boost::system::error_code& ec, size_t bytes_transferred) {
                if (!ec) {
                    HandleRead(bytes_transferred);
                } else {
                    std::cerr << "Error in StartRead: " << ec.message() << std::endl;
                }
            });
    }

    void HandleRead(size_t bytes_transferred)
    {
        std::istream is(&m_receiveBuffer);
        std::string data;
        std::getline(is, data);

        std::vector<std::string> cmdArgParseByEndFlag;
        std::istringstream ss(data);
        std::string cmdArgOnce;
        while (std::getline(ss, cmdArgOnce, CMD_SPLIT_WITH_END_OPR[0])) {
            cmdArgParseByEndFlag.push_back(cmdArgOnce);
        }

        for (const std::string& cmdArgOnce : cmdArgParseByEndFlag) {
            QDbgLn << cmdArgOnce;
            std::vector<std::string> cmdArgSet;
            std::istringstream ssSet(cmdArgOnce);
            std::string cmdArgItem;
            while (std::getline(ssSet, cmdArgItem, CMD_SPLIT_LISTOPR[0])) {
                cmdArgSet.push_back(cmdArgItem);
            }

            for (const std::string& cmdArgItem : cmdArgSet) {
                std::vector<std::string> cmdArgs;
                std::istringstream ssArgs(cmdArgItem);
                std::string cmdArg;
                while (std::getline(ssArgs, cmdArg, CMD_SPLIT_OPR[0])) {
                    cmdArgs.push_back(cmdArg);
                }

                if (!cmdArgs.empty()) {
                    std::string strCmd = cmdArgs.front();
                    QDbgLn << "m_funcReg..." << strCmd << ":";
                    cmdArgs.erase(cmdArgs.begin());
                    for (const std::string& arg : cmdArgs) {
                        QDbgLn << arg;
                    }

                    if (m_funcReg.find(strCmd) != m_funcReg.end()) {
                        // Only the synchronous mode is considered
                        m_funcReg[strCmd](&m_socket, cmdArgs);
                    } else {
                        QDbgLn << "unknown func register...";
                    }

                    QDbgLn << "m_funcReg...";
                }
            }
        }

        // Continue reading
        StartRead();
    }

private:
    unsigned int m_uPort;
    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    std::thread m_ioThread;
    boost::asio::streambuf m_receiveBuffer;
    std::map<std::string, CallBackFunc> m_funcReg;
};

int main()
{
    CommuniteThreader server(6688);

    server.RegistActionFunc("someAction", [](boost::asio::ip::tcp::socket* socket, std::vector<std::string> args) {
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
    });

    // Add more action handlers as needed...

    server.RunServer();
    server.Quit(); // Optional: use this to gracefully shutdown the server

    return 0;
}
