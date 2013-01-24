SET (CMAKE_CXX_FILES "-g -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# Common parts compilation
ADD_LIBRARY (common common.c)
SET(CAMTOOL_LIBS common)

# Assemble utility compilation
ADD_EXECUTABLE ( assemble assemble.c ) 
TARGET_LINK_LIBRARIES (assemble ${CAMTOOL_LIBS})

# Disassemble utility compilation
ADD_EXECUTABLE ( disassemble disassemble.c ) 
TARGET_LINK_LIBRARIES (disassemble ${CAMTOOL_LIBS})

