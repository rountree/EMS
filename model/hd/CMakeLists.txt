#CMakeList for EMS/model/hd
add_executable(shoc "")

target_include_directories( shoc
	PUBLIC ./include
	PUBLIC ../lib/include
	PUBLIC ../../lib/include
	PUBLIC ../../conf
)

add_subdirectory(boundary)
add_subdirectory(closure)
add_subdirectory(control)
add_subdirectory(dassim)
add_subdirectory(debug)
add_subdirectory(density)
add_subdirectory(diagnostics)
add_subdirectory(ecology)
add_subdirectory(forcings)
add_subdirectory(ginterface)
add_subdirectory(inputs)
add_subdirectory(master)
add_subdirectory(misc)
add_subdirectory(momentum)
add_subdirectory(outputs)
add_subdirectory(particles)
add_subdirectory(sediments)
add_subdirectory(slaves)
add_subdirectory(tracers)

target_link_libraries(shoc 
	libecology 
	libgrid 
	libsediments 
	libtracerstats 
	libwaves 
	libemslib 
	netcdf 
	m
	pthread
)

