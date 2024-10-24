
IF (VCPKG_TOOLCHAIN)
    FIND_PACKAGE(libxmp CONFIG REQUIRED)
    IF(TARGET libxmp::xmp_shared)
        ADD_LIBRARY(openomf::xmp ALIAS libxmp::xmp_shared)
    ELSE()
        ADD_LIBRARY(openomf::xmp ALIAS libxmp::xmp_static)
    ENDIF()

    RETURN()
ENDIF ()

SET(XMP_SEARCH_PATHS
    /usr/local/
    /usr
    /opt
)

FIND_PATH(XMP_INCLUDE_DIR xmp.h
    HINTS ${XMP_ROOT}
    PATH_SUFFIXES include
    PATHS ${XMP_SEARCH_PATHS}
)
FIND_LIBRARY(XMP_LIBRARY xmp xmp_dll
    HINTS ${XMP_ROOT}
    PATH_SUFFIXES lib64 lib
    PATHS ${XMP_SEARCH_PATHS}
)

IF(XMP_INCLUDE_DIR AND XMP_LIBRARY)
   SET(XMP_FOUND TRUE)
ENDIF()

IF(XMP_FOUND)
    MESSAGE(STATUS "Found libxmp: ${XMP_LIBRARY}")
ELSE()
    MESSAGE(WARNING "Could not find libxmp")
ENDIF()

mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY XMP_SEARCH_PATHS)

add_library(openomf::xmp UNKNOWN IMPORTED)
set_target_properties(openomf::xmp PROPERTIES
        IMPORTED_LOCATION "${XMP_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${XMP_INCLUDE_DIR}")
