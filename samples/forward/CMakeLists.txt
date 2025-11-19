set(LIBRARY_NAME libWink)

set(TARGET_NAME Forward)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
  PRIVATE
    "forward.cpp"
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
