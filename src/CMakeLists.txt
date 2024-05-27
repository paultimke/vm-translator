set(SOURCES
    codeWriter.c
    parser.c
    errorHandler.c
    keywords.c
    main.c
)

set(INCLUDES
    codeWriter.h
    parser.h
    errorHandler.h
    keywords.h
)

add_library(${PROJECT_NAME}lib STATIC ${SOURCES} ${INCLUDES})
add_executable(${PROJECT_NAME} ${SOURCES} ${INCLUDES})

message("Installing target into ${CMAKE_INSTALL_PREFIX}")
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
