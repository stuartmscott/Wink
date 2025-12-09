set(INCLUDE_DIR "../include")

################################################################
# Libraries

set(LIBRARY_NAME libWink)

add_library(${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
  PRIVATE
    "address.cpp"
    "async_mailbox.cpp"
    "client.cpp"
    "log.cpp"
    "machine.cpp"
    "udp.cpp"

  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${INCLUDE_DIR}
    FILES
      ${INCLUDE_DIR}/Wink/address.h
      ${INCLUDE_DIR}/Wink/client.h
      ${INCLUDE_DIR}/Wink/constants.h
      ${INCLUDE_DIR}/Wink/log.h
      ${INCLUDE_DIR}/Wink/machine.h
      ${INCLUDE_DIR}/Wink/mailbox.h
      ${INCLUDE_DIR}/Wink/socket.h
      ${INCLUDE_DIR}/Wink/state.h
)

install(
  TARGETS ${LIBRARY_NAME}
  EXPORT ${LIBRARY_NAME}Targets
  DESTINATION lib
  FILE_SET HEADERS DESTINATION include
)

install(
  EXPORT ${LIBRARY_NAME}Targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
  NAMESPACE Wink::
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}ConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}"
)

set(SERVER_LIBRARY_NAME libWinkServer)

add_library(${SERVER_LIBRARY_NAME})

target_link_libraries(${SERVER_LIBRARY_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

target_sources(${SERVER_LIBRARY_NAME}
  PRIVATE
    "server/server.cpp"

  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${INCLUDE_DIR}
    FILES
      ${INCLUDE_DIR}/WinkServer/server.h
)

install(
  TARGETS ${SERVER_LIBRARY_NAME}
  EXPORT ${SERVER_LIBRARY_NAME}Targets
  DESTINATION lib
  FILE_SET HEADERS DESTINATION include
)

install(
  EXPORT ${SERVER_LIBRARY_NAME}Targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${SERVER_LIBRARY_NAME}
  NAMESPACE WinkServer::
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${SERVER_LIBRARY_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${SERVER_LIBRARY_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${SERVER_LIBRARY_NAME}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${SERVER_LIBRARY_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${SERVER_LIBRARY_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${SERVER_LIBRARY_NAME}ConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${SERVER_LIBRARY_NAME}"
)

################################################################
# Executables

set(CLIENT_NAME Wink)

add_executable(${CLIENT_NAME})

target_sources(${CLIENT_NAME}
  PRIVATE
    "client/main.cpp"
)

target_link_libraries(${CLIENT_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

install(TARGETS ${CLIENT_NAME} DESTINATION bin)

set(SERVER_NAME WinkServer)

add_executable(${SERVER_NAME})

target_sources(${SERVER_NAME}
  PRIVATE
    "server/main.cpp"
)

target_link_libraries(${SERVER_NAME}
  PRIVATE
    ${SERVER_LIBRARY_NAME}
)

install(TARGETS ${SERVER_NAME} DESTINATION bin)
