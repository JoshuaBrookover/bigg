#include <bigg.hpp>
#include "bigg_assets.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <bx/fpumath.h>
#include <bgfx/platform.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <fstream>

// TODO:
// Gnm - pssl
// Metal - metal
// Vulkan - spirv
#define _getShader(name)                                             \
static const unsigned char* name##()                                 \
{                                                                    \
	switch ( bgfx::getRendererType() )                               \
	{                                                                \
		case bgfx::RendererType::Noop:                               \
		case bgfx::RendererType::Direct3D9:  return name##_dx9;      \
		case bgfx::RendererType::Direct3D11:                         \
		case bgfx::RendererType::Direct3D12: return name##_dx11;     \
		case bgfx::RendererType::OpenGL:     return name##_glsl;     \
		case bgfx::RendererType::OpenGLES:   return name##_essl;     \
	}                                                                \
	return NULL;                                                     \
}                                                                    \
static const int name##_len()                                        \
{                                                                    \
	switch ( bgfx::getRendererType() )                               \
	{                                                                \
		case bgfx::RendererType::Noop:                               \
		case bgfx::RendererType::Direct3D9:  return name##_dx9_len;  \
		case bgfx::RendererType::Direct3D11:                         \
		case bgfx::RendererType::Direct3D12: return name##_dx11_len; \
		case bgfx::RendererType::OpenGL:     return name##_glsl_len; \
		case bgfx::RendererType::OpenGLES:   return name##_essl_len; \
	}                                                                \
	return 0;                                                        \
}
_getShader( vs_ocornut_imgui );
_getShader( fs_ocornut_imgui );

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

// imgui setup

static bgfx::VertexDecl    imguiVertexDecl;
static bgfx::TextureHandle imguiFontTexture;
static bgfx::UniformHandle imguiFontUniform;
static bgfx::ProgramHandle imguiProgram;
static void                imguiRender( ImDrawData* drawData );
static void                imguiShutdown();
static const char*         imguiGetClipboardText( void* userData );
static void                imguiSetClipboardText( void* userData, const char* text );

static void imguiInit()
{
	unsigned char* data;
	int width, height;
	ImGuiIO& io = ImGui::GetIO();

	// Setup vertex declaration
	imguiVertexDecl
		.begin()
		.add( bgfx::Attrib::Position, 2, bgfx::AttribType::Float )
		.add( bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float )
		.add( bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true )
		.end();

	// Create font
	io.Fonts->AddFontDefault();
	io.Fonts->GetTexDataAsRGBA32( &data, &width, &height );
	imguiFontTexture = bgfx::createTexture2D( ( uint16_t )width, ( uint16_t )height, false, 1, bgfx::TextureFormat::BGRA8, 0, bgfx::copy( data, width*height * 4 ) );
	imguiFontUniform = bgfx::createUniform( "s_tex", bgfx::UniformType::Int1 );

	// Create shader program
	vs_ocornut_imgui();
	bgfx::ShaderHandle vs = bgfx::createShader( bgfx::makeRef( vs_ocornut_imgui(), vs_ocornut_imgui_len() ) );
	bgfx::ShaderHandle fs = bgfx::createShader( bgfx::makeRef( fs_ocornut_imgui(), fs_ocornut_imgui_len() ) );
	imguiProgram = bgfx::createProgram( vs, fs, true );

	// Setup render callback
	io.RenderDrawListsFn = imguiRender;

	// Key mapping
	io.KeyMap[ ImGuiKey_Tab ] = GLFW_KEY_TAB;
	io.KeyMap[ ImGuiKey_LeftArrow ] = GLFW_KEY_LEFT;
	io.KeyMap[ ImGuiKey_RightArrow ] = GLFW_KEY_RIGHT;
	io.KeyMap[ ImGuiKey_UpArrow ] = GLFW_KEY_UP;
	io.KeyMap[ ImGuiKey_DownArrow ] = GLFW_KEY_DOWN;
	io.KeyMap[ ImGuiKey_PageUp ] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ ImGuiKey_PageDown ] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ ImGuiKey_Home ] = GLFW_KEY_HOME;
	io.KeyMap[ ImGuiKey_End ] = GLFW_KEY_END;
	io.KeyMap[ ImGuiKey_Delete ] = GLFW_KEY_DELETE;
	io.KeyMap[ ImGuiKey_Backspace ] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ ImGuiKey_Enter ] = GLFW_KEY_ENTER;
	io.KeyMap[ ImGuiKey_Escape ] = GLFW_KEY_ESCAPE;
	io.KeyMap[ ImGuiKey_A ] = GLFW_KEY_A;
	io.KeyMap[ ImGuiKey_C ] = GLFW_KEY_C;
	io.KeyMap[ ImGuiKey_V ] = GLFW_KEY_V;
	io.KeyMap[ ImGuiKey_X ] = GLFW_KEY_X;
	io.KeyMap[ ImGuiKey_Y ] = GLFW_KEY_Y;
	io.KeyMap[ ImGuiKey_Z ] = GLFW_KEY_Z;
	io.SetClipboardTextFn = imguiSetClipboardText;
	io.GetClipboardTextFn = imguiGetClipboardText;
}

