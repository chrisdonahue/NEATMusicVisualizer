cmake_minimum_required(VERSION 2.4.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

#IF (BUILD_CONDOR_EXE)
#    SET (CONDOR_COMPILE "condor_compile  ")
#    SET (CMAKE_CXX_COMPILER "${CONDOR_COMPILE}${CMAKE_CXX_COMPILER}")
#ENDIF (BUILD_CONDOR_EXE)

include_directories(
rl_common/include
rl_env/include
rl_agent/include
ale/src
)

link_directories(
	ale
)

add_subdirectory(rl_common)
add_subdirectory(rl_agent)
add_subdirectory(rl_env)

add_executable(experiment
  src/rl.cc
)

target_link_libraries(experiment
	commonlib
)
