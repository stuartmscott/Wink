set(LIBRARY_NAME libWink)

set(PUBLISHER_NAME Publisher)

add_executable(${PUBLISHER_NAME})

target_sources(${PUBLISHER_NAME}
  PRIVATE
    "publisher.cpp"
)

target_link_libraries(${PUBLISHER_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(SUBSCRIBER_NAME Subscriber)

add_executable(${SUBSCRIBER_NAME})

target_sources(${SUBSCRIBER_NAME}
  PRIVATE
    "subscriber.cpp"
)

target_link_libraries(${SUBSCRIBER_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
