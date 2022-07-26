# Good introduction CMAKE: https://medium.com/@onur.dundar1/cmake-tutorial-585dd180109b

# --- Location of VTK build directory (set(DVTK_DIR <PATH/TO/VTK/BUILD>) ---
set(VTK_DIR ~/dev/vtk/build)

cmake_minimum_required(VERSION 3.18.4)
project("trt")

# --- Set Flags ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -g") # All compiler warnings, enable debug output
set(CMAKE_EXE_LINKER_FLAGS "-g") # Linker debug output

# --- Set build directory ---
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")

# Get all TRT files excluding main
file(GLOB SRC_FILES LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM SRC_FILES ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Sources necessary to compile main
set(MAIN_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp ${SRC_FILES})

# --- Include VTK ---
# (source: https://kitware.github.io/vtk-examples/site/Cxx/IO/DumpXMLFile/)
find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  CommonDataModel
  FiltersCore
  FiltersSources
  IOLegacy
  IOXML
  InteractionStyle
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  QUIET
)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "trt: Unable to find the VTK build folder. Ensure its location is set correctly in the VTK_DIR variable within the CMakeLists.txt file, line 3.")
endif()
# ---- End Include VTK ---

# ---- Include GSL ---
find_package(GSL REQUIRED)
# ---- Include GSL ---
 
message("${CMAKE_BINARY_DIR}")
message("${CMAKE_SOURCE_DIR}")
message("${CMAKE_INCLUDE_PATH}")
message("${LIBRARY_OUTPUT_PATH}")
message("${MAIN_SOURCES}")

include_directories("${CMAKE_SOURCE_DIR}/src")

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")

# MAIN TARGET
add_executable(main ${MAIN_SOURCES})
set(TARGETSNAMES main)
target_link_libraries(main PRIVATE ${VTK_LIBRARIES} ${GSL_LIBRARIES})

# TEST TARGETS (src: https://stackoverflow.com/questions/14306642/adding-multiple-executables-in-cmake)
file(GLOB TEST_SOURCE_FILES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

foreach(SOURCE_PATH ${TEST_SOURCE_FILES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_PATH} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${SOURCE_PATH} ${SRC_FILES})
    set(TARGETSNAMES ${TARGETSNAMES} ${EXECUTABLE_NAME})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${VTK_LIBRARIES} ${GSL_LIBRARIES})
endforeach(SOURCE_PATH ${SOURCE_FILES})

# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS main
  MODULES ${VTK_LIBRARIES}
)
