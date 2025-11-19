set(LIBRARY_NAME libWink)

set(TARGET_NAME Empty)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "empty.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME Leaf)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "leaf.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME Simple)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "simple.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME Bigger)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "bigger.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(TARGET_NAME Forrest)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "forrest.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
