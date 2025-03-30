cmake_minimum_required(VERSION 3.10)
project(simple_blockchain)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include /usr/include/cryptopp)

# src
file(GLOB_RECURSE SOURCES
        src/*.cpp
)

add_executable(simple_blockchain ${SOURCES})

# Linking libraries directly (without find_package)
target_link_libraries(simple_blockchain
        cryptopp
        ssl
        crypto
)


# === Tests ===
# If i add it in the future rsa_test.cpp or unit tests:
#enable_testing()
#add_executable(rsa_test test/rsa_test.cpp)
#target_include_directories(rsa_test PRIVATE ${CRYPTOPP_INCLUDE_DIR} include)
#target
