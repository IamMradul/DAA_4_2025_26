# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-src")
  file(MAKE_DIRECTORY "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-src")
endif()
file(MAKE_DIRECTORY
  "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-build"
  "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix"
  "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/tmp"
  "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp"
  "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src"
  "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Project/expense-tracker-cpp/build/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
