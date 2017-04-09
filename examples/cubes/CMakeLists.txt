add_executable( cubes cubes.cpp fs_cubes.sc vs_cubes.sc )
target_link_libraries( cubes bigg )
set_target_properties( cubes PROPERTIES FOLDER "examples" )

add_shader( vs_cubes.sc VERTEX   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders DX11_MODEL 5_0 GLSL 130 )
add_shader( fs_cubes.sc FRAGMENT OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders DX11_MODEL 5_0 GLSL 130 )

configure_debugging( cubes WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR} )
