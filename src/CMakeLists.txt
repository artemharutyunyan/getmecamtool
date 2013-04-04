SET (CMAKE_CXX_FILES "-g -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantici -Wformat")

# Common parts compilation
ADD_LIBRARY (common common.c)
SET(CAMTOOL_LIBS common)

# UI Packing utility compilation
ADD_EXECUTABLE ( uipack uipack.c ) 
TARGET_LINK_LIBRARIES (uipack ${CAMTOOL_LIBS})

# UI Packing utility compilation
ADD_EXECUTABLE ( syspack syspack.c ) 
TARGET_LINK_LIBRARIES (syspack ${CAMTOOL_LIBS})

# WebUI extracting utility compilation
ADD_EXECUTABLE ( uiextract uiextract.c ) 
TARGET_LINK_LIBRARIES (uiextract ${CAMTOOL_LIBS})

# System image extracting utility compilation
ADD_EXECUTABLE ( sysextract sysextract.c ) 
TARGET_LINK_LIBRARIES (sysextract ${CAMTOOL_LIBS})

# Confiuration extracting utility compilation
ADD_EXECUTABLE ( confextract confextract.c ) 
TARGET_LINK_LIBRARIES (confextract ${CAMTOOL_LIBS})

# Confiuration packing utility compilation
ADD_EXECUTABLE ( confpack confpack.c ) 
TARGET_LINK_LIBRARIES (confpack ${CAMTOOL_LIBS})

