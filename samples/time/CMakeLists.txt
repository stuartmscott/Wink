set(LIBRARY_NAME libWink)

set(INCLUDE_DIR "../../include")

set(TARGET_NAME After)

set(SOURCE
    "after.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(${TARGET_NAME} ${LIBRARY_NAME})

set(TARGET_NAME At)

set(SOURCE
    "at.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(${TARGET_NAME} ${LIBRARY_NAME})

set(TARGET_NAME StopWatch)

set(SOURCE
    "stopwatch.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(${TARGET_NAME} ${LIBRARY_NAME})

set(TARGET_NAME Ticker)

set(SOURCE
    "ticker.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${TARGET_NAME} ${SOURCE})

target_link_libraries(${TARGET_NAME} ${LIBRARY_NAME})
