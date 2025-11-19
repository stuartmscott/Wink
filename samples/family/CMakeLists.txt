set(LIBRARY_NAME libWink)

set(PARENT_NAME Parent)

add_executable(${PARENT_NAME})

target_sources(${PARENT_NAME}
  PRIVATE
    "parent.cpp"
)

target_link_libraries(${PARENT_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)

set(CHILD_NAME Child)

add_executable(${CHILD_NAME})

target_sources(${CHILD_NAME}
  PRIVATE
    "child.cpp"
)

target_link_libraries(${CHILD_NAME}
  PRIVATE
    ${LIBRARY_NAME}
)
