docker stop $(docker ps -a -q --filter ancestor=dsm:latest --format="{{.ID}}")
docker network rm dsm_network