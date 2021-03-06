# Linux detection:
if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR})



# dependency: glfw
# -- GLFW_INCLUDE_DIR
# -- GLFW_SOURCE_AMALGAMATION
# -- GLFW_DEPENDENCIES
set(GLFW_INCLUDE_DIR
	${CMAKE_CURRENT_LIST_DIR}/glfw/include)

set(GLFW_SOURCE_AMALGAMATION
	${CMAKE_CURRENT_LIST_DIR}/amalgs/glfw/glfw-amalg.c)
if (APPLE)
	set(GLFW_SOURCE_AMALGAMATION
		${GLFW_SOURCE_AMALGAMATION}
		${CMAKE_CURRENT_LIST_DIR}/amalgs/glfw/glfw-amalg.m)
endif()

set(GLFW_DEPENDENCIES)
if (APPLE)
	find_library(COCOA_LIBRARY Cocoa)
	find_library(IOKIT_LIBRARY IOKit)
	find_library(CORE_VIDEO_LIBRARY CoreVideo)
	set(GLFW_DEPENDENCIES
		${COCOA_LIBRARY}
		${IOKIT_LIBRARY}
		${CORE_VIDEO_LIBRARY}
	)
endif()


# dependency: imgui
set(IMGUI_INCLUDE_DIR
	${CMAKE_CURRENT_LIST_DIR}/imgui
	${CMAKE_CURRENT_LIST_DIR}/imgui/examples
)
set(IMGUI_SOURCE_AMALGAMATION
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/examples/imgui_impl_glfw.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/examples/imgui_impl_opengl2.cpp
)
set(IMGUI_FILESYSTEN_SOURCE_AMALGAMATION
	${CMAKE_CURRENT_LIST_DIR}/imguifilesystem/dirent_portable.h
	${CMAKE_CURRENT_LIST_DIR}/imguifilesystem/imguifilesystem.h		
	${CMAKE_CURRENT_LIST_DIR}/imguifilesystem/imguifilesystem.cpp
)



# dependency: OpenGL
find_package(OpenGL REQUIRED)



# dependency: X11
if (LINUX)
	find_package(X11 REQUIRED)
endif()
