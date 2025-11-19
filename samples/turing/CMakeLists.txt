set(LIBRARY_NAME libWink)

set(TARGET_NAME Universal)

add_executable(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PRIVATE "include")

target_sources(${TARGET_NAME}
  PRIVATE
    "include/tape.h"
    "tape.cpp"
    "universal.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
