
SET(CUNIT_SEARCH_PATHS
    /usr/local/
    /usr
    /opt
)

FIND_PATH(CUNIT_INCLUDE_DIR CUnit/CUnit.h
    HINTS
    PATH_SUFFIXES include
    PATHS ${CUNIT_SEARCH_PATHS}
)
FIND_LIBRARY(CUNIT_LIBRARY cunit
    HINTS
    PATH_SUFFIXES lib64 lib bin
    PATHS ${CUNIT_SEARCH_PATHS}
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CUnit
    REQUIRED_VARS CUNIT_LIBRARY CUNIT_INCLUDE_DIR
)

mark_as_advanced(CUNIT_INCLUDE_DIR CUNIT_LIBRARY CUNIT_SEARCH_PATHS)
