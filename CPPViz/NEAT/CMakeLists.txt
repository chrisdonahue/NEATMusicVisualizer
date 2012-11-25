#cmake_minimum_required(VERSION 2.4)
cmake_minimum_required(VERSION 2.6.2)

#PROJECT( neat )

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
SET(wxWidgets_USE_LIBS)

FIND_PACKAGE(wxWidgets)
IF(wxWidgets_FOUND)
    INCLUDE("${wxWidgets_USE_FILE}")
ELSE(wxWidgets_FOUND)
    MESSAGE("wxWidgets not found!")
ENDIF(wxWidgets_FOUND)

FILE(GLOB neat_sources *.cpp)

add_library(NEAT
    ${neat_sources}
)

target_link_libraries(NEAT
    ${wxWidgets_LIBRARIES}
)

#ADD_EXECUTABLE( neat ${neat_sources} )
