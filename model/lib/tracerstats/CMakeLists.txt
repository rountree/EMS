# CMakeList for EMS/model/lib/tracerstats

add_library(libtracerstats STATIC "")

target_sources(libtracerstats
	PRIVATE
	statistics.c
	tracerstats.c
)

target_include_directories(libtracerstats
	PUBLIC ./include
	PUBLIC ../../../lib/include
)

