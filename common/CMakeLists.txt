include(AddCatch2)


# Create a library from the common sources
add_library(common STATIC
    uqueue.cpp
    cmath.cpp
)

# Include the current directory to allow common.h to be found
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link Catch2 for unit tests
add_executable(uqueue_test uqueue_test.cpp)
target_link_libraries(uqueue_test common Catch2::Catch2WithMain)


# Enable testing and register the tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(uqueue_test)
