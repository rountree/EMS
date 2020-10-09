target_sources(shoc
	PRIVATE
	dp.c
	dp_mpi.c
	dp_none.c
	dp_pthreads.c
	globals.c
	main.c
	scheduler.c
)

# dp_slave.c present in directory but not part of the build process
