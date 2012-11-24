#cmake_minimum_required(VERSION 2.4)

#PROJECT( neat )

FILE(GLOB neat_sources *.cpp)

add_library(NEAT
    ${neat_sources}
)


#ADD_EXECUTABLE( neat ${neat_sources} )
