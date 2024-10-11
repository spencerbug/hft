# test/CMakelists.txt

include(CTest)

# Use CMake's FetchContent to download Catch2
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.3.2
)

# Make Catch2 available to the project
FetchContent_MakeAvailable(Catch2)

add_executable(unit_tests
    sbe_decode_example.cpp
    # Add more test source files here
)


# link libraries
target_link_libraries(unit_tests PRIVATE
    sbe_generated
    Catch2::Catch2WithMain # Link against catch2's main function
)

# enable testing and discover tests
# This macro integrates Catch2 with CTest, automatically discovering and registering tests
include(Catch)
catch_discover_tests(unit_tests)