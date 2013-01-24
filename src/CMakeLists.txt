SET (CMAKE_CXX_FILES "-g -Wall")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# Assemble utility compilation
ADD_EXECUTABLE ( assemble assemble.c ) 
TARGET_LINK_LIBRARIES (ch_assemble ${CAMTOOL_LIBS})

# Disassemble utility compilation
ADD_EXECUTABLE ( disassemble disassemble.c ) 
TARGET_LINK_LIBRARIES (disassemble ${CAMTOOL_LIBS})

