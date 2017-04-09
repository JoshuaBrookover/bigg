#include <bgfx/bgfx.h>
#include <imgui.h>
#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>

namespace bigg
{
	// bgfx utils
	const bgfx::Memory* loadMemory( const char* filename );
	bgfx::ShaderHandle loadShader( const char* shader );
	bgfx::ProgramHandle loadProgram( const char* vsName, const char* fsName );

	// application
	class Application
	{
		static void mouseButtonCallback( GLFWwindow* window, int button, int action, int mods );
		static void scrollCallback( GLFWwindow*, double xoffset, double yoffset );
		static void keyCallback( GLFWwindow*, int key, int, int action, int mods );
		static void charCallback( GLFWwindow*, unsigned int c );

		void imguiEvents( float dt );
	public:
		Application();

		int run(
			int argc,
			char** argv,
			bgfx::RendererType::Enum type = bgfx::RendererType::Count,
			uint16_t vendorId = BGFX_PCI_ID_NONE,
			uint16_t deviceId = 0,
			bgfx::CallbackI* callback = NULL,
			bx::AllocatorI* allocator = NULL
		);

		void reset( uint32_t flags = 0 );
		uint32_t getWidth();
		uint32_t getHeight();

		virtual void initialize( int _argc, char** _argv ) {};
		virtual void update( float dt ) {};
		virtual int shutdown() { return 0; };
		virtual void onReset() {};
	protected:
		GLFWwindow* mWindow;
	private:
		uint32_t mReset;
		uint32_t mWidth;
		uint32_t mHeight;
		bool  mMousePressed[ 3 ];
		float mMouseWheel;
	};
}