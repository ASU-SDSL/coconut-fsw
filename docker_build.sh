# Build firmware inside the container
docker build . -t coconut_fsw --progress=plain

# Copy files out of the container
mkdir docker_build
docker cp coconut_fsw:/coconut-fsw/build/main/COCONUTFSW.uf2 ./docker_build/COCONUTFSW.uf2

#