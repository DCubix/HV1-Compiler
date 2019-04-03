cmake_minimum_required(VERSION 3.10)
project(hv1 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

file(GLOB_RECURSE SRC
	"src/*.h"
	"src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC})