/*
 * @Brief: Peer struct
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 13:26:09 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-18 14:03:31
 */

#pragma once

#include <string>

namespace dsm {

struct Peer {
  std::string address;
  int port;
  int id = -1;
}; // class Peer

} // namespace dsm