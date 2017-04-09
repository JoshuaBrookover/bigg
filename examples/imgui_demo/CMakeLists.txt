add_executable( imgui_demo main.cpp )
target_link_libraries( imgui_demo bigg )
set_target_properties( imgui_demo PROPERTIES FOLDER "examples" )
