# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

project( Minamoto )

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required( VERSION 3.18.1 )

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_compile_options( -fno-exceptions
                     -fno-rtti
                     -fno-threadsafe-statics
                     -fvisibility=hidden )

# Houdini don't known llvm-ld
# if(NOT ${ANDROID_SYSROOT_ABI} STREQUAL "arm")
#     set( CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld -nostdlib++ -Wl,--build-id=sha1 -Wl,--Tbss=0x400" )
# endif()

# ARMv7 hidden symbols is broken since ndk-r17
# if(${ANDROID_SYSROOT_ABI} STREQUAL "arm")
#     add_compile_options( -mno-global-merge )
# endif()

add_library( # Sets the name of the library.
             Minamoto

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             dummy.cpp
             ../../../../../SDK/imgui/misc/freetype/imgui_freetype.cpp
             ../../../../../Minamoto/DearImGui.cpp
             ../../../../../Minamoto/mainJNI.cpp
             ../../../../../Minamoto/Plugin.cpp
             ../../../../../Minamoto/Renderer.cpp
             ../../../../../Minamoto/implement/imgui_impl_xx.cpp )

target_compile_definitions( Minamoto
                            PUBLIC
                            IMGUI_USER_CONFIG="../../build/include/imgui_user_config.h" )

target_include_directories( Minamoto
                            PUBLIC
                            ../../../../..
                            ../../../../../SDK )

add_dependencies( Minamoto
                  freetype
                  imgui
                  xxGraphic )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       Minamoto

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       android
                       freetype
                       imgui
                       xxGraphic )

# The FreeType 2 library

add_library( freetype
             SHARED
             ../../../../../SDK/freetype/src/autofit/autofit.c
             ../../../../../SDK/freetype/src/base/ftbase.c
             ../../../../../SDK/freetype/src/base/ftbbox.c
             ../../../../../SDK/freetype/src/base/ftbdf.c
             ../../../../../SDK/freetype/src/base/ftbitmap.c
             ../../../../../SDK/freetype/src/base/ftcid.c
             ../../../../../SDK/freetype/src/base/ftdebug.c
             ../../../../../SDK/freetype/src/base/ftfstype.c
             ../../../../../SDK/freetype/src/base/ftgasp.c
             ../../../../../SDK/freetype/src/base/ftglyph.c
             ../../../../../SDK/freetype/src/base/ftgxval.c
             ../../../../../SDK/freetype/src/base/ftinit.c
             ../../../../../SDK/freetype/src/base/ftmm.c
             ../../../../../SDK/freetype/src/base/ftotval.c
             ../../../../../SDK/freetype/src/base/ftpatent.c
             ../../../../../SDK/freetype/src/base/ftpfr.c
             ../../../../../SDK/freetype/src/base/ftstroke.c
             ../../../../../SDK/freetype/src/base/ftsynth.c
             ../../../../../SDK/freetype/src/base/ftsystem.c
             ../../../../../SDK/freetype/src/base/fttype1.c
             ../../../../../SDK/freetype/src/base/ftwinfnt.c
             ../../../../../SDK/freetype/src/bdf/bdf.c
             ../../../../../SDK/freetype/src/cache/ftcache.c
             ../../../../../SDK/freetype/src/cff/cff.c
             ../../../../../SDK/freetype/src/cid/type1cid.c
             ../../../../../SDK/freetype/src/gzip/ftgzip.c
             ../../../../../SDK/freetype/src/lzw/ftlzw.c
             ../../../../../SDK/freetype/src/pcf/pcf.c
             ../../../../../SDK/freetype/src/pfr/pfr.c
             ../../../../../SDK/freetype/src/psaux/psaux.c
             ../../../../../SDK/freetype/src/pshinter/pshinter.c
             ../../../../../SDK/freetype/src/psnames/psmodule.c
             ../../../../../SDK/freetype/src/raster/raster.c
             ../../../../../SDK/freetype/src/sfnt/sfnt.c
             ../../../../../SDK/freetype/src/smooth/smooth.c
             ../../../../../SDK/freetype/src/truetype/truetype.c
             ../../../../../SDK/freetype/src/type1/type1.c
             ../../../../../SDK/freetype/src/type42/type42.c
             ../../../../../SDK/freetype/src/winfonts/winfnt.c )

