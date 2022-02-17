find_path (BCM2835_INCLUDE_DIR bcm2835.h )
find_library (BCM2835_LIBRARY NAMES bcm2835)

set (BCM2835_LIBRARIES ${BCM2835_LIBRARY} )
set (BCM2835_INCLUDE_DIRS ${BCM2835_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set BCM2835_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(bcm2835 DEFAULT_MSG
                                  BCM2835_LIBRARY BCM2835_INCLUDE_DIR)
# find_package_handle_standard_args(BCM2835 DEFAULT_MSG
#  BCM2835_LIBRARY BCM2835_INCLUDE_DIR)

mark_as_advanced (BCM2835_INCLUDE_DIR BCM2835_LIBRARY )
