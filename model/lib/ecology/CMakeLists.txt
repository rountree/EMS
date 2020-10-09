# EMS/model/lib/ecology/process_library

add_library(libecology STATIC "")

add_subdirectory(process_library)
target_sources(libecology
	PRIVATE
	allprocesses.c
	bio_opt.c
	cell.c
	column.c
	constants.c
	ecofunct.c
	ecology.c
	eprocess.c
	externallibs.c
	grow_data.c
	parameter_info.c
	parameter_defaults.c
	process_defaults.c
	utils.c
)

target_include_directories(libecology
	PUBLIC ./include
	PUBLIC ../../../lib/include
)

