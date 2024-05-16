/*
 * @Brief: Peer configureation
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 13:44:29 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-18 14:21:10
 */

#pragma once

#include "dsm/peer.h"
#include "type.h"

#include <string>
#include <vector>

namespace dsm {


struct Config {

  Config() = default;

  Config(const std::string &address, uint16_t port, const std::vector<Peer> &peers, const Peer &param_server) : 
    address(address), 
    port(port), 
    peers(peers) , 
    param_server(param_server) {}

  Config(Json config_json) {
    address = config_json["address"].get<std::string>();
    port = config_json["port"].get<int>();
    id = config_json["id"].get<int>();
    for (const auto &peer : config_json["peers"].get<std::vector<Json>>()) {
      peers.push_back(
        Peer{peer["address"].get<std::string>(), peer["port"].get<int>(), peer["id"].get<int>()}
      );
    }
    param_server = Peer{config_json["param_server"]["address"].get<std::string>(), config_json["param_server"]["port"].get<int>()};
  }
  
  std::string address;
  int port;
  int id;
  std::vector<Peer> peers;

  Peer param_server;
};

} // namespace dsm