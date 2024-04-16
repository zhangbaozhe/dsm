/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 17:37:49 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-01 22:03:37
 */

#include "dsm/manager.h"
#include "dsm/utils/stringutils.h"

#include <spdlog/spdlog.h>
#include <base64.hpp>

namespace dsm {

Manager::Manager(const char *config_path) :
  m_database(),
  m_config(Json::parse(utils::read_file(config_path))) {
  // TODO: client connection failure handling
  for (const auto &peer : m_config.peers) {
    m_clients.emplace_back(
        std::move(std::make_unique<httplib::Client>(peer.address, peer.port))
    );
  }


  m_server = std::make_unique<httplib::Server>();
  m_server->new_task_queue = [] { return new httplib::ThreadPool(1); };
  

  // like malloc
  m_server->Post("/mem/registration", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");
        size_t size = std::stoul(req.get_param_value("size"));

        if (m_database.find(name) != m_database.end()) {
          spdlog::error("Request from {}:{} mem `{}` already exists at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Registration failed\n", "text/plain");
          return;
        }

        m_database[name] = new Object(name, this);
        m_database[name]->m_data.resize(size);
        std::memset(m_database[name]->m_data.data(), 0, size);

        spdlog::info("Request from {}:{} mem `{}` with size of `{}` registered OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, size,
            m_config.address, 
            m_config.port);

        res.status = 200;
        res.set_content("Registration OK\n", "text/plain");

      }
  );
  m_server->Post("/mem/deletion", 
      [this](const httplib::Request &req, httplib::Response &res) {

        auto name = req.get_param_value("name");
        auto size = m_database[name]->m_data.size();

        delete m_database[name];
        m_database.erase(name);

        spdlog::info("Request from {}:{} mem `{}` with size of `{}` deleted OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, size, 
            m_config.address, 
            m_config.port);

        res.status = 200;
        res.set_content("Deletion OK\n", "text/plain");

      }
  );
  m_server->Post("/stop", 
      [this](const httplib::Request &req, httplib::Response &res) {
        spdlog::info("Request from {}:{} server stopped at {}:{}", 
            req.remote_addr, req.remote_port,
            m_config.address, m_config.port);
        m_server->stop();
      }
  );

  m_server->Get("/mem/show", 
      [this](const httplib::Request &req, httplib::Response &res) {
        Json json;
        for (const auto &it : m_database) {
          json.push_back({{"name", it.first}, {"size", it.second->m_data.size()}});
        }
        res.status = 200;
        res.set_content(json.dump(), "text/plain");
      }
  );

  m_server->Post("/mem/write", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");      
        auto offset = std::stoul(req.get_param_value("offset"));
        auto length = std::stoul(req.get_param_value("length"));

        if (m_database.find(name) == m_database.end()) {
          spdlog::error("Request from {}:{} mem `{}` not found at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Write failed\n", "text/plain");
          return;
        }

        auto current_size = m_database[name]->m_data.size();
        if (offset + length > current_size) {
          // allocate more space
          m_database[name]->m_data.resize(offset + length);
        }

        auto data = base64::from_base64(req.get_param_value("data"));
        std::copy(data.begin(), data.end(), m_database[name]->m_data.begin() + offset);

        spdlog::info("Request from {}:{} mem `{}` write OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);
        res.status = 200;
        res.set_content("Write OK\n", "text/plain");
      }
  );

  m_server->Post("/mem/read", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");      
        auto offset = std::stoul(req.get_param_value("offset"));
        auto length = std::stoul(req.get_param_value("length"));

        if (m_database.find(name) == m_database.end()) {
          spdlog::error("Request from {}:{} mem `{}` not found at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Read failed\n", "text/plain");
          return;
        }

        auto current_size = m_database[name]->m_data.size();
        if (offset + length > current_size) {
          spdlog::error("Request from {}:{} mem `{}` out of range at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Read failed\n", "text/plain");
          return;
        }


        std::string data(m_database[name]->m_data.begin() + offset, m_database[name]->m_data.begin() + offset + length);
        res.status = 200;
        res.set_content(base64::to_base64(data), "text/plain");


        spdlog::info("Request from {}:{} mem `{}` read OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);
      }
  );

  m_listen_thread = std::thread([this]{
    // TODO: is listen thread safe?
    m_server->listen(m_config.address, m_config.port);
  });
  m_server->wait_until_ready();
  for (auto &client : m_clients) {
    auto res = client->Post("/mem/show");
    while (res.error() != httplib::Error::Success) {
      res = client->Post("/mem/show");
    }
  }

}

Manager::~Manager() {
  m_listen_thread.join();
  for (auto &object : m_database) {
    delete object.second;
  }
}



Object *Manager::mmap(const std::string &name, size_t length) {
  if (m_database.find(name) != m_database.end()) {
    return m_database[name];
  }
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

gsl::span<Byte> Manager::read(const std::string &name, size_t offset, size_t length) {
  if (m_database.find(name) == m_database.end()) {
    throw std::runtime_error("Object not found");
  }
  if (offset + length > m_database[name]->m_data.size()) {
    throw std::runtime_error("Out of range");
  }
  httplib::Params param;
  param.emplace("name", name);
  param.emplace("offset", std::to_string(offset));
  param.emplace("length", std::to_string(length));
  for (auto &client : m_clients) {
    auto res = client->Post("/mem/read", param);
    // std::cout << res->status << std::endl;
    // std::cout << (int)base64::from_base64(res->body)[0] << std::endl;
  }
  // TODO: consensus
  return gsl::span<Byte>(m_database[name]->m_data.data() + offset, length);
}

void Manager::write(const std::string &name, size_t offset, gsl::span<Byte> data) {
  if (m_database.find(name) == m_database.end()) {
    throw std::runtime_error("Object not found");
  }
  // if (offset + data.size() > m_database[name]->m_data.size()) {
  //   throw std::runtime_error("Out of range");
  // }
  httplib::Params param;
  param.emplace("name", name);
  param.emplace("offset", std::to_string(offset));
  param.emplace("length", std::to_string(data.size()));
  std::string_view data_view(data.data(), data.size());
  param.emplace("data", base64::to_base64(data_view));
  for (auto &client : m_clients) {
    auto res = client->Post("/mem/write", param);
    // std::cout << res->status << std::endl;
    // std::cout << res->body << std::endl;
  }
  std::copy(data.begin(), data.end(), m_database[name]->m_data.begin() + offset);
}

std::vector<Object *> Manager::find_objects(const std::string &name_prefix) {
  std::vector<std::pair<size_t, Object *>> kvs;
  for (const auto &it : m_database) {
    if (it.first.find(name_prefix) == 0) {
      std::vector<std::string> tmp;
      utils::split(it.first, '_', tmp);
      size_t index = std::stoul(tmp[tmp.size() - 1]);
      kvs.push_back({index, it.second});
    }
  }
  // sort by index
  std::sort(kvs.begin(), kvs.end(), [](const auto &a, const auto &b) {
    return a.first < b.first;
  });
  // return as vector
  std::vector<Object *> objects;
  for (const auto &kv : kvs) {
    objects.push_back(kv.second);
  }
  return objects;
}


} // namespace dsm