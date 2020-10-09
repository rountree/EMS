target_sources( libemslib
	PRIVATE
	coast.c
	datafile.c
	dfcoords.c
	dfeval.c
	emslogger.c
	nc_buffered.cpp
	ncw.c
	poly_coast.c
	prmfile.c
	ptrack.c
	tf_nc_cmr.c
	tf_nc_cmr_tiled.c
	tf_xyz_cmr.c
	tiled_coast.c
	timeseries.c
	topo.c
	topo_factory.c
	topo_tile_misc.c
	ts_serial.cpp
)

set_property(
	SOURCE nc_buffered.cpp
	TARGET_DIRECTORY libemslib
	PROPERTY COMPILE_DEFINITIONS _GLIBCXX_GTHREAD_USE_WEAK=0
)
