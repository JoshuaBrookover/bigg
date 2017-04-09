#include <bigg.hpp>
#include <bx/fpumath.h>

static bigg::PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};
static const uint16_t s_cubeTriList[] = { 0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6, 1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7, };
static const uint16_t s_cubeTriStrip[] = { 0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5, };

class ExampleCubes : public bigg::Application
{
	void initialize( int _argc, char** _argv )
	{
		mProgram = bigg::loadProgram( "shaders/glsl/vs_cubes.bin", "shaders/glsl/fs_cubes.bin" );
		mVbh = bgfx::createVertexBuffer( bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) ), bigg::PosColorVertex::ms_decl );
		mIbh = bgfx::createIndexBuffer( bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip) ) );
		bgfx::setDebug( BGFX_DEBUG_TEXT );
	}

	void onReset()
	{
		bgfx::setViewClear( 0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0 );
		bgfx::setViewRect( 0, 0, 0, uint16_t( getWidth() ), uint16_t( getHeight() ) );
	}

	void update( float dt )
	{
		static float time = 0;
		time += dt;

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf( 0, 1, 0x4f, "bgfx/examples/01-cube" );;
		bgfx::dbgTextPrintf( 0, 2, 0x6f, "Description: Rendering simple static mesh." );
		bgfx::dbgTextPrintf( 0, 3, 0x0f, "Frame: % 7.3f[ms]", double( dt ) * 1000 );

		float at[3]  = { 0.0f, 0.0f,   0.0f };
		float eye[3] = { 0.0f, 0.0f, -35.0f };

		// Set view and projection matrix for view 0.
		float view[16];
		bx::mtxLookAt( view, eye, at );
		float proj[16];
		bx::mtxProj( proj, 60.0f, float( getWidth() ) / float( getHeight() ), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth );
		bgfx::setViewTransform( 0, view, proj );

		// Set view 0 default viewport.
		bgfx::setViewRect( 0, 0, 0, uint16_t( getWidth() ), uint16_t( getHeight() ) );
		bgfx::touch( 0 );
		for ( uint32_t yy = 0; yy < 11; ++yy )
		{
			for ( uint32_t xx = 0; xx < 11; ++xx )
			{
				float mtx[ 16 ];
				bx::mtxRotateXY( mtx, time + xx*0.21f, time + yy*0.37f );
				mtx[ 12 ] = -15.0f + float( xx )*3.0f;
				mtx[ 13 ] = -15.0f + float( yy )*3.0f;
				mtx[ 14 ] = 0.0f;
				bgfx::setTransform( mtx );
				bgfx::setVertexBuffer( mVbh );
				bgfx::setIndexBuffer( mIbh );
				bgfx::setState( BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP );
				bgfx::submit( 0, mProgram );
			}
		}
	}
private:
	bgfx::ProgramHandle mProgram;
	bgfx::VertexBufferHandle mVbh;
	bgfx::IndexBufferHandle mIbh;
};

int main( int argc, char** argv )
{
	ExampleCubes app;
	return app.run( argc, argv, bgfx::RendererType::OpenGL );
}