static void imguiRender( ImDrawData* drawData )
{
	for ( int ii = 0, num = drawData->CmdListsCount; ii < num; ++ii )
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::TransientIndexBuffer tib;

		const ImDrawList* drawList = drawData->CmdLists[ii];
		uint32_t numVertices = ( uint32_t )drawList->VtxBuffer.size();
		uint32_t numIndices  = ( uint32_t )drawList->IdxBuffer.size();

		if ( !bgfx::getAvailTransientVertexBuffer( numVertices, imguiVertexDecl ) || !bgfx::getAvailTransientIndexBuffer( numIndices ) )
		{
			break;
		}

		bgfx::allocTransientVertexBuffer( &tvb, numVertices, imguiVertexDecl );
		bgfx::allocTransientIndexBuffer( &tib, numIndices );

		ImDrawVert* verts = ( ImDrawVert* )tvb.data;
		memcpy( verts, drawList->VtxBuffer.begin(), numVertices * sizeof( ImDrawVert ) );

		ImDrawIdx* indices = ( ImDrawIdx* )tib.data;
		memcpy( indices, drawList->IdxBuffer.begin(), numIndices * sizeof( ImDrawIdx ) );

		uint32_t offset = 0;
		for ( const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd )
		{
			if ( cmd->UserCallback )
			{
				cmd->UserCallback( drawList, cmd );
			}
			else if ( 0 != cmd->ElemCount )
			{
				uint64_t state = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_MSAA;
				bgfx::TextureHandle th = imguiFontTexture;
				if ( cmd->TextureId != NULL )
				{
					union { ImTextureID ptr; struct { uint16_t flags; bgfx::TextureHandle handle; } s; } texture = { cmd->TextureId };
					state |= BGFX_STATE_BLEND_FUNC( BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA );
					th = texture.s.handle;
				}
				else
				{
					state |= BGFX_STATE_BLEND_FUNC( BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA );
				}
				const uint16_t xx = uint16_t( bx::fmax( cmd->ClipRect.x, 0.0f ) );
				const uint16_t yy = uint16_t( bx::fmax( cmd->ClipRect.y, 0.0f ) );
				bgfx::setScissor( xx, yy, uint16_t( bx::fmin( cmd->ClipRect.z, 65535.0f ) - xx ), uint16_t( bx::fmin( cmd->ClipRect.w, 65535.0f ) - yy ) );
				bgfx::setState( state );
				bgfx::setTexture( 0, imguiFontUniform, th );
				bgfx::setVertexBuffer( &tvb, 0, numVertices );
				bgfx::setIndexBuffer( &tib, offset, cmd->ElemCount );
				bgfx::submit( 0, imguiProgram );
			}

			offset += cmd->ElemCount;
		}
	}
}

static void imguiShutdown()
{
	bgfx::destroyUniform( imguiFontUniform );
	bgfx::destroyTexture( imguiFontTexture );
	bgfx::destroyProgram( imguiProgram );
	ImGui::Shutdown();
}

static const char* imguiGetClipboardText( void* userData )
{
	return glfwGetClipboardString( ( GLFWwindow* )userData );
}

static void imguiSetClipboardText( void* userData, const char* text )
{
	glfwSetClipboardString( ( GLFWwindow* )userData, text );
}

// application

void bigg::Application::mouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	if ( action == GLFW_PRESS && button >= 0 && button < 3 )
	{
		app->mMousePressed[button] = true;
	}
}

void bigg::Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	bigg::Application* app = ( bigg::Application* )glfwGetWindowUserPointer( window );
	app->mMouseWheel += (float)yoffset;
}

void bigg::Application::keyCallback( GLFWwindow*, int key, int, int action, int mods )
{
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
}

void bigg::Application::charCallback(GLFWwindow*, unsigned int c)
{
	ImGuiIO& io = ImGui::GetIO();
	if ( c > 0 && c < 0x10000 )
	{
		io.AddInputCharacter( ( unsigned short )c );
	}
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
	mWindow = glfwCreateWindow( 1280, 768, "", NULL, NULL );
	if ( !mWindow )
	{
		glfwTerminate();
		return -1;
	}

	// Setup input callbacks
	glfwSetWindowUserPointer( mWindow, this );
	glfwSetMouseButtonCallback( mWindow, mouseButtonCallback );
	glfwSetScrollCallback( mWindow, scrollCallback );
	glfwSetKeyCallback( mWindow, keyCallback );
	glfwSetCharCallback( mWindow, charCallback );

	// Setup bgfx
	bgfx::PlatformData platformData;
	memset( &platformData, 0, sizeof( platformData ) );
	platformData.nwh = glfwGetWin32Window( mWindow );
	bgfx::setPlatformData( platformData );
	bgfx::init( type, vendorId, deviceId, callback, allocator );

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
			reset();
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