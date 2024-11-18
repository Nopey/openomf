# temporarily set these? ehh
set(ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

if(NOT VCPKG_TOOLCHAIN)
	message(WARNING "TODO: Dependencies.cmake for non-vcpkg builds")
	# SDL2, etc..
endif()

#---------
# dependencies that use find_package CONFIG search in vcpkg
#---------
if(VCPKG_TOOLCHAIN)
	find_package(SDL2 CONFIG REQUIRED)
    add_library(openomf::SDL2 INTERFACE IMPORTED)
    target_link_libraries(openomf::SDL2 INTERFACE "$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>")
    add_library(openomf::SDL2main INTERFACE IMPORTED)
    target_link_libraries(openomf::SDL2main INTERFACE SDL2::SDL2main openomf::SDL2)

	find_package(SDL2_mixer CONFIG REQUIRED)
	add_library(openomf::SDL2_mixer INTERFACE IMPORTED)
	target_link_libraries(openomf::SDL2_mixer INTERFACE "$<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>")

	find_package(libxmp CONFIG REQUIRED)
	add_library(openomf::xmp INTERFACE IMPORTED)
	target_link_libraries(openomf::xmp INTERFACE "$<IF:$<TARGET_EXISTS:libxmp::xmp_shared>,libxmp::xmp_shared,libxmp::xmp_static>")

	find_package(Argtable3 CONFIG REQUIRED)
	add_library(openomf::argtable INTERFACE IMPORTED)
	target_link_libraries(openomf::argtable INTERFACE argtable3::argtable3)
	target_compile_definitions(openomf::argtable INTERFACE ARGTABLE3_FOUND)

	find_package(unofficial-enet CONFIG REQUIRED)
	add_library(openomf::enet ALIAS unofficial::enet::enet)
	
	find_package(unofficial-libconfuse CONFIG REQUIRED)
	add_library(openomf::confuse ALIAS unofficial::libconfuse::libconfuse)

	if(USE_MINIUPNPC)
		find_package(miniupnpc CONFIG REQUIRED)
		add_library(openomf::miniupnpc INTERFACE IMPORTED)
		target_compile_definitions(openomf::miniupnpc INTERFACE MINIUPNPC_FOUND)
		target_link_libraries(openomf::miniupnpc INTERFACE miniupnpc::miniupnpc)
	endif()
	if(USE_NATPMP)
		find_package(natpmp CONFIG REQUIRED)
		message(FATAL_ERROR "TODO")
	endif()
else()
	find_package(enet REQUIRED)
	find_package(confuse REQUIRED)
endif()

# TODO: miniupnpc
# if(MINIUPNPC_FOUND)
#     set(CORELIBS ${CORELIBS} ${MINIUPNPC_LIBRARY})
#     set(COREINCS ${COREINCS} ${MINIUPNPC_INCLUDE_DIR})
#     add_compile_definitions(MINIUPNPC_FOUND)
# endif()
#
# if(LIBNATPMP_FOUND)
#     set(CORELIBS ${CORELIBS} ${LIBNATPMP_LIBRARY})
#     set(COREINCS ${COREINCS} ${LIBNATPMP_INCLUDE_DIR})
#     add_compile_definitions(NATPMP_FOUND)
# endif()

#---------
# dependencies that unconditionally use cmake module search
#---------
find_package(Epoxy REQUIRED)
# openomf::epoxy is defined in our FindEpoxy.cmake

find_package(PNG REQUIRED)
add_library(openomf::png INTERFACE IMPORTED)
target_link_libraries(openomf::png INTERFACE ${PNG_LIBRARIES})
target_include_directories(openomf::png INTERFACE ${PNG_INCLUDE_DIRS})


# If tests are enabled, find CUnit
if(USE_TESTS)
    find_package(CUnit REQUIRED)
	add_library(openomf::cunit INTERFACE IMPORTED)
	target_link_libraries(openomf::cunit INTERFACE ${CUNIT_LIBRARY})
	target_include_directories(openomf::cunit INTERFACE ${CUNIT_INCLUDE_DIR})
endif()


set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
