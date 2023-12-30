#include <stdlib.h>
#include <iostream>
#include <boost/asio.hpp>

using namespace std;

int main(int argc, char* argv)
{
    try
    {
        cout << "client start...." << endl;
        //io_service对象
        boost::asio::io_service ios;
        //创建socket对象
        boost::asio::ip::tcp::socket sock(ios);

        //创建连接端
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"),6688);
        //连接
        sock.connect(ep);

        // 设置禁用 Nagle 算法
        boost::asio::ip::tcp::no_delay option(true);
        sock.set_option(option);

        //定义一个缓冲区
        //char data[128];
        char msgToServer[256];
        char msgFromServer[256];
        for(int i = 0; i<3; i++)
        {
            std::memset(msgToServer, 0x00, sizeof(msgToServer));
            std::memset(msgFromServer, 0x00, sizeof(msgFromServer));
            
            string tmp = "client_msg_" + to_string(i);
            strcpy(msgToServer, tmp.c_str());
            
            cout << "msgToServer: " << msgToServer <<endl;
 
            boost::asio::write(sock,boost::asio::buffer(msgToServer));
            
            // writing server dealing with the message...
 
            boost::asio::read(sock,boost::asio::buffer(msgFromServer));
            cout << "dataHandled: " << msgFromServer << endl; 
            
        }
    }
    catch (exception& e)
    {
        cout << e.what()<< endl;
    }
    std::cout << "Hello, world!" << std::endl;

    system("pause");
    return 0;
}