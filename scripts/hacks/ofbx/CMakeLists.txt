cmake_minimum_required(VERSION 3.11)

project(OFBX)

option(OFBX_LIB    "Use Static Libaray" ON)

set (OFBX_SRC    
    src/ofbx.cpp
    src/miniz.c
)

include_directories(src)

add_definitions(-DNDEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (${OFBX_LIB})
    add_library(OFBX STATIC ${OFBX_SRC})
else()
    add_library(OFBX SHARED ${OFBX_SRC})
endif()

IF(MSVC)
  target_compile_options(OFBX PRIVATE /W3)
endif()

