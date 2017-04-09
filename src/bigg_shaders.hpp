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