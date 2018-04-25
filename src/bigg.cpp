/*
 * This is free and unencumbered software released into the public domain. 
 */

#include <bigg.hpp>

#ifdef _WIN32
#	define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifdef __APPLE__
#	define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <bx/math.h>
#include <bgfx/platform.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <fstream>

#include "bigg_assets.h"
#include "bigg_shaders.hpp"
#include "bigg_imgui.hpp"

// bgfx utils

const bgfx::Memory* bigg::loadMemory( const char* filename )
{
	std::ifstream file( filename, std::ios::binary | std::ios::ate );
	std::streamsize size = file.tellg();
	file.seekg( 0, std::ios::beg );
	const bgfx::Memory* mem = bgfx::alloc( uint32_t( size + 1 ) );
	if ( file.read( ( char* )mem->data, size ) )
	{
		mem->data[ mem->size - 1 ] = '\0';
		return mem;
	}
	return nullptr;
}

bgfx::ShaderHandle bigg::loadShader( const char* shader )
{
	return bgfx::createShader( loadMemory( shader ) );
}

bgfx::ProgramHandle bigg::loadProgram( const char* vsName, const char* fsName )
{
	bgfx::ShaderHandle vs = loadShader( vsName );
	bgfx::ShaderHandle fs = loadShader( fsName );
	return bgfx::createProgram( vs, fs, true );
}

// glm utils

glm::tmat4x4<float, glm::defaultp> bigg::perspective( float fovy, float aspect, float zNear, float zFar )
{
	glm::tmat4x4<float, glm::defaultp> mtx;
#	if GLM_COORDINATE_SYSTEM == GLM_LEFT_HANDED
		bx::mtxProjLh( &mtx[ 0 ][ 0 ], glm::degrees( fovy ), aspect, zNear, zFar, bgfx::getCaps()->homogeneousDepth );
#	else
		bx::mtxProjRh( &mtx[ 0 ][ 0 ], glm::degrees( fovy ), aspect, zNear, zFar, bgfx::getCaps()->homogeneousDepth );
#	endif
	return mtx;
}

glm::tmat4x4<float, glm::defaultp> bigg::ortho( float left, float right, float bottom, float top, float zNear, float zFar, float offset )
{
	glm::tmat4x4<float, glm::defaultp> mtx;
#	if GLM_COORDINATE_SYSTEM == GLM_LEFT_HANDED
		bx::mtxOrthoLh( &mtx[ 0 ][ 0 ], left, right, bottom, top, zNear, zFar, offset, bgfx::getCaps()->homogeneousDepth );
#	else
		bx::mtxOrthoRh( &mtx[ 0 ][ 0 ], left, right, bottom, top, zNear, zFar, offset, bgfx::getCaps()->homogeneousDepth );
#	endif
	return mtx;
}

// application

void bigg::Application::keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	ImGuiIO& io = ImGui::GetIO();
	if ( action == GLFW_PRESS )
	{
		io.KeysDown[ key ] = true;
	}
	if ( action == GLFW_RELEASE )
	{
		io.KeysDown[ key ] = false;
	}
	io.KeyCtrl = io.KeysDown[ GLFW_KEY_LEFT_CONTROL ] || io.KeysDown[ GLFW_KEY_RIGHT_CONTROL ];
	io.KeyShift = io.KeysDown[ GLFW_KEY_LEFT_SHIFT ] || io.KeysDown[ GLFW_KEY_RIGHT_SHIFT ];
	io.KeyAlt = io.KeysDown[ GLFW_KEY_LEFT_ALT ] || io.KeysDown[ GLFW_KEY_RIGHT_ALT ];
	io.KeySuper = io.KeysDown[ GLFW_KEY_LEFT_SUPER ] || io.KeysDown[ GLFW_KEY_RIGHT_SUPER ];
	if ( !io.WantCaptureKeyboard )
	{
		app->onKey( key, scancode, action, mods );
	}
}

void bigg::Application::charCallback( GLFWwindow* window, unsigned int codepoint )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	ImGuiIO& io = ImGui::GetIO();
	if ( codepoint > 0 && codepoint < 0x10000 )
	{
		io.AddInputCharacter( ( unsigned short )codepoint );
	}
	app->onChar( codepoint );
}

void bigg::Application::charModsCallback( GLFWwindow* window, unsigned int codepoint, int mods )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->onCharMods( codepoint, mods );
}

void bigg::Application::mouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	ImGuiIO& io = ImGui::GetIO();
	if ( action == GLFW_PRESS && button >= 0 && button < 3 )
	{
		app->mMousePressed[button] = true;
	}
	if ( !io.WantCaptureMouse )
	{
		app->onMouseButton( button, action, mods );
	}
}

void bigg::Application::cursorPosCallback( GLFWwindow* window, double xpos, double ypos )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->onCursorPos( xpos, ypos );
}

void bigg::Application::cursorEnterCallback( GLFWwindow* window, int entered )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->onCursorEnter( entered );
}

void bigg::Application::scrollCallback( GLFWwindow* window, double xoffset, double yoffset )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->mMouseWheel += (float)yoffset;
	app->onScroll( xoffset, yoffset );
}

void bigg::Application::dropCallback( GLFWwindow* window, int count, const char** paths )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->onDrop( count, paths );
}

