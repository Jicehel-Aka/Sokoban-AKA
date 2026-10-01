# Sources of the game, shared by the ESP-IDF build (main/CMakeLists.txt) and the PC build (pc/CMakeLists.txt).
set(SOKOBAN_GAME_SRCS
  app_main.cpp
  app.cpp
  audio.cpp
  gfx.cpp
  i18n.cpp
  platform.cpp
  save.cpp
  assets/font_accents.cpp
  assets/tiles_data.cpp
  engine/sok_board.cpp
  engine/sok_game.cpp
  engine/sok_edit.cpp
  engine/sok_pack.cpp
  engine/sok_builtin.cpp
)
