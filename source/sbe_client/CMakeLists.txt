
add_executable(sbe_client
    sbe_client.cpp
    # Add more source files here
)


target_link_libraries(sbe_client PRIVATE sbe_generated)


