SET (CAMTOOL_SRCS
      assemble.c
    )
    
SET (CMAKE_CXX_FILES "-g -Wall")

ADD_EXECUTABLE ( ch_assemble ${CAMTOOL_SRCS}) 
TARGET_LINK_LIBRARIES (ch_assemble ${CAMTOOL_LIBS})

