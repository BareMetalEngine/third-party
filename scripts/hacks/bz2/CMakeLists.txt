cmake_minimum_required(VERSION 3.11)

project(bz2)

option(BZ2_LIB    "Use Static Libaray" ON)

set (BZ2_SRC
blocksort.cbzip2.cbzip2recover.cbzlib.ccompress.ccrctable.cdecompress.cdlltest.chuffman.cmk251.crandtable.cspewG.cunzcrash.c
)

include_directories(src)

add_definitions(-DNDEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (${BZ2_LIB})
    add_library(bz2 STATIC ${BZ2_SRC})
else()
    add_library(bz2 SHARED ${BZ2_SRC})
endif()

IF(MSVC)
  target_compile_options(bz2 PRIVATE /W3)
endif()

