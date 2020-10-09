# cmake build file for EMS
#
# Barry Rountree
# rountree@llnl.gov

#
cmake_minimum_required(VERSION 3.18.4 FATAL_ERROR)

set(CMP0048=NEW) # Allow version information in the project stanza
project(EMS
	VERSION 0.0.0.0
	DESCRIPTION "Environmental Modeling Suite"
	HOMEPAGE_URL https://research.csiro.au/cem/software/ems/
	LANGUAGES C CXX)

add_subdirectory(./lib)
add_subdirectory(./model/hd)	# shoc
add_subdirectory(./model/lib)	# compas
