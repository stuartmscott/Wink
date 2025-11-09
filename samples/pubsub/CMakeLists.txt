set(LIBRARY_NAME libWink)

set(INCLUDE_DIR "../../include")

set(PUBLISHER_NAME Publisher)

set(SOURCE
    "publisher.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${PUBLISHER_NAME} ${SOURCE})

target_link_libraries(${PUBLISHER_NAME} ${LIBRARY_NAME})

set(SUBSCRIBER_NAME Subscriber)

set(SOURCE
    "subscriber.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${SUBSCRIBER_NAME} ${SOURCE})

target_link_libraries(${SUBSCRIBER_NAME} ${LIBRARY_NAME})
