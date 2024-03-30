/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-28 14:59:26 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 18:52:05
 */

#include "dsm/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

void *create_shared_memeory(size_t size) {
  int protection = PROT_READ | PROT_WRITE;
  int visibility = MAP_SHARED;
  return mmap(NULL, size, protection, visibility, -1, 0);
}

int main(int argc, char *argv[]) {
  // dsm::init(argc, argv);
  return 0;
}