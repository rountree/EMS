target_sources( libemslib
	PUBLIC
	colourtable.c
	eqn_parser.cpp
	error_handlers.c
	hash.c
	hqueue.c
	istack.c
	memory.c
	moon.c
	pointsourcesink.c
	solar.c
	string_utils.c
	stringtable.c
	time.c
	tracer_utils.c
)

# Generates -D_GLIBCXX_GTHREAD_USE_WEAK=0 for this one file.
set_property( 
	SOURCE eqn_parser.cpp
	TARGET_DIRECTORY libemslib
	PROPERTY COMPILE_DEFINITIONS _GLIBCXX_GTHREAD_USE_WEAK=0
)
