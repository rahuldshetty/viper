cmake_minimum_required(VERSION 3.10.2)

SET(SRC_DIR "../src")

SET(PACKAGE "viper")
SET(PACKAGE_LIB "lib")

project(${PACKAGE} LANGUAGES C)
set(CMAKE_C_FLAGS "-std=c99")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

ADD_EXECUTABLE(${PACKAGE} ${SRC_DIR}/main.c)

add_library(${PACKAGE_LIB}
	${SRC_DIR}/builtin.c
    ${SRC_DIR}/builtin.h
	${SRC_DIR}/chunk.c
    ${SRC_DIR}/chunk.h
    ${SRC_DIR}/common.h
    ${SRC_DIR}/compiler.c
    ${SRC_DIR}/compiler.h
    ${SRC_DIR}/debug.c
    ${SRC_DIR}/debug.h
    ${SRC_DIR}/list.c
    ${SRC_DIR}/list.h
    ${SRC_DIR}/map.c
    ${SRC_DIR}/map.h
    ${SRC_DIR}/memory.c
    ${SRC_DIR}/memory.h
    ${SRC_DIR}/object.c
    ${SRC_DIR}/object.h
    ${SRC_DIR}/runtime.c
    ${SRC_DIR}/runtime.h
    ${SRC_DIR}/scanner.c
    ${SRC_DIR}/scanner.h
    ${SRC_DIR}/table.c
    ${SRC_DIR}/table.h
    ${SRC_DIR}/token.c
    ${SRC_DIR}/token.h
    ${SRC_DIR}/utils.c
    ${SRC_DIR}/utils.h
    ${SRC_DIR}/value.c
    ${SRC_DIR}/value.h
    ${SRC_DIR}/vm.c
    ${SRC_DIR}/vm.h
)

target_link_libraries(${PACKAGE} PUBLIC ${PACKAGE_LIB})

install(TARGETS ${PACKAGE} DESTINATION bin)