target_compile_definitions( freetype
                            PUBLIC
                            FT_CONFIG_MODULES_H="../../build/include/ftmodule.h"
                            FT_CONFIG_OPTIONS_H="../../build/include/ftoption.h"
                            FT2_BUILD_LIBRARY )

target_include_directories( freetype
                            PUBLIC
                            ../../../../../SDK/freetype/include )

# Dear ImGui

add_library( imgui
             SHARED
             ../../../../../SDK/imgui/imgui.cpp
             ../../../../../SDK/imgui/imgui_demo.cpp
             ../../../../../SDK/imgui/imgui_draw.cpp
             ../../../../../SDK/imgui/imgui_tables.cpp
             ../../../../../SDK/imgui/imgui_widgets.cpp )

target_compile_definitions( imgui
                            PUBLIC
                            IMGUI_USER_CONFIG="../../build/include/imgui_user_config.h"
                            IMGUI_API=__attribute__\(\(visibility\(\"default\"\)\)\) )

target_include_directories( imgui
                            PUBLIC
                            ../../../../../SDK
                            ../../../../../SDK/imgui
                            ../../../../../SDK/stb )

# xxGraphic

add_library( xxGraphic
             SHARED
             ../../../../../SDK/xxGraphic/internal/xxGraphicInternalGL.cpp
             ../../../../../SDK/xxGraphic/internal/xxGraphicInternalVulkan.cpp
             ../../../../../SDK/xxGraphic/xxGraphic.cpp
             ../../../../../SDK/xxGraphic/xxGraphicEGL.cpp
             ../../../../../SDK/xxGraphic/xxGraphicGLES2.cpp
             ../../../../../SDK/xxGraphic/xxGraphicGLES3.cpp
             ../../../../../SDK/xxGraphic/xxGraphicGLES31.cpp
             ../../../../../SDK/xxGraphic/xxGraphicGLES32.cpp
             ../../../../../SDK/xxGraphic/xxGraphicNULL.cpp
             ../../../../../SDK/xxGraphic/xxGraphicVulkan.cpp
             ../../../../../SDK/xxGraphic/xxGraphicVulkanAsm.cpp
             ../../../../../SDK/xxGraphic/xxSystem.cpp )

target_compile_definitions( xxGraphic
                            PUBLIC
                            XX_BUILD_LIBRARY )

target_include_directories( xxGraphic
                            PUBLIC
                            ../../../../../SDK/xxGraphic )

target_link_libraries( xxGraphic
                       ${log-lib}
                       android )

# xxGraphicPlus

add_library( xxGraphicPlus
             SHARED
             ../../../../../SDK/xxGraphic/utility/xxCamera.cpp
             ../../../../../SDK/xxGraphic/utility/xxImage.cpp
             ../../../../../SDK/xxGraphic/utility/xxMaterial.cpp
             ../../../../../SDK/xxGraphic/utility/xxMath.cpp
             ../../../../../SDK/xxGraphic/utility/xxMesh.cpp
             ../../../../../SDK/xxGraphic/utility/xxNode.cpp )

target_compile_definitions( xxGraphicPlus
                            PUBLIC
                            XXPLUS_BUILD_LIBRARY )

target_include_directories( xxGraphicPlus
                            PUBLIC
                            ../../../../../SDK/xxGraphic )

target_link_libraries( xxGraphicPlus
                       ${log-lib}
                       android
                       xxGraphic )

# sample

add_library( sample
             SHARED
             ../../../../../module/sample/sample.cpp )

target_compile_definitions( sample
                            PUBLIC
                            IMGUI_USER_CONFIG="../../build/include/imgui_user_config.h" )

target_include_directories( sample
                            PUBLIC
                            ../../../../..
                            ../../../../../SDK
                            ../../../../../module )

target_link_libraries( sample
                       imgui )

# validator

add_library( validator
             SHARED
             ../../../../../module/validator/validator.cpp )

target_compile_definitions( validator
                            PUBLIC
                            IMGUI_USER_CONFIG="../../build/include/imgui_user_config.h" )

target_include_directories( validator
                            PUBLIC
                            ../../../../..
                            ../../../../../SDK
                            ../../../../../SDK/xxGraphic
                            ../../../../../module )

target_link_libraries( validator
                       imgui
                       xxGraphic
                       xxGraphicPlus )
