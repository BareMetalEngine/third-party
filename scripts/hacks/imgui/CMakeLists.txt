cmake_minimum_required(VERSION 3.11)

project(imgui)

set(CMAKE_CXX_STANDARD 17)
option(IMGUI_LIB    "Use Static Libaray" ON)

set (IMGUI_SRC
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
)

add_definitions(-DNDEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (${IMGUI_LIB})
    add_library(imgui STATIC ${IMGUI_SRC})
else()
    add_library(imgui SHARED ${IMGUI_SRC})
endif()

IF(MSVC)
  target_compile_options(imgui PRIVATE /W3)
endif()

