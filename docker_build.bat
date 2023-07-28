:: Build firmware inside the container
docker build . -t coconut_fsw

:: Copy files out of the container
mkdir -p docker_build

docker run --name coconut_build -id coconut_fsw
docker cp coconut_build:/coconut-fsw/build/main/COCONUTFSW.uf2 ./docker_build/COCONUTFSW.uf2
docker kill coconut_build
docker rm coconut_build