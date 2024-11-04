# OMF 2097 Epic Challenge Arena
set(OMF_STR_COUNT 1013)
set(OMF_LANGS ENGLISH GERMAN)
# OpenOMF-specific
set(OPENOMF_LANGS DANISH)

set(OMF_COMMAND_WRAPPER "" CACHE STRING "Optional wrapper to run languagetool with")

macro(from_base enum_name description idx)
    set(BASETXT_CONTENTS "${BASETXT_CONTENTS}\nID: ${Lang_Count}\nTitle: ${description}\nCopyBase: ${idx}\n")

    if("${idx}" MATCHES "^([0-9]+)\\.\\.=([0-9]+)$")
        set(range_start "${CMAKE_MATCH_1}")
        math(EXPR range_end "${CMAKE_MATCH_2}")
        math(EXPR count "${range_end} - ${range_start}  + 1")

        set(LANG_ENUM "${LANG_ENUM}\n    // ${description}\n    ${enum_name} = ${Lang_Count},")
        # emit a _LAST, too
        set(LANG_ENUM "${LANG_ENUM}\n    // Number of ${enum_name} string entries\n    ${enum_name}_LAST = ${range_end},\n")
        math(EXPR Lang_Count "${Lang_Count} + ${count}")
    else()
        set(LANG_ENUM "${LANG_ENUM}\n    // ${description}\n    ${enum_name} = ${Lang_Count},\n")
        math(EXPR Lang_Count "${Lang_Count} + 1")
    endif()
endmacro()
macro(openomf_string enum_name description)
    # TODO parse "COUNT 5" from argn
    set(LANG_ENUM "${LANG_ENUM}\n    // ${description}\n    ${enum_name} = ${Lang_Count},")
    math(EXPR Lang_Count "${Lang_Count} + 1")
endmacro()

set(Lang_Count 0)
set(LANG_ENUM "")
set(BASETXT_CONTENTS "# Generated by BuildLanguages.cmake")
include("cmake-scripts/LanguageStrings.cmake")
openomf_strings()
set(LANG_ENUM "${LANG_ENUM}\n\n    // Base OMF strings start here\n")
base_strings()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/resources/generated_languages.h.in" "${CMAKE_CURRENT_BINARY_DIR}/src/resources/generated_languages.h")

set(BASE_TXT "${CMAKE_CURRENT_BINARY_DIR}/BuildLanguages_BASE.TXT")
file(WRITE "${BASE_TXT}" "${BASETXT_CONTENTS}")

message(STATUS "BuildLanguages' Lang_Count: ${Lang_Count}")



if(NOT BUILD_LANGUAGES)
    # early out, having written generated_languages.h
    return()
endif()


if(WIN32)
    set(LANGUAGE_INSTALL_PATH "openomf/resources/")
else()
    set(LANGUAGE_INSTALL_PATH "share/games/openomf/")
endif()

# generate custom target info
set(BUILD_LANG_COMMANDS)
set(BUILD_LANG_SOURCES)
foreach(LANG ${OMF_LANGS})
    set(TXT2 "${PROJECT_SOURCE_DIR}/resources/${LANG}2.TXT")
    set(LNG "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.LNG")
    find_file("${LANG}_DAT" "${LANG}.DAT" PATHS "${CMAKE_CURRENT_BINARY_DIR}/resources" "${PROJECT_SOURCE_DIR}/resources" REQUIRED)
    set(BASE_DAT "${${LANG}_DAT}")
    list(APPEND BUILD_LANG_SOURCES "${TXT2}")
    list(APPEND BUILD_LANG_COMMANDS
        DEPENDS "${TXT2}"
        BYPRODUCTS "${LNG}"
        COMMAND ${CMAKE_COMMAND} -E echo_append "${LANG}, "
        COMMAND ${OMF_COMMAND_WRAPPER} "$<TARGET_FILE:languagetool>" --import "${TXT2}" --import "${BASE_TXT}" --base "${BASE_DAT}" --base-count "${OMF_STR_COUNT}" --output "${LNG}" --check-count ${Lang_Count}
    )
    install(FILES "${LNG}" DESTINATION "${LANGUAGE_INSTALL_PATH}")
endforeach()
foreach(LANG ${OPENOMF_LANGS})
    set(TXT "${PROJECT_SOURCE_DIR}/resources/${LANG}.TXT")
    set(TXT2 "${PROJECT_SOURCE_DIR}/resources/${LANG}2.TXT")
    set(DAT "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.DAT")
    set(LNG "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.LNG")
    list(APPEND BUILD_LANG_SOURCES "${TXT}" "${TXT2}")
    list(APPEND BUILD_LANG_COMMANDS
        DEPENDS "${TXT}" "${TXT2}"
        BYPRODUCTS "${DAT}" "{LNG}"
        COMMAND ${CMAKE_COMMAND} -E echo_append "${LANG}, "
        COMMAND ${OMF_COMMAND_WRAPPER} "$<TARGET_FILE:languagetool>" --import "${TXT}" --output "${DAT}" --check-count ${OMF_STR_COUNT}
        # XXX HACK: Using DANISH.TXT as base of DANISH2 until we merge the two.
        COMMAND ${OMF_COMMAND_WRAPPER} "$<TARGET_FILE:languagetool>" --import "${TXT2}" --import "${BASE_TXT}" --base "${DAT}"  --base-count "${OMF_STR_COUNT}" --output "${LNG}" --check-count ${Lang_Count}
    )
    install(FILES "${LNG}" DESTINATION "${LANGUAGE_INSTALL_PATH}")
endforeach()



add_custom_target(build_languages
    COMMAND ${CMAKE_COMMAND} -E echo_append "Building Languages... "
    ${BUILD_LANG_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E echo "done"
)
target_sources(build_languages PRIVATE ${BUILD_LANG_SOURCES})
add_dependencies(openomf build_languages)
add_dependencies(build_languages languagetool)
