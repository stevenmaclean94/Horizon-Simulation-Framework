//************************************************************************
//
// WGL bridge file.
// Implements structures and functions used by windows gl utility functions (WGL)
//
// Original work by Colin Fahey, http://www.colinfahey.com/opengl/csharp.htm
// Adapted by Chris Hegarty, avariant@hotmail.com
//   Removed demo code.
//   Modified pixel format structure implementation
//   Made funtion definitions safe (pointer free)
// Retrived from NeHe OpenGL resources by Brian Kirkpatrick, Tythos Creatives, on 10/19/2009
//   Formatted and integrated into a standalone dll defining a base OpenGL context and OpenGL,
//   WGL, and GLU symbols. WGL primarily extends relevant symbols from opengl32, user32, and gdi32
//**************************************************************************

using System;
using System.Runtime.InteropServices;

namespace CsOpenGl
{
	public class WGL
	{
        // Returns window / form's device context
		[DllImport("user32", EntryPoint = "GetDC")]
		public static extern uint GetDC( uint hwnd );

        // Releases handle of window / form device context
		[DllImport("user32", EntryPoint = "ReleaseDC")]
		public static extern int ReleaseDC( uint hwnd, uint dc );

        // Defines pixel format
		[StructLayout(LayoutKind.Sequential)] 
		public struct PIXELFORMATDESCRIPTOR 
		{
			public ushort  nSize; 
			public ushort  nVersion; 
			public uint    dwFlags; 
			public byte    iPixelType; 
			public byte    cColorBits; 
			public byte    cRedBits; 
			public byte    cRedShift; 
			public byte    cGreenBits; 
			public byte    cGreenShift; 
			public byte    cBlueBits; 
			public byte    cBlueShift; 
			public byte    cAlphaBits; 
			public byte    cAlphaShift; 
			public byte    cAccumBits; 
			public byte    cAccumRedBits; 
			public byte    cAccumGreenBits; 
			public byte    cAccumBlueBits; 
			public byte    cAccumAlphaBits; 
			public byte    cDepthBits; 
			public byte    cStencilBits; 
			public byte    cAuxBuffers; 
			public byte    iLayerType; 
			public byte    bReserved; 
			public uint    dwLayerMask; 
			public uint    dwVisibleMask; 
			public uint    dwDamageMask; 
			// 40 bytes total
		}

        // Default pixel format
		public static void ZeroPixelDescriptor(ref PIXELFORMATDESCRIPTOR pfd)
		{
			pfd.nSize           = 40; // sizeof(PIXELFORMATDESCRIPTOR); 
			pfd.nVersion        = 0; 
			pfd.dwFlags         = 0;
			pfd.iPixelType      = 0;
			pfd.cColorBits      = 0; 
			pfd.cRedBits        = 0; 
			pfd.cRedShift       = 0; 
			pfd.cGreenBits      = 0; 
			pfd.cGreenShift     = 0; 
			pfd.cBlueBits       = 0; 
			pfd.cBlueShift      = 0; 
			pfd.cAlphaBits      = 0; 
			pfd.cAlphaShift     = 0; 
			pfd.cAccumBits      = 0; 
			pfd.cAccumRedBits   = 0; 
			pfd.cAccumGreenBits = 0;
			pfd.cAccumBlueBits  = 0; 
			pfd.cAccumAlphaBits = 0;
			pfd.cDepthBits      = 0; 
			pfd.cStencilBits    = 0; 
			pfd.cAuxBuffers     = 0; 
			pfd.iLayerType      = 0;
			pfd.bReserved       = 0; 
			pfd.dwLayerMask     = 0; 
			pfd.dwVisibleMask   = 0; 
			pfd.dwDamageMask    = 0; 
		}

		/* pixel types */
		public const uint  PFD_TYPE_RGBA        = 0;
		public const uint  PFD_TYPE_COLORINDEX  = 1;

		/* layer types */
		public const uint  PFD_MAIN_PLANE       = 0;
		public const uint  PFD_OVERLAY_PLANE    = 1;
		public const uint  PFD_UNDERLAY_PLANE   = 0xff; // (-1)

		/* PIXELFORMATDESCRIPTOR flags */
		public const uint  PFD_DOUBLEBUFFER            = 0x00000001;
		public const uint  PFD_STEREO                  = 0x00000002;
		public const uint  PFD_DRAW_TO_WINDOW          = 0x00000004;
		public const uint  PFD_DRAW_TO_BITMAP          = 0x00000008;
		public const uint  PFD_SUPPORT_GDI             = 0x00000010;
		public const uint  PFD_SUPPORT_OPENGL          = 0x00000020;
		public const uint  PFD_GENERIC_FORMAT          = 0x00000040;
		public const uint  PFD_NEED_PALETTE            = 0x00000080;
		public const uint  PFD_NEED_SYSTEM_PALETTE     = 0x00000100;
		public const uint  PFD_SWAP_EXCHANGE           = 0x00000200;
		public const uint  PFD_SWAP_COPY               = 0x00000400;
		public const uint  PFD_SWAP_LAYER_BUFFERS      = 0x00000800;
		public const uint  PFD_GENERIC_ACCELERATED     = 0x00001000;
		public const uint  PFD_SUPPORT_DIRECTDRAW      = 0x00002000;

		/* PIXELFORMATDESCRIPTOR flags for use in ChoosePixelFormat only */
		public const uint  PFD_DEPTH_DONTCARE          = 0x20000000;
		public const uint  PFD_DOUBLEBUFFER_DONTCARE   = 0x40000000;
		public const uint  PFD_STEREO_DONTCARE         = 0x80000000;

		// Retrieves an index for a pixel format closest to what is passed
		[DllImport("gdi32", EntryPoint = "ChoosePixelFormat")]
		public static extern int ChoosePixelFormat( uint hdc, ref PIXELFORMATDESCRIPTOR p_pfd );

		// Sets the pixel format for the device context to the format specified by the index
		[DllImport("gdi32", EntryPoint = "SetPixelFormat")]
		public static extern uint SetPixelFormat( uint hdc, int iPixelFormat, ref PIXELFORMATDESCRIPTOR p_pfd );

		// Creates a rendering context for the Device context.
		[DllImport("opengl32", EntryPoint = "wglCreateContext")]
		public static extern uint wglCreateContext( uint hdc );

		// Sets the current rendering context
		[DllImport("opengl32", EntryPoint = "wglMakeCurrent")]
		public static extern int wglMakeCurrent( uint hdc, uint hglrc );

		// Deletes the rendering context
		[DllImport("opengl32", EntryPoint = "wglDeleteContext")]
		public static extern int wglDeleteContext( uint hglrc );

		// Swaps the display buffers in a double buffer context
		[DllImport("opengl32", EntryPoint = "wglSwapBuffers")]
		public static extern uint wglSwapBuffers( uint hdc );

	}
}
