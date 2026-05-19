/*
 * Shim for SDL2_gfxPrimitives_font.h
 *
 * Older versions of SDL2_gfx shipped this header containing the
 * built-in 8x8 bitmap font (gfxPrimitivesFontdata). The Arch
 * Linux sdl2_gfx package no longer installs it, and the code in
 * this project only references the header to satisfy the
 * #include — no symbol from it is actually used. This empty
 * compatibility header makes the build go through.
 */
#ifndef _SDL2_gfxPrimitives_font_h
#define _SDL2_gfxPrimitives_font_h
#endif
