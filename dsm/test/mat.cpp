#include "dsm/manager.h"
#include <algorithm>


int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);


  size_t M = std::stoi(argv[2]);
  size_t N = std::stoi(argv[3]);
  size_t P = std::stoi(argv[4]);


  manager.create_mutex("mat");

  manager.mutex_lock("mat");
  auto objects1 = dsm::generate_objects<dsm::Float>(manager, M * N, "mat1");
  auto mat1 = dsm::Matrix<dsm::Float>(objects1, M, N);
  auto objects2 = dsm::generate_objects<dsm::Float>(manager, N * P, "mat2");
  auto mat2 = dsm::Matrix<dsm::Float>(objects2, N, P);
  auto objects3 = dsm::generate_objects<dsm::Float>(manager, M * P, "mat3");
  auto mat3 = dsm::Matrix<dsm::Float>(objects3, M, P);
  manager.mutex_unlock("mat");

  auto my_idx = manager.get_id() - 1; // id starts from 1
  auto node_num = manager.get_peer_num() + 1;


  // othre node wait for the first node to finish
  if (my_idx != 0) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  } 

  auto fill = [&](dsm::Matrix<dsm::Float> &mat, float val) {
    for (size_t i = 0; i < mat.M; i++) {
      for (size_t j = 0; j < mat.N; j++) {
        mat(i, j) = val;
      }
    }
  };

  if (my_idx == 0) {
    manager.mutex_lock("mat");
    // fill mat1 and mat2 in threads
    std::thread t1(fill, std::ref(mat1), 1.0);
    std::thread t2(fill, std::ref(mat2), 2.0);
    t1.join();
    t2.join();
    manager.mutex_unlock("mat");
  } else {
    manager.mutex_lock("mat");
    manager.mutex_unlock("mat");
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));

  size_t total_num = M * P;
  size_t num_per_node = M * P / node_num;

  // indexing 
  // [ my_idx * num_per_node : (my_idx + 1) * num_per_node )
  // part of [ 0 : M * P ]
  // i = linear_idx / P
  // j = linear_idx % p
  size_t linear_idx_min = 0;
  size_t linear_idx_max = 0;
  if (my_idx != node_num - 1) {
    linear_idx_min = my_idx * num_per_node;
    linear_idx_max = (my_idx + 1) * num_per_node;
  } else {
    linear_idx_min = my_idx * num_per_node;
    linear_idx_max = total_num;
  }
  std::vector<std::pair<size_t, size_t>> range;
  for (size_t k = linear_idx_min; k < linear_idx_max; k++) {
    range.emplace_back(
      k / P, k % P
    );
  }

  // computing
  auto start_time = std::chrono::high_resolution_clock::now();
  for (auto pair : range) {
    auto i = pair.first;
    auto j = pair.second;
    for (auto k = 0; k < N; k++) {
      mat3(i, j) += mat1(i, k) * mat2(k, j);
    }
  }
  auto end_time = std::chrono::high_resolution_clock::now();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::cout << mat1 << std::endl;
  std::cout << mat2 << std::endl;
  std::cout << mat3 << std::endl;
  std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms" << std::endl;
  
}