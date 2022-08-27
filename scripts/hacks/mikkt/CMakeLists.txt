cmake_minimum_required(VERSION 3.11)

project(mikkt)

option(MIKKT_LIB    "Use Static Libaray" ON)

set (MIKKT_SRC
	mikktspace.c	
)

include_directories(src)

add_definitions(-DNDEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (${MIKKT_LIB})
    add_library(mikkt STATIC ${MIKKT_SRC})
else()
    add_library(mikkt SHARED ${MIKKT_SRC})
endif()

IF(MSVC)
  target_compile_options(mikkt PRIVATE /W3)
endif()

