/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 17:37:49 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-17 18:52:22
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

  m_server->Post("/mutex/registration", 
      [this](const httplib::Request &req, httplib::Response &res) {


        auto name = req.get_param_value("name");
        if (m_mutexes.find(name)) {
          spdlog::warn("Request from {}:{} mutex `{}` already exists at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Registration failed\n", "text/plain");
          return;
        }

        m_mutexes[name] = 0;

        spdlog::info("Request from {}:{} mutex `{}` registered OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);

        res.status = 200;
        res.set_content("Registration OK\n", "text/plain");

      }
  );

  m_server->Post("/mutex/deletion", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");
        if (!m_mutexes.find(name)) {
          spdlog::error("Request from {}:{} mutex `{}` not found at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Deletion failed\n", "text/plain");
          return;
        }

        m_mutexes.erase(name);

        spdlog::info("Request from {}:{} mutex `{}` deleted OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);

        res.status = 200;
        res.set_content("Deletion OK\n", "text/plain");

      }
  );
  m_server->Post("/mutex/lock", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");
        if (!m_mutexes.find(name)) {
          spdlog::error("Request from {}:{} mutex `{}` not found at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Lock failed\n", "text/plain");
          return;
        }

        if (m_mutexes[name] == 1) {
          spdlog::warn("Request from {}:{} mutex `{}` already locked at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Already locked\n", "text/plain");
          return;
        }
        m_mutexes[name] = 1;

        spdlog::info("Request from {}:{} mutex `{}` locked OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);

        res.status = 200;
        res.set_content("Lock OK\n", "text/plain");

      }
  );
  m_server->Post("/mutex/unlock", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");
        if (!m_mutexes.find(name)) {
          spdlog::error("Request from {}:{} mutex `{}` not found at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Unlock failed\n", "text/plain");
          return;
        }

        if (m_mutexes[name] == 0) {
          spdlog::warn("Request from {}:{} mutex `{}` already unlocked at {}:{}", 
              req.remote_addr, req.remote_port, 
              name, 
              m_config.address, 
              m_config.port);
          res.status = 400;
          res.set_content("Already unlocked\n", "text/plain");
          return;
        }
        m_mutexes[name] = 0;

        spdlog::info("Request from {}:{} mutex `{}` unlocked OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);

        res.status = 200;
        res.set_content("Unlock OK\n", "text/plain");

      }
  );
  

  // like malloc
  m_server->Post("/mem/registration", 
      [this](const httplib::Request &req, httplib::Response &res) {
        auto name = req.get_param_value("name");
        size_t size = std::stoul(req.get_param_value("size"));

        if (m_database.find(name)) {
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
        for (const auto &it : m_database.get_map()) {
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

        if (!m_database.find(name)) {
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

        if (!m_database.find(name)) {
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
  for (auto &object : m_database.get_map()) {
    delete object.second;
  }
}


void Manager::create_mutex(const std::string &name) {
  if (m_mutexes.find(name)) {
    return;
  }
  m_mutexes[name] = 0;
  httplib::Params param;
  param.emplace("name", std::string(name));
  for (auto &client : m_clients) {
    auto res = client->Post("/mutex/registration", param);
  }
}

void Manager::delete_mutex(const std::string &name) {
  if (!m_mutexes.find(name)) {
    throw std::runtime_error("Mutex not found");
  }
  m_mutexes.erase(name);
  httplib::Params param;
  param.emplace("name", std::string(name));
  for (auto &client : m_clients) {
    auto res = client->Post("/mutex/deletion", param);
  }
}

void Manager::mutex_lock(const std::string &name) {
  if (!m_mutexes.find(name)) {
    throw std::runtime_error("Mutex not found");
  }

  // if the current mutex is locked, i.e., 
  // in this process, the other process is using this mutex
  // so this process needs to wait it for unlocking 
  while (m_mutexes[name] == 1) {
    spdlog::warn("In `{}` mutex `{}` is locked, waiting ...", __func__, name);
    // FIXME: mutex -- sleep time ?
    // consistency?
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  httplib::Params param;
  param.emplace("name", std::string(name));
  for (auto &client : m_clients) {
    auto res = client->Post("/mutex/lock", param);
  }
}

bool Manager::mutex_try_lock(const std::string &name) {
  if (!m_mutexes.find(name)) {
    throw std::runtime_error("Mutex not found");
  }

  // the mutex is currently locked
  // return false
  if (m_mutexes[name] == 1) {
    return false;
  }

  mutex_lock(name);
  return true;
}

void Manager::mutex_unlock(const std::string &name) {
  if (!m_mutexes.find(name)) {
    throw std::runtime_error("Mutex not found");
  }

  httplib::Params param;
  param.emplace("name", std::string(name));
  for (auto &client : m_clients) {
    auto res = client->Post("/mutex/unlock", param);
  }
}





Object *Manager::mmap(const std::string &name, size_t length) {
  if (m_database.find(name)) {
    return m_database[name];
  }
  m_database[name] = new Object(name, this);
  m_database[name]->m_data.resize(length);
  httplib::Params param;
  param.emplace("name", std::string(name));
  param.emplace("size", std::to_string(length));
  for (auto &client : m_clients) {
    auto res = client->Post("/mem/registration", param);
    // std::cout << res->status << std::endl;
    // std::cout << res->body << std::endl;
  }
  return m_database[name];
}

void Manager::munmap(const std::string &name) {
  if (!m_database.find(name)) {
    throw std::runtime_error("Object not found");
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
}

gsl::span<Byte> Manager::read(const std::string &name, size_t offset, size_t length) {
  if (!m_database.find(name)) {
    throw std::runtime_error("Object not found");
  }
  if (offset + length > m_database[name]->m_data.size()) {
    throw std::runtime_error("Out of range");
  }

  // // wait for the mutex
  // bool is_ok = false;
  // while (!is_ok) {
  //   // check if any mutex is locked
  //   for (const auto &mutex : m_mutexes.get_map()) {
  //     if (mutex.second == 1) {
  //       is_ok = false;
  //       // sleep for a while
  //       spdlog::warn("In `{}` mutex `{}` is locked, waiting ...", __func__, name);
  //       std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //       break;
  //     }
  //     is_ok = true;
  //   }
  // }


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
  if (!m_database.find(name)) {
    throw std::runtime_error("Object not found");
  }
  // if (offset + data.size() > m_database[name]->m_data.size()) {
  //   throw std::runtime_error("Out of range");
  // }

  // // wait for the mutex
  // bool is_ok = false;
  // while (!is_ok) {
  //   // check if any mutex is locked
  //   for (const auto &mutex : m_mutexes.get_map()) {
  //     if (mutex.second == 1) {
  //       is_ok = false;
  //       // sleep for a while
  //       spdlog::warn("In `{}` mutex `{}` is locked, waiting ...", __func__, name);
  //       std::this_thread::sleep_for(std::chrono::milliseconds(10));
  //       break;
  //     }
  //     is_ok = true;
  //   }
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
  for (const auto &it : m_database.get_map()) {
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