cmake_minimum_required(VERSION 3.9)
# Toolchain file needs to be set before project() 
set(CMAKE_TOOLCHAIN_FILE "C:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(oglFusionNM)


if (WIN32)

if ("${CMAKE_VERSION}" VERSION_GREATER 3.6.3)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ("${CMAKE_VERSION}" VERSION_GREATER 3.6.3)
  
endif()

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# c++17 required for filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(BasicConfig)


# OpenCV
#set(OpenCV_DIR "${THIRD_PARTY_DIR}/opencv")

find_package(freenect2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(eigen3 REQUIRED)
find_package(tinyxml2 REQUIRED)

# Header only
find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp)

#find_package(OpenCV REQUIRED)

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


### COPY SHADER FILES TO BUILD AND BINARY LOCATIONS IF THEY HAVE BEEN CHANGED ###
add_custom_target(
  shadersNM ALL
  DEPENDS ${SHD}
  COMMENT "Copying shader files"
  VERBATIM
)
foreach(ShaderFile ${SHD})
get_filename_component(ShaderName ${ShaderFile} NAME)
	add_custom_command(
		TARGET shadersNM
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ShaderFile}
			${PROJECT_BINARY_DIR}/bin/$(Configuration)/shaders/${ShaderName}
	)
		add_custom_command(
		TARGET shadersNM
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ShaderFile}
			${PROJECT_BINARY_DIR}/shaders/${ShaderName}
	)
endforeach()

### COPY RESOURCES FILES TO BUILD AND BINARY LOCATIONS IF THEY HAVE BEEN CHANGED ###
add_custom_target(
  resourcesNM ALL
  DEPENDS ${RES}
  COMMENT "Copying resources files"
  VERBATIM
)
foreach(ResourceFile ${RES})
get_filename_component(ResourceName ${ResourceFile} NAME)
	add_custom_command(
		TARGET resourcesNM
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ResourceFile}
			${PROJECT_BINARY_DIR}/bin/$(Configuration)/resources/${ResourceName}
	)
		add_custom_command(
		TARGET resourcesNM
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ResourceFile}
			${PROJECT_BINARY_DIR}/resources/${ResourceName}
	)
endforeach()

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data/meshes)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data/motion)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/bin/debug/data/meshes)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/bin/release/data/motion)


add_executable(${PROJECT_NAME} ${SRC})
add_dependencies(${PROJECT_NAME} shadersNM resourcesNM)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG "${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX}")

#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
#ADD_DEFINITIONS(-DUNICODE)
#ADD_DEFINITIONS(-D_UNICODE)

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PUBLIC "graphics")

#target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC ${freenect2_INCLUDE_DIRS})


if (WIN32)
	target_link_libraries(${PROJECT_NAME} debug ${freenect2_DEBUG_LIBRARY})
	target_link_libraries(${PROJECT_NAME} optimized ${freenect2_LIBRARY})
else()
	target_link_libraries(${PROJECT_NAME} ${FREENECT2_LIBRARIES})
endif()
#target_link_libraries(${PROJECT_NAME} ${freenect2_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} imgui::imgui)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)
target_link_libraries(${PROJECT_NAME} tinyxml2)

#Header only
include_directories(${NLOHMANN_INCLUDE_DIR})
