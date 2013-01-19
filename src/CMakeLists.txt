SET (CMAKE_CXX_FILES "-g -Wall")

ADD_EXECUTABLE ( assemble assemble.c ) 
TARGET_LINK_LIBRARIES (ch_assemble ${CAMTOOL_LIBS})

ADD_EXECUTABLE ( disassemble disassemble.c ) 
TARGET_LINK_LIBRARIES (disassemble ${CAMTOOL_LIBS})

