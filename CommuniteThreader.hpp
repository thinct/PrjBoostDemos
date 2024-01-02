#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <map>

#define CMD_SPLIT_OPR          "-|-"
#define CMD_SPLIT_LISTOPR      "-;-"
#define CMD_SPLIT_WITH_END_OPR "-$-"

#define QDbgLn(_SENTENCE_) std::cout << __LINE__ << "\t" << _SENTENCE_ << std::endl

class CommuniteThreader {
public:
    using CallBackFunc = std::function<long(boost::asio::ip::tcp::socket*, std::vector<std::string>)>;

    CommuniteThreader(unsigned int uPort)
        : m_uPort(uPort), m_ioService(), m_acceptor(m_ioService), m_socket(m_ioService),
          m_bQuitWaitFlag(true)
    {
    }

    void RegistActionFunc(std::string strActionName, CallBackFunc func)
    {
        m_funcReg[strActionName] = func;
    }

    void Quit(unsigned int timeout) { 
        m_bQuitWaitFlag = false;
    }

    void RunServer() {
        // 启动一个新线程
        std::thread cmdServiceThread([this] {
          try {
            m_acceptor.open(boost::asio::ip::tcp::v4());
            m_acceptor.set_option(
                boost::asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor.bind(boost::asio::ip::tcp::endpoint(
                boost::asio::ip::tcp::v4(), m_uPort));
            m_acceptor.listen();

            std::cout << "Server started on port " << m_uPort << std::endl;

            m_ioThread = std::thread([this]() {
              std::cout << __LINE__ << std::endl;
              m_ioService.run();
            });

            StartAccept();
          } catch (const std::exception& e) {
            std::cerr << "Exception in RunServer: " << e.what() << std::endl;
          }

          // wait for quit
          while (true) {
            if (!m_bQuitWaitFlag) {
              break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
          }
          m_socket.close();
          m_acceptor.close();
          m_ioService.stop();
        });
        cmdServiceThread.join();
        m_ioThread.join();
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

    // 使用 find 函数和 substr 函数拆分字符串
    static std::vector<std::string> SplitCMDString(std::string dataClone,
                                                   std::string delimiter) {
        std::vector<std::string> cmdArgParseByEndFlag;
        std::string cmdArgOnce;
        size_t pos = 0;
        while ((pos = dataClone.find(delimiter)) != std::string::npos) {
            cmdArgOnce = dataClone.substr(0, pos);
            cmdArgParseByEndFlag.push_back(cmdArgOnce);
            dataClone.erase(0, pos + delimiter.length());
        }
        if (!dataClone.empty()) {
            cmdArgParseByEndFlag.push_back(dataClone);
        }
        return cmdArgParseByEndFlag;
    }

    void HandleRead(size_t bytes_transferred)
    {
        std::istream is(&m_receiveBuffer);
        std::string data;
        std::getline(is, data);
        QDbgLn(data);

        // 使用 find 函数和 substr 函数拆分字符串
        std::vector<std::string> cmdArgParseByEndFlag =
            SplitCMDString(data, CMD_SPLIT_WITH_END_OPR);
        QDbgLn(cmdArgParseByEndFlag.size());

        for (const std::string& cmdArgOnce : cmdArgParseByEndFlag) {
            QDbgLn(cmdArgOnce);
            std::vector<std::string> cmdArgSet =
                SplitCMDString(cmdArgOnce, CMD_SPLIT_LISTOPR);
            QDbgLn(cmdArgSet.size());

            for (const std::string& cmdArgItem : cmdArgSet) {
              std::vector<std::string> cmdArgs =
                  SplitCMDString(cmdArgItem, CMD_SPLIT_OPR);
              QDbgLn(cmdArgs.size());

              if (!cmdArgs.empty()) {
                std::string strCmd = cmdArgs.front();
                cmdArgs.erase(cmdArgs.begin());
                QDbgLn("m_funcReg..." << strCmd << ": len=" << cmdArgs.size());
                if (m_funcReg.find(strCmd) != m_funcReg.end()) {
                  // Only the synchronous mode is considered
                  m_funcReg[strCmd](&m_socket, cmdArgs);
                } else {
                  QDbgLn("unknown func register...");
                }

                QDbgLn("m_funcReg...");
              }
            }
        }

        // Continue reading
        if (!m_ioService.stopped() && m_bQuitWaitFlag) {
            StartRead();
        }
    }

private:
    bool                                m_bQuitWaitFlag;
    unsigned int                        m_uPort;
    boost::asio::io_service             m_ioService;
    boost::asio::ip::tcp::acceptor      m_acceptor;
    boost::asio::ip::tcp::socket        m_socket;
    std::thread                         m_ioThread;
    boost::asio::streambuf              m_receiveBuffer;
    std::map<std::string, CallBackFunc> m_funcReg;
};
