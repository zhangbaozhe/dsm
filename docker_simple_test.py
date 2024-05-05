#!/usr/bin/python
"""
Start a series of docker containers to test the functionality of the DSM
"""
import subprocess
import os, time
import sys
import json
import ipaddress

INIT_PORT = 9090
INIT_SUBNET = "192.0.0.0/24"
INTT_GATEWAY = "192.0.0.1"
INIT_IP = ipaddress.ip_address("192.0.0.2")
NETWORK_NAME = "dsm_network"
TEST_TYPES = [
    "int32", 
    "float", 
    "mutex", 
    "mat"
]


def start_docker_param_server(server_port, network_name, ip):
    return subprocess.Popen(
        [
            "docker", 
            "run", "--rm", "-d",
            "--network", network_name,
            "--ip", ip,
            "--name", "param_server",
            "dsm:latest", 
            "/dsm/dsm/param_server", 
            ip, 
            str(server_port), 
            "> /log.txt"
        ])

def start_docker_node(raw_config_json, network_name, ip, id, test_type):
    # first create the config file
    # then start dsm
    # TODO: use test type
    print("Strating dsm_node_{} ...".format(id))
    return subprocess.Popen(
        [
            "docker", 
            "run", "--rm", "-d", 
            "--network", network_name,
            "--ip", ip,
            "--name", "dsm_node_{}".format(id),
            "dsm:latest", 
            "bash", "-c", "echo '{}' > /config.json && /dsm/dsm/test/{} /config.json > /log.txt".format(raw_config_json, test_type)
        ], 
    )

def start_docker_mat_node(raw_config_json, network_name, ip, id, args):
    # first create the config file
    # then start dsm
    # TODO: use test type
    print("Strating dsm_node_{} ...".format(id))
    return subprocess.Popen(
        [
            "docker", 
            "run", "--rm", "-d", 
            "--network", network_name,
            "--ip", ip,
            "--name", "dsm_node_{}".format(id),
            "dsm:latest", 
            "bash", "-c", "echo '{}' > /config.json && /dsm/dsm/test/mat /config.json {} {} {} > /log.txt".format(raw_config_json, args[0], args[1], args[2])
        ], 
    )

def start_docker_web_node(ip):
    print("Starting dsm_web_node ...")
    return subprocess.Popen(
        [
            "docker", 
            "run", "--rm", "-d", 
            "-p", "{}:{}".format(6006, 6006), 
            "--network", NETWORK_NAME,
            "--ip", ip, 
            "--name", "dsm_web_node",
            "dsm:latest", 
            "python3", "-m", "http.server", "-d", "/web", str(6006)
        ]
    )


def check_docker_has_network(name):
    return subprocess.run(
        ["docker", "network", "inspect", name],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    ).returncode == 0


def create_docker_network(name):
    if not check_docker_has_network(name):
        return subprocess.run(
            [
                "docker", 
                "network", 
                "create", 
                "--subnet={}".format(INIT_SUBNET),
                "--gateway={}".format(INTT_GATEWAY),
                name, 
            ]
        )


def generate_dsm_node_ip_id_port_map(num_nodes):
    pool = {} # ip -> {port: port, id: id}
    for i in range(num_nodes):
        pool[INIT_IP + i + 1] = {
            "id": i + 1,
            "port": INIT_PORT 
        }

    # print nodes.json for web client to use
    nodes_json = {}
    nodes_json["nodes"] = []
    for ip in pool:
        nodes_json["nodes"].append({
            "id": pool[ip]["id"],
            "address": str(ip),
            "port": pool[ip]["port"]
        })
    nodes_json["param_server"] = {
        "address": str(INIT_IP),
        "port": INIT_PORT
    }
    print(json.dumps(nodes_json))
    return pool

def generate_dsm_node_config(ego_ip: ipaddress.IPv4Address, pool):
    param_server = {}
    param_server["address"] = str(INIT_IP)
    param_server["port"] = INIT_PORT
    config = {}
    config["param_server"] = param_server
    config["id"] = pool[ego_ip]["id"]
    config["address"] = str(ego_ip)
    config["port"] = pool[ego_ip]["port"]  
    config["peers"] = []
    for ip in pool:
        if ip != ego_ip:
            config["peers"].append({
                "id": pool[ip]["id"],
                "address": str(ip),
                "port": pool[ip]["port"]
            })
    return json.dumps(config)

def stop_all_dsm_nodes():
    # docker rm $(docker stop $(docker ps -a -q --filter ancestor=<image-name> --format="{{.ID}}"))
    return subprocess.run(
        [
            "docker", 
            "rm", 
            "$(docker", 
            "stop", 
            "$(docker", 
            "ps", 
            "-a", 
            "-q", 
            "--filter", 
            "ancestor=dsm:latest", 
            "--format={{.ID}}))"
        ]
    )




def main():
    test_type = sys.argv[1]
    num = int(sys.argv[2])
    if test_type not in TEST_TYPES:
        raise RuntimeError("Do not support test type {}".format(test_type))
    # create network
    create_docker_network(NETWORK_NAME)
    # start param server
    start_docker_param_server(INIT_PORT, NETWORK_NAME, str(INIT_IP))
    # start nodes
    pool = generate_dsm_node_ip_id_port_map(num)
    last_ip = INIT_IP + 1
    for ip, id_port in pool.items():
        last_ip = ip
        if test_type != "mat":
            start_docker_node(generate_dsm_node_config(ip, pool), NETWORK_NAME, str(ip), id_port["id"], test_type)
        else: 
            start_docker_mat_node(generate_dsm_node_config(ip, pool), NETWORK_NAME, str(ip), id_port["id"], sys.argv[3:])
    last_ip += 1
    start_docker_web_node(str(last_ip))

    print("All nodes started")
    time.sleep(1)


if __name__ == "__main__":
    main()