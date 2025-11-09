set(LIBRARY_NAME libWink)

set(INCLUDE_DIR "../../include")

set(PARENT_NAME Parent)

set(SOURCE
    "parent.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${PARENT_NAME} ${SOURCE})

target_link_libraries(${PARENT_NAME} ${LIBRARY_NAME})

set(CHILD_NAME Child)

set(SOURCE
    "child.cpp"
)

include_directories(${INCLUDE_DIR})

add_executable(${CHILD_NAME} ${SOURCE})

target_link_libraries(${CHILD_NAME} ${LIBRARY_NAME})
