#cmake_minimum_required(VERSION 2.4.6)
cmake_minimum_required(VERSION 2.6.2)

project(visualizer)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

# PORTAUDIO STUFF
FIND_PACKAGE(Portaudio)
# LIBSNDFILE STUFF
FIND_PACKAGE(Sndfile)
#PKG_CHECK_MODULES(SNDFILE REQUIRED sndfile>=1.0.11)

include_directories(
    ${PORTAUDIO_INCLUDE_DIR},
    ${SNDFILE_INCLUDE_DIRS}
)

link_directories(
	NEAT,
    ${SNDFILE_LIBRARY_DIRS}
)

add_subdirectory(NEAT)

# WX STUFF
SET(wxWidgets_USE_LIBS)
FIND_PACKAGE(wxWidgets)
IF(wxWidgets_FOUND)
    INCLUDE("${wxWidgets_USE_FILE}")
    INCLUDE("NEAT")
#    ADD_EXECUTABLE(MyTest WIN32 main.cpp main.h)
    # and for each of your dependant executable/library targets:
#    TARGET_LINK_LIBRARIES(MyTest ${wxWidgets_LIBRARIES})
ELSE(wxWidgets_FOUND)
## For convenience. When we cannot continue, inform the user
    MESSAGE("wxWidgets not found!")
ENDIF(wxWidgets_FOUND)

add_executable(visualize
  HyperViz.cpp
  Visualizer.cpp
)

target_link_libraries(visualize
    ${PORTAUDIO_LIBRARIES},
    ${SNDFILE_LIBRARIES},
	NEAT
#    ${wxWidgets_LIBRARIES},
#    sndfile,
#    portaudio
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




#IF (BUILD_CONDOR_EXE)
#    SET (CONDOR_COMPILE "condor_compile  ")
#    SET (CMAKE_CXX_COMPILER "${CONDOR_COMPILE}${CMAKE_CXX_COMPILER}")
#ENDIF (BUILD_CONDOR_EXE)

#include_directories(
#rl_agent/include
#ale/src
#)

   
##---------------------------------------------------
## Please set your wxWidgets configuration here
##--------------------------------------------------- 
     
# Here you can define what libraries of wxWidgets you need for your
# application. You can figure out what libraries you need here;
# http://www.wxwidgets.org/manuals/2.8/wx_librarieslist.html
# We need the Find package for wxWidgets to work
# NOTE: if you're using aui, include aui in this required components list.
      
# It was noticed that when using MinGW gcc it is essential that 'core' is mentioned before 'base'.
       
##---------------------------------------------------
## Actual config file starts here
##--------------------------------------------------- 
        
# wxWidgets include (this will do all the magic to configure everything)
         
# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out
#set(SRCS minimal.cpp)
          
# If we build for windows systems, we also include the resource file
# containing the manifest, icon and other resources
#if(WIN32)
 #   set(SRCS ${SRCS} minimal.rc)
#endif(WIN32)
                 
# Here we define the executable minimal.exe or minimal on other systems
# the above paths and defines will be used in this build
#add_executable(minimal WIN32 ${SRCS})
                  
# We add to our target 'minimal' the wxWidgets libraries. These are
# set for us by the find script. If you need other libraries, you
# can add them here as well. 



#find_package(wxWidgets COMPONENTS core base REQUIRED)
#include( "${wxWidgets_USE_FILE}" )


