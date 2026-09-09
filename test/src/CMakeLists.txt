################################################################
# Tests

set(LIBRARY_NAME libWinkServer)

set(TARGET_NAME WinkTests)

set(INCLUDE_DIR "../include")

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "address.cpp"
    "async_mailbox.cpp"
    "client.cpp"
    "machine.cpp"
    "mailbox.cpp"
    "server.cpp"
    "socket.cpp"

  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${INCLUDE_DIR}
    FILES
      ${INCLUDE_DIR}/WinkTest/constants.h
      ${INCLUDE_DIR}/WinkTest/mailbox.h
      ${INCLUDE_DIR}/WinkTest/socket.h
      ${INCLUDE_DIR}/WinkTest/utils.h
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
    gtest_main
)

# Fetch GoogleTest Library
include(FetchContent)
FetchContent_Declare(
  GoogleTest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        063de7e9578f82b369302001269680b4b1553359 # release-1.18.0
)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest." FORCE)
FetchContent_MakeAvailable(GoogleTest)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
