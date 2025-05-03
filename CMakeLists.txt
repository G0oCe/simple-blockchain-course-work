cmake_minimum_required(VERSION 3.10)
project(simple_blockchain)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# === Find and include Crypto++ ===
find_path(CRYPTOPP_INCLUDE_DIR cryptlib.h
    PATHS /usr/include /usr/local/include
    PATH_SUFFIXES cryptopp
)

find_library(CRYPTOPP_LIBRARY cryptopp
    PATHS /usr/lib /usr/local/lib
)

if (NOT CRYPTOPP_INCLUDE_DIR OR NOT CRYPTOPP_LIBRARY)
    message(FATAL_ERROR "Crypto++ not found. Please install libcrypto++-dev.")
endif()

include_directories(${CRYPTOPP_INCLUDE_DIR} include)

# === Sources ===
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(simple_blockchain ${SOURCES})

target_link_libraries(simple_blockchain
    ${CRYPTOPP_LIBRARY}
    ssl
    crypto
)


# === Tests ===
# If i add it in the future rsa_test.cpp or unit tests:
#enable_testing()
#add_executable(rsa_test test/rsa_test.cpp)
#target_include_directories(rsa_test PRIVATE ${CRYPTOPP_INCLUDE_DIR} include)
#target
