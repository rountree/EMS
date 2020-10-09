# CMakeList for EMS/model/lib/waves

add_library(libwaves STATIC "")

target_sources(libwaves
	PRIVATE
	waves.c
)

target_include_directories(libwaves
	PUBLIC ./include
	PUBLIC ../../../lib/include 
)

