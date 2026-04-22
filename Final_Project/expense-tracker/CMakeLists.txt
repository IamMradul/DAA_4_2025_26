cmake_minimum_required(VERSION 3.20)
project(expense_tracker_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  cpp_httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.20.0
)
FetchContent_MakeAvailable(cpp_httplib)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(expense_tracker_cpp
  src/main.cpp
  src/utils.cpp
  src/models.cpp
  src/storage.cpp
  src/auth.cpp
  src/session.cpp
)

target_include_directories(expense_tracker_cpp PRIVATE src)

target_link_libraries(expense_tracker_cpp
  PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
)
