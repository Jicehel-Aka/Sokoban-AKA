# Sokoban for Gamebuino AKA - desktop (SDL2) build, Linux / Windows (MSYS2 MinGW) / macOS.
#
#   cmake -S pc -B build-pc -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-pc
#
# The game and the gamebuino mid level library are compiled UNCHANGED, as for the
# console; only gb_ll_* (the hardware drivers) is replaced by pc/gb_ll_pc.cpp (SDL2).
cmake_minimum_required(VERSION 3.16)
project(sokoban_pc C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(SOKOBAN_PC_STATIC "Link the compiler runtime statically (MinGW: no libgcc/libstdc++ DLL to ship)" OFF)
option(SOKOBAN_PC_WERROR "Treat game warnings as errors (CI)" OFF)
option(SOKOBAN_PC_SMOKE  "Also build the backend smoke test" OFF)

set(ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(GB ${ROOT}/components/gamebuino)

find_package(SDL2 REQUIRED)
if(TARGET SDL2::SDL2)
  set(SDL2_LIB SDL2::SDL2)
elseif(TARGET SDL2)
  set(SDL2_LIB SDL2)
else()
  set(SDL2_LIB ${SDL2_LIBRARIES})
endif()

# --- the real gamebuino library, minus the hardware drivers --------------------
add_library(gamebuino_pc STATIC
  ${GB}/gb_lib/gb_core.cpp
  ${GB}/gb_lib/gb_graphics.cpp
  ${GB}/include_lib/gb_graphics_image.cpp
  ${GB}/gb_lib/gb_audio_player.cpp
  ${GB}/gb_lib/gb_audio_track_tone.cpp
  ${GB}/gb_lib/gb_audio_track_wav.cpp
  ${CMAKE_CURRENT_LIST_DIR}/gb_ll_pc.cpp)
target_include_directories(gamebuino_pc PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/idf_shim ${GB}/include_lib ${GB}/include_ll ${CMAKE_CURRENT_LIST_DIR})
if(SDL2_INCLUDE_DIRS)
  target_include_directories(gamebuino_pc PUBLIC ${SDL2_INCLUDE_DIRS})
endif()
# The console's compiler has an unsigned `char`; the font tables rely on it.
target_compile_options(gamebuino_pc PRIVATE -funsigned-char -w)
target_link_libraries(gamebuino_pc PUBLIC ${SDL2_LIB})

# --- the game ---------------------------------------------------------------------
include(${ROOT}/main/sources.cmake)
set(GAME_SRCS "")
foreach(f ${SOKOBAN_GAME_SRCS})
  list(APPEND GAME_SRCS ${ROOT}/main/${f})
endforeach()

add_executable(sokoban ${CMAKE_CURRENT_LIST_DIR}/main_pc.cpp ${GAME_SRCS})
target_include_directories(sokoban PRIVATE ${ROOT}/main)
target_compile_definitions(sokoban PRIVATE AKA_PC=1)
target_compile_options(sokoban PRIVATE -Wall -Wextra -fno-exceptions -fno-rtti)   # same as the console build
if(SOKOBAN_PC_WERROR)
  target_compile_options(sokoban PRIVATE -Werror)
endif()
target_link_libraries(sokoban PRIVATE gamebuino_pc)

if(SOKOBAN_PC_SMOKE)
  add_executable(pc_backend_smoke ${ROOT}/tests/pc_backend_smoke.cpp ${CMAKE_CURRENT_LIST_DIR}/main_pc.cpp)
  target_link_libraries(pc_backend_smoke PRIVATE gamebuino_pc)
endif()

if(MINGW)
  if(SOKOBAN_PC_STATIC)
    target_link_options(sokoban PRIVATE -static-libgcc -static-libstdc++)
  endif()
  target_link_options(sokoban PRIVATE -mwindows)
endif()
