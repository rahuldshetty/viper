cmake_minimum_required(VERSION 3.10.2)

project(${PACKAGE} LANGUAGES C)

SET(SRC_DIR "../src")

SET(PACKAGE "viper")
SET(PACKAGE_LIB "lib")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(gcc_generic_flags -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable)


ADD_EXECUTABLE(${PACKAGE} ${SRC_DIR}/main.c ${HEADERS})

set(VIPER_LIB_HEADERS
    ${SRC_DIR}/builtin.h   
    ${SRC_DIR}/bytes.h    
    ${SRC_DIR}/chunk.h
    ${SRC_DIR}/common.h
    ${SRC_DIR}/comp.h
    ${SRC_DIR}/compiler.h
    ${SRC_DIR}/debug.h
    ${SRC_DIR}/file.h
    ${SRC_DIR}/list.h
    ${SRC_DIR}/map.h
    ${SRC_DIR}/memory.h
    ${SRC_DIR}/object.h
    ${SRC_DIR}/runtime.h
    ${SRC_DIR}/scanner.h
    ${SRC_DIR}/table.h
    ${SRC_DIR}/token.h
    ${SRC_DIR}/utils.h
    ${SRC_DIR}/value.h
    ${SRC_DIR}/vm.h
)

set(VIPER_LIB_SOURCES
	${SRC_DIR}/builtin.c
    ${SRC_DIR}/bytes.c   
	${SRC_DIR}/chunk.c
    ${SRC_DIR}/comp.c
    ${SRC_DIR}/compiler.c
    ${SRC_DIR}/debug.c
    ${SRC_DIR}/file.c
    ${SRC_DIR}/list.c
    ${SRC_DIR}/map.c
    ${SRC_DIR}/memory.c
    ${SRC_DIR}/object.c
    ${SRC_DIR}/runtime.c
    ${SRC_DIR}/scanner.c
    ${SRC_DIR}/table.c
    ${SRC_DIR}/token.c
    ${SRC_DIR}/utils.c
    ${SRC_DIR}/value.c
    ${SRC_DIR}/vm.c
)


add_library(${PACKAGE_LIB} STATIC ${VIPER_LIB_SOURCES} ${VIPER_LIB_HEADERS})
target_link_libraries(${PACKAGE} PUBLIC ${PACKAGE_LIB} m)

install(TARGETS ${PACKAGE} DESTINATION bin)