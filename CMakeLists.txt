cmake_minimum_required(VERSION 3.16)

project(DeliveryManagementSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)
find_package(unofficial-libmysql REQUIRED)

add_executable(server
    api/server.cpp
)

target_link_libraries(server
    PRIVATE
    Crow::Crow
    unofficial::libmysql::libmysql
)