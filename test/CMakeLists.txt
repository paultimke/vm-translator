set(THIS vm-translator-tests)

set(SOURCES 
    parser_test.cpp
)

set(INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}/src
)

add_subdirectory(googletest)

add_executable(${THIS} ${SOURCES})
target_include_directories(${THIS} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${THIS} PUBLIC
    gtest_main
    vm-translatorlib
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

