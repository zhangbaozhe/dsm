/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 13:26:09 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 20:25:28
 */

#pragma once

#include <string>

namespace dsm {

struct Peer {
  std::string address;
  int port;
}; // class Peer

} // namespace dsm