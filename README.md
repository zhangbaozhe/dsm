# dsm - a simple implementation of distributed shared memory system

## basic design

- Full replication: Each node has a copy of the shared virtual memory
- Write-update: Each write on each node will be notified to other nodes and then update the VM

## build

Run the following commands
```bash
cmake -B build 
cmake --build build -j10
```

If you have Docker installed in your OS, you can also run the provided script
```bash
bash build_docker_dsm.sh
```
This will build this project and build the Docker image.

## test

### integer addition (without synchronization)

### integer addition (with synchronization)
