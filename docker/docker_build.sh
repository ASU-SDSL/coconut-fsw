# EXPERIMENTAL, RECOMMENDED TO USE NATIVE LINUX BUILD
# Build firmware inside the container
docker build . -t coconut_fsw

# Copy files out of the container
mkdir -p docker_build

docker run -dit --name coconut_build -v $(pwd)/..:/coconut-fsw coconut_fsw bash

docker exec coconut_build bash -c "cd /coconut-fsw && ./deploy.sh -b -g"
docker exec coconut_build bash -c "cd /coconut-fsw && ./deploy.sh -b -s"

docker kill coconut_build
docker rm coconut_build
