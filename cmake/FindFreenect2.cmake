SET(FREENECT2_ROOT_DIR "$ENV{THIRD_PARTY_DIR}/libfreenect2")

set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	${FREENECT2_ROOT_DIR}/lib
	)

find_library(FREENECT2_LIBRARY 
	NAMES freenect2
	PATHS ${LIBRARY_PATHS}
	)
	
find_path(FREENECT2_INCLUDE_PATH libfreenect2/libfreenect2.hpp
#    PATH_SUFFIXES libfreenect
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${FREENECT2_ROOT_DIR}/include
	)
	
find_path(LIBUSB1_INCLUDE_PATH libusb-1.0/libusb.h
#    PATH_SUFFIXES libusb-1.0
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${FREENECT2_ROOT_DIR}/include
	)
	
if(FREENECT2_LIBRARY AND FREENECT2_INCLUDE_PATH)
	set(FREENECT2_FOUND TRUE)
	set(FREENECT2_INCLUDE_PATHS ${LIBUSB1_INCLUDE_PATH} ${FREENECT2_INCLUDE_PATH} CACHE STRING "The include paths needed to use freenect2")
    set(FREENECT2_LIBRARIES ${FREENECT2_LIBRARY} CACHE STRING "The libraries needed to use freenect2")
endif()

mark_as_advanced(
    FREENECT2_INCLUDE_PATHS
    FREENECT2_LIBRARIES
	)