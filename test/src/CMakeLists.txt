################################################################
# Tests

set(LIBRARY_NAME libWink)
set(SERVER_LIBRARY_NAME libWinkServer)

set(TARGET_NAME WinkTests)

set(SOURCE
  "address.cpp"
  "client.cpp"
  "machine.cpp"
  "server.cpp"
  "socket.cpp"
  "udp.cpp"
)

set(INCLUDE_DIR
  "../include"
  "../../include"
)

include_directories(${INCLUDE_DIR})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(
  ${TARGET_NAME}
  ${LIBRARY_NAME}
  ${SERVER_LIBRARY_NAME}
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
