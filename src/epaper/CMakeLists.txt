set(srcs
        src/core/epaper_spi.c
        src/font/epaper_font_19x35_en.c
        src/font/epaper_utf8_to_gbk.c
        src/font/epaper_font_36x35_cn.c
        src/picture/epaper_picture.c
        src/screen/epaper_bw_213.c
        src/screen/epaper.c)

set(includes inc src/internal_inc)
set(priv_includes inc src/internal_inc)

idf_component_register(SRCS "${srcs}"
        INCLUDE_DIRS "${includes}"
        PRIV_INCLUDE_DIRS "${priv_includes}"
        REQUIRES bt app_update)
target_include_directories()