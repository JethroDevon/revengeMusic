IF(CMAKE_VERSION VERSION_GREATER 3.6 OR CMAKE_VERSION VERSION_EQUAL 3.6)
  cmake_minimum_required (VERSION 3.6)
ELSE()
  cmake_minimum_required (VERSION 3.4)
  IF(WIN32)
    message( CMake version is less than 3.6, you must manually set revengeMusic as the default startup project for use in Visual Studio )
  ENDIF()
ENDIF()

project (revengeMusic)
set (projectVersion 1.0.0)

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

#Find Boost
find_package(Boost 1.58 COMPONENTS filesystem REQUIRED)
IF(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
ENDIF(Boost_FOUND)

IF(WIN32)
add_executable(
  revengeMusic
  src/main.cpp
  src/Windows/Pipe.cpp
  src/Sound.cpp
  src/Logger.cpp
)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "revengeMusic")
ELSEIF(UNIX)
add_executable(
  revengeMusic
  src/main.cpp
  src/Linux/Pipe.cpp
  src/Sound.cpp
  src/Logger.cpp
)

add_custom_target (
    deb
    COMMAND ${CMAKE_SOURCE_DIR}/packages/debian/build-deb.sh ${projectVersion}
)

ENDIF(WIN32)

IF(FMOD_FOUND)
  target_link_libraries(revengeMusic ${Boost_LIBRARIES} ${FMOD_LIBRARIES})
ENDIF(FMOD_FOUND)


