cmake_minimum_required (VERSION 3.6)
project (revengeMusic)

include_directories("${CMAKE_SOURCE_DIR}/include/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
IF(NOT WIN32)
  SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++11")
ENDIF(NOT WIN32)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

#Find FMOD
find_package(FMOD)
IF(FMOD_FOUND)
  include_directories(${FMOD_INCLUDE_DIRS})
ELSEIF(NOT WIN32)
  include_directories(${PROJECT_BINARY_DIR}/fmod/api/lowlevel/inc)
ENDIF(FMOD_FOUND)

IF(WIN32)
add_executable(
  revengeMusic
  src/main.cpp
  src/Windows/Pipe.cpp
  src/Sound.cpp
  src/SysError.cpp
)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "revengeMusic")
ELSEIF(UNIX)
add_executable(
  revengeMusic
  src/main.cpp
  src/Linux/Pipe.cpp
  src/Sound.cpp
  src/SysError.cpp
)
ENDIF(WIN32)

IF(FMOD_FOUND)
  target_link_libraries(revengeMusic ${FMOD_LIBRARIES})
ENDIF(FMOD_FOUND)
