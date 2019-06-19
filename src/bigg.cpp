/*
 * This is free and unencumbered software released into the public domain. 
 */

#include <bigg.hpp>

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	define GLFW_EXPOSE_NATIVE_X11
#	define GLFW_EXPOSE_NATIVE_GLX
#elif BX_PLATFORM_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif // BX_PLATFORM_
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

void bigg::Application::windowSizeCallback( GLFWwindow* window, int width, int height )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->mWidth = width;
	app->mHeight = height;
	app->reset( app->mReset );
	app->onWindowSize( width, height );
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

bigg::Application::Application( const char* title, uint32_t width, uint32_t height )
{
	mWidth = width;
	mHeight = height;
	mTitle = title;
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
	mWindow = glfwCreateWindow( getWidth(), getHeight(), getTitle(), NULL, NULL );
	if ( !mWindow )
	{
		glfwTerminate();
		return -1;
	}

	// Setup input callbacks
	glfwSetWindowUserPointer( mWindow, this );
	glfwSetKeyCallback( mWindow, keyCallback );
	glfwSetCharCallback( mWindow, charCallback );
	glfwSetCharModsCallback( mWindow, charModsCallback );
	glfwSetMouseButtonCallback( mWindow, mouseButtonCallback );
	glfwSetCursorPosCallback( mWindow, cursorPosCallback );
	glfwSetCursorEnterCallback( mWindow, cursorEnterCallback );
	glfwSetScrollCallback( mWindow, scrollCallback );
	glfwSetDropCallback( mWindow, dropCallback );
	glfwSetWindowSizeCallback( mWindow, windowSizeCallback );

	// Setup bgfx
	bgfx::PlatformData platformData;
	memset( &platformData, 0, sizeof( platformData ) );
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(mWindow);
	platformData.ndt = glfwGetX11Display();
#elif BX_PLATFORM_OSX
	platformData.nwh = glfwGetCocoaWindow(mWindow);
#elif BX_PLATFORM_WINDOWS
	platformData.nwh = glfwGetWin32Window(mWindow);
#endif // BX_PLATFORM_
	bgfx::setPlatformData( platformData );

	// Init bgfx
	bgfx::Init init;
	init.type = type;
	init.vendorId = vendorId;
	init.deviceId = deviceId;
	init.callback = callback;
	init.allocator = allocator;
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

uint32_t bigg::Application::getWidth() const
{
	return mWidth;
}

uint32_t bigg::Application::getHeight() const
{
	return mHeight;
}

void bigg::Application::setSize( int width, int height )
{
	glfwSetWindowSize( mWindow, width, height );
}

const char* bigg::Application::getTitle() const
{
	return mTitle;
}

void bigg::Application::setTitle( const char* title )
{
	mTitle = title;
	glfwSetWindowTitle( mWindow, title);
}
