target_sources(shoc
	PRIVATE
	TransferBuffer.cpp
	transfers.c
	windows.c
	zoom.c
)

# TransferBuffer.cpp should be compiled with -D_GLIBCXX_GTHREAD_USE_WEAK=0
