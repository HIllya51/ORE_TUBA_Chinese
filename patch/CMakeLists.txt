cmake_minimum_required(VERSION 3.16)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_FINAL_OUTPUT_DIRECTORY "C:\\dataH\\俺たちに翼はない")
 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_FINAL_OUTPUT_DIRECTORY}>) 
add_compile_options(
  /std:c++17
  /MP
  /wd4018 
  /wd4819 
  /wd4244
  /wd4267
  /DVERSION="${VERSION}"
  /DUNICODE 
  /D_UNICODE
)
project(patch)

include_directories(Detours-4.0.1/include) 
add_library(lyric lyric/lyric.cpp lyric/OverlayLyric.cpp lyric/Wnd.cpp)

add_library(ORE_TUBA_CHS MODULE patch.cpp patch.rc)   
target_link_libraries(ORE_TUBA_CHS Gdiplus Shlwapi lyric ${CMAKE_CURRENT_SOURCE_DIR}/Detours-4.0.1/lib.X86/detours.lib)
add_library(ORE_TUBA_CHT MODULE patch.cpp patch.rc)   
target_compile_definitions(ORE_TUBA_CHT PRIVATE ISCHT)
target_link_libraries(ORE_TUBA_CHT Gdiplus Shlwapi lyric ${CMAKE_CURRENT_SOURCE_DIR}/Detours-4.0.1/lib.X86/detours.lib)

 