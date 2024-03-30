/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 17:37:49 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 23:10:27
 */

#include "dsm/manager.h"
#include "dsm/utils/stringutils.h"

namespace dsm {

Manager::Manager(const char *config_path) :
  m_database(),
  m_config(Json::parse(utils::read_file(config_path))) {
  for (const auto &peer : m_config.peers) {
    m_clients.emplace_back(
        std::move(std::make_unique<httplib::Client>(peer.address, peer.port))
    );
  }


  m_server = std::make_unique<httplib::Server>();
  m_server->new_task_queue = [] { return new httplib::ThreadPool(1); };
  

  m_server->Post("/mem/registration", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");
        size_t size = std::stoul(req.get_param_value("size"));
        std::cout << name << std::endl;

        m_database[name] = new Object(name, this);
        m_database[name]->m_data.resize(size);

        std::cout << "After registration:";
        for (const auto &it : m_database) {
          std::cout << it.first << std::endl;
        }

        res.status = 200;
        res.set_content("Registration OK", "text/plain");

      }
  );
  m_server->Post("/mem/deletion", 
      [this](const httplib::Request &req, httplib::Response &res) {

        auto name = req.get_param_value("name");

        delete m_database[name];
        m_database.erase(name);

        std::cout << "After deletion";
        for (const auto &it : m_database) {
          std::cout << it.first << std::endl;
        }

        res.status = 200;
        res.set_content("Deletion OK", "text/plain");

      }
  );
  m_server->Post("/stop", 
      [this](const httplib::Request &req, httplib::Response &res) {
        m_server->stop();
      }
  );

  m_listen_thread = std::thread([this]{
    m_server->listen("localhost", m_config.port);
  });
  m_server->wait_until_ready();

}

Manager::~Manager() {
  for (auto &object : m_database) {
    delete object.second;
  }
  m_listen_thread.join();
}

int Manager::open(const std::string &name, OpenFlag flag) {
  if (m_database.find(name) != m_database.end()) {
    return -1;
  }
  return 0;
}


Object *Manager::mmap(const std::string &name, size_t length, MapProt prot, MapFlag flags) {
  m_database[name] = new Object(name, this);
  m_database[name]->m_data.resize(length);
  httplib::Params param;
  param.emplace("name", std::string(name));
  param.emplace("size", std::to_string(length));
  for (auto &client : m_clients) {
    auto res = client->Post("/mem/registration", param);
    std::cout << res->status << std::endl;
    std::cout << res->body << std::endl;
  }
  return m_database[name];
}

int Manager::munmap(const std::string &name) {
  if (m_database.find(name) == m_database.end()) {
    return -1;
  }
  delete m_database[name];
  m_database.erase(name);
  
  httplib::Params param;
  param.emplace("name", std::string(name));

  for (auto &client : m_clients) {
    auto res = client->Post("/mem/deletion", param);
    std::cout << res->status << std::endl;
    std::cout << res->body << std::endl;
  }
  return 0;
}

} // namespace dsm