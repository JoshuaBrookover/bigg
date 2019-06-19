# bgfx
set( BGFX_BUILD_TOOLS     ON  CACHE INTERNAL "" )
set( BGFX_BUILD_EXAMPLES  OFF CACHE INTERNAL "" )
set( BGFX_CUSTOM_TARGETS  OFF CACHE INTERNAL "" )
add_subdirectory( bgfx.cmake )

# glfw
set( GLFW_BUILD_EXAMPLES  OFF CACHE INTERNAL "" )
set( GLFW_BUILD_TESTS     OFF CACHE INTERNAL "" )
set( GLFW_BUILD_DOCS      OFF CACHE INTERNAL "" )
set( GLFW_INSTALL         OFF CACHE INTERNAL "" )
add_subdirectory( glfw )

# glm
set( GLM_TEST_ENABLE      OFF CACHE INTERNAL "" )
set( BUILD_SHARED_LIBS    OFF CACHE INTERNAL "" )
set( BUILD_STATIC_LIBS    OFF CACHE INTERNAL "" )
add_subdirectory( glm )

# imgui
include( imgui.cmake )
