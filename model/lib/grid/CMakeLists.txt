# CMakeList for EMS/model/lib/grid

add_library(libgrid STATIC "")

add_subdirectory(./nn)
add_subdirectory(./csa)
add_subdirectory(./gridmap)
add_subdirectory(./delaunay)
add_subdirectory(./misc)
add_subdirectory(./lsq)
add_subdirectory(./lsq_lin)
add_subdirectory(./lsq_quad)
add_subdirectory(./linear)

target_sources(libgrid
	PRIVATE
	grid_entry.c
)

target_include_directories(libgrid
	PUBLIC ./include
	PUBLIC ../../../lib/include
)

