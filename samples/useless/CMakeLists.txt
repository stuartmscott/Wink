set(LIBRARY_NAME libWink)

set(INCLUDE_DIR "../../include")

set(TARGET_NAME Useless)

set(SOURCE
    "useless.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(${TARGET_NAME} ${LIBRARY_NAME})
