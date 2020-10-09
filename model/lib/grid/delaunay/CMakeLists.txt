# CMakeList for /EMS/model/lib/grid/delaunay

target_sources( libgrid
	PRIVATE
	delaunay.c
	triangle.c
)

set_property(
	SOURCE triangle.c
	TARGET_DIRECTORY libgrid
	PROPERTY COMPILE_DEFINITIONS TRILIBRARY
)

