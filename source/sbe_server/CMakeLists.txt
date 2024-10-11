
add_executable(sbe_server
    sbe_server.cpp
    # Add more source files here
)


target_link_libraries(sbe_client PRIVATE sbe_generated)


