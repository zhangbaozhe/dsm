/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-28 14:59:26 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-17 17:05:36
 */


#include <thread>
#include <mutex>
#include <iostream>



int main(int argc, char *argv[]) {
  std::mutex m;

  int i = 0;

  auto f1 = [&]() {
    for (int j = 0; j < 1000; j++) {
      m.lock();
      i += 1;
      std::cout << "f1 " << i << std::endl;
      m.unlock();
    }
  };

  auto f2 = [&]() {
    for (int j = 0; j < 1000; j++) {
      m.lock();
      i += 1;
      std::cout << "f2 " << i << std::endl;
      m.unlock();
    }
  };

  std::thread t1(f1);
  std::thread t2(f2);

  t1.join();
  t2.join();
}