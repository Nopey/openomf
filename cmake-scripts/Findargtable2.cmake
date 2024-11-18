set(ARGTABLE2_SEARCH_PATHS
    /usr/local
    /usr
    /opt
)

find_path(ARGTABLE2_INCLUDE_DIR argtable2.h 
    HINTS
    PATH_SUFFIXES include
    PATHS ${ARGTABLE2_SEARCH_PATHS}
)
find_library(ARGTABLE2_LIBRARY argtable2 
    HINTS
    PATH_SUFFIXES lib64 lib
    PATHS ${ARGTABLE2_SEARCH_PATHS}
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments
FIND_PACKAGE_HANDLE_STANDARD_ARGS(argtable2
    REQUIRED_VARS ARGTABLE2_LIBRARY ARGTABLE2_INCLUDE_DIR
)

mark_as_advanced(ARGTABLE2_INCLUDE_DIR ARGTABLE2_LIBRARY ARGTABLE2_SEARCH_PATHS)
