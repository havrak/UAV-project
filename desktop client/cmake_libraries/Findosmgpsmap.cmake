find_path(osmgpsmap_INCLUDE_DIR
	NAMES osm-gps-map.h
	# PATH_SUFFIXES /osmgpsmap-1.0
	)
find_library(osmgpsmap_LIBRARY
	NAMES osmgpsmap-1.0
	)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(osmgpsmap
	osmgpsmap_LIBRARY
	osmgpsmap_INCLUDE_DIR
	)
