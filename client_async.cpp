#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

namespace BoostAsio
{
	static const unsigned int s_nBufferLength = 1024;
	void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred, char* buffer, tcp::socket& sock) {
		if (!error) {
			// 处理接收到的数据
			cout << "dataHandled: " << buffer << endl;

			// 如果读取到 'Q'，退出循环
			if (strcmp(buffer, "Q") == 0) {
				cout << "Received 'Q', exiting." << endl;
				sock.close();
				return;
			}

			// 继续异步读取
			memset(buffer, 0, s_nBufferLength);
			boost::asio::async_read(sock, boost::asio::buffer(buffer, s_nBufferLength),
				boost::asio::transfer_at_least(1),
				std::bind(handleRead, std::placeholders::_1, std::placeholders::_2, buffer, std::ref(sock)));
		}
		else {
			cerr << "Error during async read: " << error.message() << endl;
		}
	}

	void handleConnect(const boost::system::error_code& error, tcp::socket& sock) {
		if (!error) {
			cout << "Connected to the server." << endl;

			// 禁用 Nagle 算法
			boost::asio::ip::tcp::no_delay option(true);
			sock.set_option(option);

			// 异步读取数据
			static char msgFromServer[s_nBufferLength]{ 0 };
			boost::asio::async_read(sock, boost::asio::buffer(msgFromServer, sizeof(msgFromServer)),
				boost::asio::transfer_at_least(1),
				std::bind(handleRead, std::placeholders::_1, std::placeholders::_2, msgFromServer, std::ref(sock)));
		}
		else {
			cerr << "Error during async connect: " << error.message() << endl;
		}
	}

} // namespace BoostAsio

int main() {
	boost::asio::io_context ioContext;
	tcp::socket sock(ioContext);

	// 异步连接到服务器
	sock.async_connect(tcp::endpoint(address::from_string("127.0.0.1"), 6688), std::bind(BoostAsio::handleConnect, std::placeholders::_1, std::ref(sock)));

	// 运行 io_context，直到所有异步操作完成
	ioContext.run();

	return 0;
}
