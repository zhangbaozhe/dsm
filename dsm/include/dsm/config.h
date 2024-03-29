/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 13:44:29 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-29 13:51:38
 */

#pragma once

#include "dsm/peer.h"

#include <string>
#include <vector>

namespace dsm {

struct Config {

  Config(const std::string &address, uint16_t port, const std::vector<Peer> peers) : 
    address(address), 
    port(port), 
    peers(peers) {}

  Config()
  
  std::string address;
  uint16_t port;
  std::vector<Peer> peers;
};

} // namespace dsm