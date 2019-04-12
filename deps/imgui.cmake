add_library(
	imgui
	STATIC
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
)
target_include_directories( imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui )
set_target_properties( imgui PROPERTIES FOLDER "imgui" )
