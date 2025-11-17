set(LIBRARY_NAME libWink)

set(INCLUDE_DIRS
    "include"
    "../../include"
)

set(TARGET_NAME Universal)

set(SOURCE
    "tape.cpp"
    "universal.cpp"
)

include_directories(${INCLUDE_DIRS})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(${TARGET_NAME} ${LIBRARY_NAME})
