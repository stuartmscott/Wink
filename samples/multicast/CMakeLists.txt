set(LIBRARY_NAME libWink)

set(RECEIVER_NAME Receiver)

add_executable(${RECEIVER_NAME})

target_sources(${RECEIVER_NAME}
  PRIVATE
    "receiver.cpp"
)

target_link_libraries(${RECEIVER_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(MULTICASTER_NAME Multicaster)

add_executable(${MULTICASTER_NAME})

target_sources(${MULTICASTER_NAME}
  PRIVATE
    "multicaster.cpp"
)

target_link_libraries(${MULTICASTER_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
