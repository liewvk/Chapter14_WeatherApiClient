cmake_minimum_required(VERSION 3.20)

project(Chapter14_WeatherApiClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(Chapter14_WeatherApiClient
    src/main.cpp
)

target_link_libraries(Chapter14_WeatherApiClient
    PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)
