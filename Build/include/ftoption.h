/****************************************************************************
 *
 * ftoption.h
 *
 *   User-selectable configuration macros (specification only).
 *
 * Copyright (C) 1996-2021 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <freetype/config/ftoption.h>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4101)
#pragma warning(disable : 4189)
#pragma warning(disable : 4267)
#pragma warning(disable : 4701)
#pragma warning(disable : 4706)
#pragma warning(disable : 4819)

#undef FT_CONFIG_OPTION_ENVIRONMENT_PROPERTIES
#undef FT_CONFIG_OPTION_USE_LZW
#undef FT_CONFIG_OPTION_USE_ZLIB
#undef FT_CONFIG_OPTION_POSTSCRIPT_NAMES
#undef FT_CONFIG_OPTION_ADOBE_GLYPH_LIST
#undef FT_CONFIG_OPTION_MAC_FONTS
#undef FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK
#undef FT_CONFIG_OPTION_INCREMENTAL
#undef FT_CONFIG_OPTION_SVG

#undef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
/* #undef TT_CONFIG_OPTION_COLOR_LAYERS */
#undef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
#undef TT_CONFIG_OPTION_SFNT_NAMES

#undef TT_CONFIG_CMAP_FORMAT_0
#undef TT_CONFIG_CMAP_FORMAT_2
/* #undef TT_CONFIG_CMAP_FORMAT_4 */
#undef TT_CONFIG_CMAP_FORMAT_6
#undef TT_CONFIG_CMAP_FORMAT_8
#undef TT_CONFIG_CMAP_FORMAT_10
/* #undef TT_CONFIG_CMAP_FORMAT_12 */
#undef TT_CONFIG_CMAP_FORMAT_13
#undef TT_CONFIG_CMAP_FORMAT_14

#undef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#undef TT_CONFIG_OPTION_SUBPIXEL_HINTING
#undef TT_CONFIG_OPTION_GX_VAR_SUPPORT
#undef TT_CONFIG_OPTION_BDF

#undef TT_USE_BYTECODE_INTERPRETER
#undef TT_SUPPORT_SUBPIXEL_HINTING_MINIMAL
/* #undef TT_SUPPORT_COLRV1 */

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-function"
#endif

/* END */
