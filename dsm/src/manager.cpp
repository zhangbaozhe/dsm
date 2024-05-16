/*
 * @Brief: Manager implementation
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 17:37:49 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-05-05 18:10:37
 */

#include "dsm/manager.h"
#include "dsm/utils/stringutils.h"


#include <spdlog/spdlog.h>
#include <base64.hpp>


namespace dsm {

// read and write must be called in sequence
int64_t read_from_param_server(int id, const std::string &name, httplib::Client *client) {
  httplib::Params param;
  param.emplace("id", std::to_string(id));
  param.emplace("name", name);
  auto res = client->Post("/param/read", param);
  while (res->status != 200) {
    res = client->Post("/param/read", param);
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return std::stoll(res->body);
}

void write_to_param_server(int id, const std::string &name, int64_t value, httplib::Client *client) {
  httplib::Params param;
  param.emplace("id", std::to_string(id));
  param.emplace("name", name);
  param.emplace("value", std::to_string(value));
  auto res = client->Post("/param/write", param);
  while (res->status != 200) {
    res = client->Post("/param/write", param);
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

Manager::Manager(const char *config_path) :
  m_database(),
  m_config(Json::parse(utils::read_file(config_path))) {
  
  m_pattern = 0;
  for (const auto &peer : m_config.peers) {
    m_clients.emplace_back(
        std::move(std::make_unique<httplib::Client>(peer.address, peer.port))
    );
    m_pattern = m_pattern | (0 << peer.id);
  }
  m_pattern = m_pattern | (1 << m_config.id);
  m_param_server = std::make_unique<httplib::Client>(m_config.param_server.address, m_config.param_server.port);


  m_server = std::make_unique<httplib::Server>();
  m_server->new_task_queue = [] { return new httplib::ThreadPool(2); };

  m_server->Post("/health", [](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.status = 200;
    res.set_content("OK", "text/plain");
  });


  // like malloc
  m_server->Post("/mem/registration", 
      [this](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
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

#ifdef DEBUG_LOG
        spdlog::info("Request from {}:{} mem `{}` with size of `{}` registered OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, size,
            m_config.address, 
            m_config.port);
#endif // DEBUG_LOG

        res.status = 200;
        res.set_content("Registration OK\n", "text/plain");

      }
  );
  m_server->Post("/mem/deletion", 
      [this](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto name = req.get_param_value("name");
        auto size = m_database[name]->m_data.size();

        delete m_database[name];
        m_database.erase(name);

#ifdef DEBUG_LOG
        spdlog::info("Request from {}:{} mem `{}` with size of `{}` deleted OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, size, 
            m_config.address, 
            m_config.port);
#endif // DEBUG_LOG

        res.status = 200;
        res.set_content("Deletion OK\n", "text/plain");

      }
  );

  m_server->Post("/stop", 
      [this](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        spdlog::info("Request from {}:{} server stopped at {}:{}", 
            req.remote_addr, req.remote_port,
            m_config.address, m_config.port);
        m_server->stop();
      }
  );

  m_server->Get("/mem/show", 
      [this](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
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
        res.set_header("Access-Control-Allow-Origin", "*");
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
#ifdef DEBUG_LOG
        spdlog::info("Request from {}:{} mem `{}` write OK at {}:{}", 
            req.remote_addr, req.remote_port, 
            name, 
            m_config.address, 
            m_config.port);
#endif // DEBUG_LOG
        res.status = 200;
        res.set_content("Write OK\n", "text/plain");
      }
  );

  m_server->Post("/mem/read", 
      [this](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
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
        // res.set_content(base64::to_base64(data) + "," + std::to_string(m_database[name]->m_update_count), "text/plain");
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
    auto res = client->Post("/health");
    while (res.error() != httplib::Error::Success) {
      res = client->Post("/health");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "Waiting client ..." << std::endl;
    }
  }
  while (m_param_server->Post("/health").error() != httplib::Error::Success) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Waiting server ..." << std::endl;
  }

}

Manager::~Manager() {
  m_listen_thread.join();
  for (auto &object : m_database.get_map()) {
    delete object.second;
  }
}


void Manager::create_mutex(const std::string &name) {
  
  httplib::Params param;
  param.emplace("name", std::string(name));

  auto res = m_param_server->Post("/param/registration", param);
}

void Manager::delete_mutex(const std::string &name) {
  
  httplib::Params param;
  param.emplace("name", std::string(name));

  auto res = m_param_server->Post("/param/deletion", param);
}

void Manager::mutex_lock(const std::string &name) {


  std::vector<httplib::Client *> unlocked_clients;

  httplib::Params param;
  param.emplace("name", std::string(name));

  // TODO: avoid lambda
  auto check_avilable = [name, this] {
    auto mutex_param = read_from_param_server(m_config.id, name, m_param_server.get());
    write_to_param_server(m_config.id, name, mutex_param, m_param_server.get());
    return mutex_param == 0;
  };

  std::function<void()> lock;
  lock = [name, this, &lock] {
    auto temp = read_from_param_server(m_config.id, name, m_param_server.get());
    if (temp == 0) {
      write_to_param_server(m_config.id, name, m_pattern, m_param_server.get());
      return;
    } else {
      write_to_param_server(m_config.id, name, temp, m_param_server.get());
      lock();
    }
  };

  if (check_avilable()) {
    // lock
    lock();
    // spdlog::warn("lock");
    return;
  } else {
    while (true) {
      // TODO: this is too silly
      // std::this_thread::sleep_for(std::chrono::milliseconds(1));
      if (check_avilable()) {
        lock();
        // spdlog::warn("lock");
        return;
      }
    }
  }


}


void Manager::mutex_unlock(const std::string &name) {
  

  // read
  auto mutex_param = read_from_param_server(m_config.id, name, m_param_server.get());
  write_to_param_server(m_config.id, name, mutex_param, m_param_server.get());

  // spdlog::warn("unlock {} == {}", mutex_param, m_pattern);
  assert(mutex_param == m_pattern);

  httplib::Params param;
  param.emplace("name", std::string(name));

  // running -> 1
  int temp = 0;
  // temp = temp | (1 << m_config.id);
  read_from_param_server(m_config.id, name, m_param_server.get());
  write_to_param_server(m_config.id, name, temp, m_param_server.get());
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
  }
}

gsl::span<Byte> Manager::read(const std::string &name, size_t offset, size_t length) {
  if (!m_database.find(name)) {
    throw std::runtime_error("Object not found");
  }
  if (offset + length > m_database[name]->m_data.size()) {
    throw std::runtime_error("Out of range");
  }

  return gsl::span<Byte>(m_database[name]->m_data.data() + offset, length);

}

void Manager::write(const std::string &name, size_t offset, gsl::span<Byte> data) {
  if (!m_database.find(name)) {
    throw std::runtime_error("Object not found");
  }
  
  std::copy(data.begin(), data.end(), m_database[name]->m_data.begin() + offset);
  // m_database[name]->m_update_count++;

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