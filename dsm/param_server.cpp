/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-04-18 13:32:24 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-18 14:57:10
 */

#include "utils/safe_map.h"

#include <atomic>
#include <string>

#include "spdlog/spdlog.h"
#include "httplib.h"


int main(int argc, char *argv[]) {
  dsm::utils::SafeMap<std::string, std::atomic_int64_t> PARAMS;

  std::string address = argv[1];
  int port = std::stoi(argv[2]);

  auto server = httplib::Server();
  server.new_task_queue = [] { return new httplib::ThreadPool(1); };

  server.Post("/health", [](const httplib::Request &req, httplib::Response &res) {
    res.status = 200;
    res.set_content("OK", "text/plain");
  });

  server.Post("/stop", [&server](const httplib::Request &req, httplib::Response &res) {
    server.stop();
  });

  server.Post("/param/registration", [&PARAMS](const httplib::Request &req, httplib::Response &res) {
    auto name = req.get_param_value("name");
    if (PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name already exists", "text/plain");
      return;
    }

    PARAMS[name] = 0;
    spdlog::info("Registered parameter {}", name);
    res.status = 200;
    res.set_content("Registered", "text/plain");
  });

  server.Post("/param/deletion", [&PARAMS](const httplib::Request &req, httplib::Response &res) {
    auto name = req.get_param_value("name");
    if (!PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name does not exist", "text/plain");
      return;
    }

    PARAMS.erase(name);
    spdlog::info("Deleted parameter {}", name);
    res.status = 200;
    res.set_content("Deleted", "text/plain");
  });


  server.Post("/param/write", [&PARAMS](const httplib::Request &req, httplib::Response &res) {
    auto name = req.get_param_value("name");
    auto value = std::stoll(req.get_param_value("value"));

    if (!PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name does not exist", "text/plain");
      return;
    }

    PARAMS[name] = value;
    spdlog::info("Wrote {} to parameter {}", value, name);
    res.status = 200;
    res.set_content("Wrote", "text/plain");
  });

  server.Post("/param/read", [&PARAMS](const httplib::Request &req, httplib::Response &res) {
    auto name = req.get_param_value("name");
    if (!PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name does not exist", "text/plain");
      return;
    }

    int64_t value = PARAMS[name];
    spdlog::info("Read {} from parameter {}", value, name);
    res.status = 200;
    res.set_content(std::to_string(value), "text/plain");
  });

  server.listen(address, port);
}