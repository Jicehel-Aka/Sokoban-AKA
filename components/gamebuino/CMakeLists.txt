idf_component_register(
    SRCS 

            # Gamebuino low level : drivers
        "gb_ll/gb_ll_i2c.c"
        "gb_ll/gb_ll_adc.c"
        "gb_ll/gb_ll_audio.c"
        "gb_ll/gb_ll_lcd.c"
        "gb_ll/gb_ll_expander.c"
        "gb_ll/gb_ll_sdcard.c"
        "gb_ll/gb_ll_system.c"

            # Gamebuino mid level librarys : user entry level
        "gb_lib/gb_audio_player.cpp"
        "gb_lib/gb_audio_track_tone.cpp"
        "gb_lib/gb_audio_track_fx.cpp"
        "gb_lib/gb_audio_track_wav.cpp"
        "gb_lib/pmf_player/pmf_player.cpp"
        "gb_lib/gb_audio_track_pmf.cpp"
        "gb_lib/gb_core.cpp"
        "gb_lib/gb_graphics.cpp"
        "include_lib/gb_graphics_image.cpp"  # AJOUTE (Claude, session Dark & Under) :
                                              # famille drawImage/drawImageScaled, deja
                                              # utilisee par poa_aka, absente jusqu'ici de
                                              # cette bibliotheque -- voir gb_graphics.h
        
    REQUIRES 
        esp_driver_ledc 
        esp_adc 
        esp_driver_i2c 
        esp_driver_gpio 
        spi_flash 
        esp_psram 
        esp_lcd 
        esp_timer 
        esp_driver_i2s 
        fatfs
    
    INCLUDE_DIRS "include_lib" "include_ll"
)

