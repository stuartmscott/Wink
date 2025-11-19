set(LIBRARY_NAME libWink)

set(TARGET_NAME After)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "after.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME At)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "at.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME StopWatch)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "stopwatch.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME Ticker)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "ticker.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
