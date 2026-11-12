cmake_minimum_required(VERSION 3.10)
project(BankManagmentSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_PREFIX_PATH "/mnt/disk_d/Qt/5.15.2/gcc_64")

find_library(MYSQL_CPPCONN_LIB mysqlcppconn)
if (NOT MYSQL_CPPCONN_LIB)
    message(FATAL_ERROR "mysqlcppconn library not found")
endif()

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp" "main.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "include/*.h")

include_directories(include)

add_executable(BankManagmentSystem ${SOURCES} ${HEADERS})

target_link_libraries(BankManagmentSystem
    ${MYSQL_CPPCONN_LIB}
    OpenSSL::Crypto
    CURL::libcurl
    Qt5::Widgets
    Qt5::PrintSupport
)

enable_testing()
find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

add_executable(run_unit_tests
    ${TEST_SOURCES}
    src/CreditScorer.cpp
)

target_include_directories(run_unit_tests PRIVATE include)

target_link_libraries(run_unit_tests
    GTest::GTest
    OpenSSL::Crypto
    pthread
)

add_test(NAME AllUnitTests COMMAND run_unit_tests)
