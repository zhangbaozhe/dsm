/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 13:26:09 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-29 13:36:25
 */

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

namespace dsm {

class Peer {
 public: 
  Peer() : m_internet_address(0), m_port(0) {}

  Peer(const std::string &ip_address, uint16_t port) {
    m_internet_address = parse_internet_address(ip_address);
    m_port = port;
  }

  Peer(uint64_t internet_address, uint16_t port) : 
    m_internet_address(internet_address), 
    m_port(port) {}

  const std::string get_string() const;

  const struct sockaddr_in get_socket() const;

  const uint16_t get_port() const;

  const uint64_t get_canonical_id() const;

  bool operator==(const Peer &rhs) const;

  bool operator!=(const Peer &rhs) const;

 private: 
  const std::string unparse_internet_address(uint64_t ip) const;
  const uint64_t parse_internet_address(const std::string &ip) const;

  uint64_t m_internet_address;
  uint16_t m_port;
}; // class Peer

} // namespace dsm