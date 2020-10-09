# CMakeList for EMS/model/lib/sediments

add_library(libsediments STATIC "")

target_sources(libsediments
	PRIVATE
	bbl.c
	hd2sed.c
	sed2hd.c
	sed_init.c
	trvdiffsettl.c
	vtransp.c
)

target_include_directories(libsediments
	PUBLIC ./include
	PUBLIC ../../../lib/include
)

