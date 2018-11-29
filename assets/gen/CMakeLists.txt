unset( ASSET_COMMANDS )
unset( ASSET_FILES )
set( OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/generated )
file( MAKE_DIRECTORY ${OUTDIR} )
file( MAKE_DIRECTORY ${OUTDIR}/glsl )
file( MAKE_DIRECTORY ${OUTDIR}/essl )
file( MAKE_DIRECTORY ${OUTDIR}/dx9 )
file( MAKE_DIRECTORY ${OUTDIR}/dx11 )
file( MAKE_DIRECTORY ${OUTDIR}/metal )
macro( asset_shader FILE VAR )
	set( ARGS ${ARGN} )
	set( BASE_OPTIONS FILE ${FILE} INCLUDES ${BGFX_DIR}/src ${ARGS} )
	list( FIND ARGS VERTEX IND )
	if( IND EQUAL -1 )
		set( TYPE "ps" )
	else()
		set( TYPE "vs" )
	endif()

	# glsl
	set( OPTIONS ${BASE_OPTIONS} OUTPUT ${OUTDIR}/glsl/${VAR}.bin LINUX PROFILE 120 )
	shaderc_parse( OUT ${OPTIONS} )
	list( APPEND ASSET_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${OUT} )
	list( APPEND ASSET_FILES ${OUTDIR}/glsl/${VAR}.bin ${VAR}_glsl )

	# gles
	set( OPTIONS ${BASE_OPTIONS} OUTPUT ${OUTDIR}/essl/${VAR}.bin ANDROID PROFILE 120 )
	shaderc_parse( OUT ${OPTIONS} )
	list( APPEND ASSET_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${OUT} )
	list( APPEND ASSET_FILES ${OUTDIR}/essl/${VAR}.bin ${VAR}_essl )

	# dx9
	set( OPTIONS ${BASE_OPTIONS} OUTPUT ${OUTDIR}/dx9/${VAR}.bin WINDOWS PROFILE ${TYPE}_3_0 )
	shaderc_parse( OUT ${OPTIONS} )
	list( APPEND ASSET_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${OUT} )
	list( APPEND ASSET_FILES ${OUTDIR}/dx9/${VAR}.bin ${VAR}_dx9 )

	# dx11
	set( OPTIONS ${BASE_OPTIONS} OUTPUT ${OUTDIR}/dx11/${VAR}.bin WINDOWS PROFILE ${TYPE}_4_0 )
	shaderc_parse( OUT ${OPTIONS} )
	list( APPEND ASSET_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${OUT} )
	list( APPEND ASSET_FILES ${OUTDIR}/dx11/${VAR}.bin ${VAR}_dx11 )

	# metal
	set( OPTIONS ${BASE_OPTIONS} OUTPUT ${OUTDIR}/metal/${VAR}.bin OSX PROFILE metal )
	shaderc_parse( OUT ${OPTIONS} )
	list( APPEND ASSET_COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${OUT} )
	list( APPEND ASSET_FILES ${OUTDIR}/metal/${VAR}.bin ${VAR}_metal )
endmacro()
macro( asset FILE VAR )
	list( APPEND ASSET_FILES ${FILE} ${VAR} )
endmacro()

include( ../assets.cmake )

add_executable( bin2c bin2c.cpp )
add_custom_target(
	bigg_assets
	${ASSET_COMMANDS}
	COMMAND
	"$<TARGET_FILE:bin2c>" ${PROJECT_SOURCE_DIR}/src/bigg_assets.h ${ASSET_FILES}
	DEPENDS
	shaderc
	DEPENDS
	bin2c
	WORKING_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/..
)
set_target_properties( bin2c PROPERTIES FOLDER "assets" )
set_target_properties( bigg_assets PROPERTIES FOLDER "assets" )
