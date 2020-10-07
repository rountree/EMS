add_library(libemslib STATIC "")

target_sources(libemslib
	PRIVATE
		ems.c
)

target_include_directories(libemslib
	PUBLIC
		./include
)

add_subdirectory(./geodesy)
add_subdirectory(./geom)
add_subdirectory(./grid)
add_subdirectory(./io)
add_subdirectory(./math)
add_subdirectory(./misc)

