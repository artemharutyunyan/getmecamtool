SET (CMAKE_CXX_FILES "-g -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# Common parts compilation
ADD_LIBRARY (common common.c)
SET(CAMTOOL_LIBS common)

# Assemble utility compilation
ADD_EXECUTABLE ( assemble assemble.c ) 
TARGET_LINK_LIBRARIES (assemble ${CAMTOOL_LIBS})

# WebUI extracting utility compilation
ADD_EXECUTABLE ( uiextract uiextract.c ) 
TARGET_LINK_LIBRARIES (uiextract ${CAMTOOL_LIBS})

# System image extracting utility compilation
ADD_EXECUTABLE ( sysextract sysextract.c ) 
TARGET_LINK_LIBRARIES (sysextract ${CAMTOOL_LIBS})


