# temporarily set these? ehh
set(ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

if(NOT VCPKG_TOOLCHAIN)
	message(WARNING "TODO: Dependencies.cmake for non-vcpkg builds")
	# SDL2, etc..
endif()

# SDL2, SDL2_mixer
if(VCPKG_TOOLCHAIN)
	find_package(SDL2 CONFIG REQUIRED)
    add_library(openomf::SDL2 INTERFACE IMPORTED)
    target_link_libraries(openomf::SDL2 INTERFACE "$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>")
    add_library(openomf::SDL2main INTERFACE IMPORTED)
    target_link_libraries(openomf::SDL2main INTERFACE SDL2::SDL2main openomf::SDL2)

	find_package(SDL2_mixer CONFIG REQUIRED)
	add_library(openomf::SDL2_mixer INTERFACE IMPORTED)
	target_link_libraries(openomf::SDL2_mixer INTERFACE "$<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>")
else()
	find_package(SDL2 REQUIRED)
    add_library(openomf::SDL2 INTERFACE IMPORTED)
    target_link_libraries(openomf::SDL2 INTERFACE SDL2::Core)
    add_library(openomf::SDL2main INTERFACE IMPORTED)
    target_link_libraries(openomf::SDL2main INTERFACE SDL2::Main openomf::SDL2)

	find_package(SDL2_mixer REQUIRED)
	add_library(openomf::SDL2_mixer INTERFACE IMPORTED)
	target_link_libraries(openomf::SDL2_mixer INTERFACE SDL2::Mixer)
endif()

# xmp
if(VCPKG_TOOLCHAIN)
	find_package(libxmp CONFIG REQUIRED)
	add_library(openomf::xmp INTERFACE IMPORTED)
	target_link_libraries(openomf::xmp INTERFACE "$<IF:$<TARGET_EXISTS:libxmp::xmp_shared>,libxmp::xmp_shared,libxmp::xmp_static>")
else()
	find_package(xmp REQUIRED)
	add_library(openomf::xmp INTERFACE IMPORTED)
	target_link_libraries(openomf::xmp INTERFACE ${XMP_LIBRARY})
	target_include_directories(openomf::xmp INTERFACE ${XMP_INCLUDE_DIR})
endif()

# argtable
if(VCPKG_TOOLCHAIN)
	find_package(Argtable3 CONFIG REQUIRED)
	add_library(openomf::argtable INTERFACE IMPORTED)
	target_link_libraries(openomf::argtable INTERFACE argtable3::argtable3)
	target_compile_definitions(openomf::argtable INTERFACE ARGTABLE3_FOUND)
else()
	find_package(argtable2)
	find_package(Argtable3 CONFIG)

	add_library(openomf::argtable INTERFACE IMPORTED)
	if(ARGTABLE2_FOUND)
		target_link_libraries(openomf::argtable INTERFACE ${ARGTABLE2_LIBRARY})
		target_include_directories(openomf::argtable INTERFACE ${ARGTABLE2_INCLUDE_DIR})
		target_compile_definitions(openomf::argtable INTERFACE ARGTABLE2_FOUND)
	elseif(Argtable3_FOUND)
		target_link_libraries(openomf::argtable INTERFACE argtable3::argtable3)
		target_compile_definitions(openomf::argtable INTERFACE ARGTABLE3_FOUND)
	else()
		message(FATAL_ERROR "Neither argtable2 or argtable3 was found")
	endif()
endif()

# enet
if(VCPKG_TOOLCHAIN)
	find_package(unofficial-enet CONFIG REQUIRED)
	add_library(openomf::enet ALIAS unofficial::enet::enet)
else()
	find_package(enet REQUIRED)
	add_library(openomf::enet INTERFACE IMPORTED)
	target_link_libraries(openomf::enet INTERFACE ${ENET_LIBRARY})
	target_include_directories(openomf::enet INTERFACE ${ENET_INCLUDE_DIR})
endif()

# confuse
if(VCPKG_TOOLCHAIN)
	find_package(unofficial-libconfuse CONFIG REQUIRED)
	add_library(openomf::confuse ALIAS unofficial::libconfuse::libconfuse)
else()
	find_package(confuse REQUIRED)
	add_library(openomf::confuse INTERFACE IMPORTED)
	target_link_libraries(openomf::confuse INTERFACE ${CONFUSE_LIBRARY})
	target_include_directories(openomf::confuse INTERFACE ${CONFUSE_INCLUDE_DIR})
endif()

if(USE_MINIUPNPC)
	add_library(openomf::miniupnpc INTERFACE IMPORTED)
	target_compile_definitions(openomf::miniupnpc INTERFACE MINIUPNPC_FOUND)
	if(VCPKG_TOOLCHAIN)
		find_package(miniupnpc CONFIG REQUIRED)
		target_link_libraries(openomf::miniupnpc INTERFACE miniupnpc::miniupnpc)
	else()
		find_package(miniupnpc REQUIRED)
		target_link_libraries(openomf::miniupnpc INTERFACE ${MINIUPNPC_LIBRARY})
		target_include_directories(openomf::miniupnpc INTERFACE ${MINIUPNPC_INCLUDE_DIR})
	endif()
endif()

if(USE_NATPMP)
	add_library(openomf::natpmp INTERFACE IMPORTED)
	target_compile_definitions(openomf::natpmp INTERFACE NATPMP_FOUND)
	if(VCPKG_TOOLCHAIN)
		find_package(libnatpmp CONFIG REQUIRED)
		target_link_libraries(openomf::natpmp INTERFACE libnatpmp::natpmp)
	else()
		find_package(natpmp REQUIRED)
		target_link_libraries(openomf::natpmp INTERFACE ${NATPMP_LIBRARY})
		target_include_directories(openomf::natpmp INTERFACE ${NATPMP_INCLUDE_DIR})
	endif()
endif()

#---------
# dependencies that unconditionally use cmake module search
#---------
find_package(Epoxy REQUIRED)
# openomf::epoxy is defined in our FindEpoxy.cmake

find_package(PNG REQUIRED)
add_library(openomf::png INTERFACE IMPORTED)
target_link_libraries(openomf::png INTERFACE ${PNG_LIBRARIES})
target_include_directories(openomf::png INTERFACE ${PNG_INCLUDE_DIRS})

if(USE_TESTS)
    find_package(CUnit REQUIRED)
	add_library(openomf::cunit INTERFACE IMPORTED)
	target_link_libraries(openomf::cunit INTERFACE ${CUNIT_LIBRARY})
	target_include_directories(openomf::cunit INTERFACE ${CUNIT_INCLUDE_DIR})
endif()


set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
