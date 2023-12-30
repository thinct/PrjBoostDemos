#include <iostream>
#include <boost/thread/thread.hpp>

void threadFunction() {
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread running: " << i << std::endl;
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));  // 休眠1秒
    }
}

int main() {
    // 启动一个新线程
    boost::thread myThread(&threadFunction);

    // 主线程继续执行其他任务
    for (int i = 0; i < 3; ++i) {
        std::cout << "Main thread running: " << i << std::endl;
        boost::this_thread::sleep(boost::posix_time::milliseconds(500));  // 休眠0.5秒
    }

    // 等待子线程结束
    myThread.join();

    return 0;
}