void bigg::Application::imguiEvents( float dt )
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = dt;
	int w, h;
	int displayW, displayH;
	glfwGetWindowSize( mWindow, &w, &h );
	glfwGetFramebufferSize( mWindow, &displayW, &displayH );
	io.DisplaySize = ImVec2( ( float )w, ( float )h );
	io.DisplayFramebufferScale = ImVec2( w > 0 ? ( ( float )displayW / w ) : 0, h > 0 ? ( ( float )displayH / h ) : 0 );
	if ( glfwGetWindowAttrib( mWindow, GLFW_FOCUSED ) )
	{
		double mouse_x, mouse_y;
		glfwGetCursorPos( mWindow, &mouse_x, &mouse_y );
		io.MousePos = ImVec2( ( float )mouse_x, ( float )mouse_y );
	}
	else
	{
		io.MousePos = ImVec2( -1, -1 );
	}
	for (int i = 0; i < 3; i++)
	{
		io.MouseDown[ i ] = mMousePressed[ i ] || glfwGetMouseButton( mWindow, i ) != 0;
		mMousePressed[ i ] = false;
	}
	io.MouseWheel = mMouseWheel;
	mMouseWheel = 0.0f;
	glfwSetInputMode( mWindow, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL );
	io.ClipboardUserData = mWindow;
#ifdef _WIN32
	io.ImeWindowHandle = glfwGetWin32Window( mWindow );
#endif
}

bigg::Application::Application()
{
	mWidth = 1280;
	mHeight = 768;
	mMousePressed[ 0 ] = false;
	mMousePressed[ 1 ] = false;
	mMousePressed[ 2 ] = false;
	mMouseWheel = 0.0f;
}

int bigg::Application::run( int argc, char** argv, bgfx::RendererType::Enum type, uint16_t vendorId, uint16_t deviceId, bgfx::CallbackI* callback, bx::AllocatorI* allocator )
{
	// Initialize the glfw
	if ( !glfwInit() )
	{
		return -1;
	}

	// Create a window
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	mWindow = glfwCreateWindow( getWidth(), getHeight(), "", NULL, NULL );
	if ( !mWindow )
	{
		glfwTerminate();
		return -1;
	}

	// Setup input callbacks
	glfwSetWindowUserPointer( mWindow, this );
	glfwSetKeyCallback( mWindow, keyCallback );
	glfwSetMouseButtonCallback( mWindow, mouseButtonCallback );
	glfwSetScrollCallback( mWindow, scrollCallback );
	glfwSetCharCallback( mWindow, charCallback );

	glfwSetKeyCallback( mWindow, keyCallback );
	glfwSetCharCallback( mWindow, charCallback );
	glfwSetCharModsCallback( mWindow, charModsCallback );
	glfwSetMouseButtonCallback( mWindow, mouseButtonCallback );
	glfwSetCursorPosCallback( mWindow, cursorPosCallback );
	glfwSetCursorEnterCallback( mWindow, cursorEnterCallback );
	glfwSetScrollCallback( mWindow, scrollCallback );
	glfwSetDropCallback( mWindow, dropCallback );

	// Setup bgfx
	bgfx::PlatformData platformData;
	memset( &platformData, 0, sizeof( platformData ) );
#	ifdef _WIN32
	platformData.nwh = glfwGetWin32Window( mWindow );
#	endif
#	ifdef __APPLE__
	platformData.nwh = glfwGetCocoaWindow( mWindow );
#	endif
	bgfx::setPlatformData( platformData );

	//bgfx::init( bgfx::RendererType::OpenGL, vendorId, deviceId, callback, allocator );

	uint32_t m_width = getWidth();
	uint32_t m_height = getHeight();
	uint32_t m_debug = BGFX_DEBUG_TEXT;
	uint32_t m_reset = BGFX_RESET_VSYNC;

	bgfx::Init init;
	init.type = bgfx::RendererType::OpenGL;
	init.vendorId = vendorId;
	init.resolution.width = m_width;
	init.resolution.height = m_height;
	init.resolution.reset = m_reset;
	bgfx::init(init);




	// Setup ImGui
	imguiInit();

	// Initialize the application
	reset();
	initialize( argc, argv );

	// Loop until the user closes the window
	float lastTime = 0;
	float dt;
	float time;
	while ( !glfwWindowShouldClose( mWindow ) )
	{
		time = ( float )glfwGetTime();
		dt = time - lastTime;
		lastTime = time;

		glfwPollEvents();
		imguiEvents( dt );
		ImGui::NewFrame();
		update( dt );
		ImGui::Render();
		bgfx::frame();

		int w, h;
		glfwGetWindowSize( mWindow, &w, &h );
		if ( w != mWidth || h != mHeight )
		{
			mWidth = w;
			mHeight = h;
			reset( mReset );
		}
	}

	// Shutdown application and glfw
	int ret = shutdown();
	imguiShutdown();
	bgfx::shutdown();
	glfwTerminate();
	return ret;
}

void bigg::Application::reset( uint32_t flags )
{
	mReset = flags;
	bgfx::reset( mWidth, mHeight, mReset );
	imguiReset( uint16_t( getWidth() ), uint16_t( getHeight() ) );
	onReset();
}

uint32_t bigg::Application::getWidth()
{
	return mWidth;
}

uint32_t bigg::Application::getHeight()
{
	return mHeight;
}