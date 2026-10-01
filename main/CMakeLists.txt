include(${CMAKE_CURRENT_LIST_DIR}/sources.cmake)

idf_component_register(
    SRCS ${SOKOBAN_GAME_SRCS}
    INCLUDE_DIRS "." "assets"
    # Components go in REQUIRES, not INCLUDE_DIRS.
    REQUIRES gamebuino nvs_flash app_update esp_partition esp_system fatfs
)
