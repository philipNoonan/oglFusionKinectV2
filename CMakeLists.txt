cmake_minimum_required(VERSION 3.9)


project(oglFusion)


# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()



set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(BasicConfig)

#include(${THIRD_PARTY_DIR}/dlib-18.18/dlib/cmake)
#freeglut
set(THIRD_PARTY_DIR $ENV{THIRD_PARTY_DIR})
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${THIRD_PARTY_DIR}/freeglut/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${THIRD_PARTY_DIR}/freeglut/lib)
#glew
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${THIRD_PARTY_DIR}/glew/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${THIRD_PARTY_DIR}/glew/lib)
#PCL

#find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freenect2 REQUIRED)
find_package(Aruco REQUIRED)
find_package(GLM REQUIRED)
find_package(Eigen3 REQUIRED)

add_definitions(-DEIGEN_USE_NEW_STDVECTOR
                -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET)#find_package(REALSENSE2 REQUIRED)



file(GLOB_RECURSE SRC "src/*")
file(GLOB_RECURSE GFX "graphics/*")
file(GLOB_RECURSE SHD "shaders/*")
file(GLOB_RECURSE HDR "include/*")
file(GLOB_RECURSE RES "resources/*")


list(APPEND SRC ${SHD})
list(APPEND SRC ${GFX})
list(APPEND SRC ${HDR})
list(APPEND SRC ${RES})


#imgui include - not currently on the git 
file(TO_CMAKE_PATH $ENV{IMGUI_DIR} IMGUI_DIR) # this is to make windows path slashes happy, otherwise a invalid escape error is made
file(GLOB imGUI ${IMGUI_DIR}/*.cpp)
list(APPEND SRC ${imGUI})



add_executable(${PROJECT_NAME} ${SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG "${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX}")

#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
#ADD_DEFINITIONS(-DUNICODE)
#ADD_DEFINITIONS(-D_UNICODE)

target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PUBLIC "graphics")

target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
#target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${FREENECT2_INCLUDE_PATH})
target_include_directories(${PROJECT_NAME} PUBLIC ${ARUCO_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLUT_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${EIGEN3_INCLUDE_DIR})

#target_include_directories(${PROJECT_NAME} PUBLIC ${REALSENSE2_INCLUDE_PATHS})

#target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${FREENECT2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLEW_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${ARUCO_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} PUBLIC ${REALSENSE2_LIBRARIES})

#target_link_libraries(${PROJECT_NAME} PUBLIC ${PCL_COMMON_LIBRARIES}) #this includes a lot of vtk libs

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/shaders/
                ${PROJECT_BINARY_DIR}/bin/$(Configuration)/shaders/)
				
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/shaders/
                ${PROJECT_BINARY_DIR}/shaders/)
				
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/resources/
                ${PROJECT_BINARY_DIR}/bin/$(Configuration)/resources/)
				
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/resources/
                ${PROJECT_BINARY_DIR}/resources/)


