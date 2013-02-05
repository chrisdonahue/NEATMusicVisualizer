cmake_minimum_required(VERSION 2.6.2)

project(visualizer)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

add_subdirectory(NEAT)
include(NEAT)

# AUDIO STUFF
#FIND_PACKAGE(Portaudio)
#FIND_PACKAGE(LibSndFile)

include_directories(
#    ${PORTAUDIO_INCLUDE_DIRS},
#    ${LIBSNDFILE_INCLUDE_DIR}
)

link_directories(
#    ${PORTAUDIO_LIBRARIES},
#    ${LIBSNDFILE_LIBRARY}
)

# WX STUFF
FIND_PACKAGE(wxWidgets)
INCLUDE("${wxWidgets_USE_FILE}")

add_executable(visualize
  HyperVizNoAudio.cpp
  Visualizer.cpp
)

target_link_libraries(visualize
#    ${LIBSNDFILE_LIBRARY},
	NEAT
)


#     wxWidgets_LIBRARIES
#PROJECT(Test)
#SET(wxWidgets_USE_LIBS)
#FIND_PACKAGE(wxWidgets)
#IF(wxWidgets_FOUND)
#    INCLUDE("${wxWidgets_USE_FILE}")
#    ADD_EXECUTABLE(MyTest WIN32 main.cpp main.h)
#    # and for each of your dependant executable/library targets:
#    TARGET_LINK_LIBRARIES(MyTest ${wxWidgets_LIBRARIES})
#ELSE(wxWidgets_FOUND)
### For convenience. When we cannot continue, inform the user
#    MESSAGE("wxWidgets not found!")
#ENDIF(wxWidgets_FOUND)
