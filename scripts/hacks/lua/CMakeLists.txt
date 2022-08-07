cmake_minimum_required(VERSION 3.11)

project(lua)

option(LUA_LIB    "Use Static Libaray" ON)

set (LUA_SRC
	lapi.c
	lauxlib.c
	lbaselib.c
	lcode.c
	lcorolib.c
	lctype.c
	ldblib.c
	ldebug.c
	ldo.c
	ldump.c
	lfunc.c
	lgc.c
	linit.c
	liolib.c
	llex.c
	lmathlib.c
	lmem.c
	loadlib.c
	lobject.c
	lopcodes.c
	loslib.c
	lparser.c
	lstate.c
	lstring.c
	lstrlib.c
	ltable.c
	ltablib.c
	ltests.c
	ltm.c
	#lua.c
	lundump.c
	lutf8lib.c
	lvm.c
	lzio.c
	#onelua.c
)

include_directories(src)

add_definitions(-DNDEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (${LUA_LIB})
    add_library(lua STATIC ${LUA_SRC})
else()
    add_library(lua SHARED ${LUA_SRC})
endif()

IF(MSVC)
  target_compile_options(lua PRIVATE /W3)
endif()

