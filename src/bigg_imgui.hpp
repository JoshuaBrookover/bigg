/*
 * This is free and unencumbered software released into the public domain. 
 */

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
	ImGui::CreateContext();
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
	imguiFontUniform = bgfx::createUniform( "s_tex", bgfx::UniformType::Sampler );

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

static void imguiReset( uint16_t width, uint16_t height )
{
	bgfx::setViewRect( 200, 0, 0, width, height );
	bgfx::setViewClear( 0, BGFX_CLEAR_COLOR, 0x00000000 );
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
				uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;
				bgfx::TextureHandle th = imguiFontTexture;
				if ( cmd->TextureId != NULL )
				{
					th.idx = uint16_t( uintptr_t( cmd->TextureId ) );
				}
				state |= BGFX_STATE_BLEND_FUNC( BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA );
				const uint16_t xx = uint16_t( bx::max( cmd->ClipRect.x, 0.0f ) );
				const uint16_t yy = uint16_t( bx::max( cmd->ClipRect.y, 0.0f ) );
				bgfx::setScissor( xx, yy, uint16_t( bx::min( cmd->ClipRect.z, 65535.0f ) - xx ), uint16_t( bx::min( cmd->ClipRect.w, 65535.0f ) - yy ) );
				bgfx::setState( state );
				bgfx::setTexture( 0, imguiFontUniform, th );
				bgfx::setVertexBuffer( 0, &tvb, 0, numVertices );
				bgfx::setIndexBuffer( &tib, offset, cmd->ElemCount );
				bgfx::submit( 200, imguiProgram );
			}

			offset += cmd->ElemCount;
		}
	}
}

static void imguiShutdown()
{
	bgfx::destroy( imguiFontUniform );
	bgfx::destroy( imguiFontTexture );
	bgfx::destroy( imguiProgram );
	ImGui::DestroyContext();
}

static const char* imguiGetClipboardText( void* userData )
{
	return glfwGetClipboardString( ( GLFWwindow* )userData );
}

static void imguiSetClipboardText( void* userData, const char* text )
{
	glfwSetClipboardString( ( GLFWwindow* )userData, text );
}
