SET(REALSENSE2_ROOT_DIR "$ENV{THIRD_PARTY_DIR}/librealsense2")

set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	${REALSENSE2_ROOT_DIR}/lib
	)

find_library(REALSENSE2_LIBRARY 
	NAMES realsense2
	PATHS ${LIBRARY_PATHS}
	)
	
find_path(REALSENSE2_INCLUDE_PATH librealsense2/rs.hpp
#    PATH_SUFFIXES librealsense2
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${REALSENSE2_ROOT_DIR}/include
	)
		
if(REALSENSE2_LIBRARY AND REALSENSE2_INCLUDE_PATH)
	set(REALSENSE2_FOUND TRUE)
	set(REALSENSE2_INCLUDE_PATHS ${REALSENSE2_INCLUDE_PATH} CACHE STRING "The include paths needed to use realsense2")
    set(REALSENSE2_LIBRARIES ${REALSENSE2_LIBRARY} CACHE STRING "The libraries needed to use realsense2")
endif()

mark_as_advanced(
    REALSENSE2_INCLUDE_PATHS
    REALSENSE2_LIBRARIES
	)