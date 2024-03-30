/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 13:44:29 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 21:30:56
 */

#pragma once

#include "dsm/peer.h"
#include "type.h"

#include <string>
#include <vector>

namespace dsm {


struct Config {

  Config() = default;

  Config(const std::string &address, uint16_t port, const std::vector<Peer> &peers) : 
    address(address), 
    port(port), 
    peers(peers) {}

  Config(Json config_json) {
    address = config_json["address"].get<std::string>();
    port = config_json["port"].get<int>();
    for (const auto &peer : config_json["peers"].get<std::vector<Json>>()) {
      peers.push_back(
        Peer{peer["address"].get<std::string>(), peer["port"].get<int>()}
      );
    }
  }
  
  std::string address;
  int port;
  std::vector<Peer> peers;
};

} // namespace dsm