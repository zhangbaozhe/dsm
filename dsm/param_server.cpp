/*
 * @Brief: In the parameter server, for each parameter we only allow 
 *         read and write operations one by one.
 * @Author: Baozhe ZHANG 
 * @Date: 2024-04-18 13:32:24 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-05-05 16:11:34
 */

#include "utils/safe_map.h"

#include <atomic>
#include <string>

#include "spdlog/spdlog.h"
#include "httplib.h"


int main(int argc, char *argv[]) {
  dsm::utils::SafeMap<std::string, std::atomic_int64_t> PARAMS;
  dsm::utils::SafeMap<std::string, std::atomic_int64_t> SYNC_FLAGS;

  std::string address = argv[1];
  int port = std::stoi(argv[2]);

  auto server = httplib::Server();
  server.new_task_queue = [] { return new httplib::ThreadPool(1); };

  server.Post("/health", [](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.status = 200;
    res.set_content("OK", "text/plain");
  });

  server.Post("/stop", [&server](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    server.stop();
  });

  server.Post("/param/registration", [&PARAMS, &SYNC_FLAGS](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    auto name = req.get_param_value("name");
    if (PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name already exists", "text/plain");
      return;
    }

    PARAMS[name] = 0;
    SYNC_FLAGS[name] = 0;
    spdlog::info("Registered parameter {}", name);
    res.status = 200;
    res.set_content("Registered", "text/plain");
  });

  server.Post("/param/deletion", [&PARAMS, &SYNC_FLAGS](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    auto name = req.get_param_value("name");
    if (!PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name does not exist", "text/plain");
      return;
    }

    PARAMS.erase(name);
    SYNC_FLAGS.erase(name);
    spdlog::info("Deleted parameter {}", name);
    res.status = 200;
    res.set_content("Deleted", "text/plain");
  });


  server.Post("/param/write", [&PARAMS, &SYNC_FLAGS](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    int id = std::stoi(req.get_param_value("id"));
    auto name = req.get_param_value("name");
    auto value = std::stoll(req.get_param_value("value"));

    if (!PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name does not exist", "text/plain");
      return;
    }

    // std::cout << "Write SYNC_FLAGS[name]: " << SYNC_FLAGS[name] << std::endl;
    if (SYNC_FLAGS[name] != (1 << id)) {
      res.status = 400;
      res.set_content("Request busy", "text/plain");
      return;
    }

    PARAMS[name] = value;
    spdlog::info("ID {} wrote {} to parameter {}", id, value, name);
    res.status = 200;
    res.set_content("Wrote", "text/plain");
    SYNC_FLAGS[name] = 0;
  });

  server.Post("/param/read", [&PARAMS, &SYNC_FLAGS](const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    int id = std::stoi(req.get_param_value("id"));
    auto name = req.get_param_value("name");


    if (!PARAMS.find(name)) {
      res.status = 400;
      res.set_content("Name does not exist", "text/plain");
      return;
    }

    // std::cout << "Read SYNC_FLAGS[name]: " << SYNC_FLAGS[name] << std::endl;
    if (SYNC_FLAGS[name] != 0) {
      res.status = 400;
      res.set_content("Request busy", "text/plain");
      return;
    }

    int64_t value = PARAMS[name];
    spdlog::info("ID {} read {} from parameter {}", id, value, name);
    res.status = 200;
    res.set_content(std::to_string(value), "text/plain");
    SYNC_FLAGS[name] = (1 << id);
  });

  server.listen(address, port);
}