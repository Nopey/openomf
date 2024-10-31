if(VCPKG_TOOLCHAIN)
  if(TARGET miniupnpc::miniupnpc)
    return()
  endif()
  find_package(miniupnpc CONFIG)
  if(TARGET miniupnpc::miniupnpc)
    MESSAGE(STATUS "Found miniupnp in vcpkg")
    set(MINIUPNPC_LIBRARY miniupnpc::miniupnpc)
    SET(MINIUPNPC_FOUND TRUE)
    set(MINIUPNPC_INCLUDE_DIR)
    return()
  endif()
endif(VCPKG_TOOLCHAIN)

SET(MINIUPNPC_SEARCH_PATHS
    /usr/local/
    /usr
    /opt
)

FIND_PATH(MINIUPNPC_INCLUDE_DIR miniupnpc/miniupnpc.h
    HINTS
    PATH_SUFFIXES include
    PATHS ${MINIUPNPC_SEARCH_PATHS}
)
FIND_LIBRARY(MINIUPNPC_LIBRARY miniupnpc
    HINTS
    PATH_SUFFIXES lib64 lib bin
    PATHS ${MINIUPNPC_SEARCH_PATHS}
)

IF (MINIUPNPC_INCLUDE_DIR AND MINIUPNPC_LIBRARY)
    SET(MINIUPNPC_FOUND TRUE)
ENDIF (MINIUPNPC_INCLUDE_DIR AND MINIUPNPC_LIBRARY)

IF (MINIUPNPC_FOUND)
    MESSAGE(STATUS "Found libminiupnpc: ${MINIUPNPC_LIBRARY}")
ELSE (MINIUPNPC_FOUND)
    MESSAGE(WARNING "Could not find libminiupnpc")
ENDIF (MINIUPNPC_FOUND)

mark_as_advanced(MINIUPNPC_INCLUDE_DIR MINIUPNPC_LIBRARY MINIUPNPC_SEARCH_PATHS)
