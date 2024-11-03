#ifndef LANGUAGES_H
#define LANGUAGES_H

#include "resources/generated_languages.h" // for LANG2_STR enum
#include <stdbool.h>

/*
 * This file should handle loading language file(s)
 * and support getting text. Maybe some function to rendering text index on
 * texture with the help of text.h ?
 */

bool lang_init(void);
void lang_close(void);

enum
{
    // OMFv2.1 (Epic Challenge Arena) ships 1013 strings.
    // (latest official ENGLISH.DAT has this many strings)
    LANG_STR_COUNT = 1013,

    // OMF versions before 2.1 shipped 990 strings.
    // (GERMAN.DAT, and old ENGLISH.DAT have this many strings)
    OLD_LANG_STR_COUNT = 990,
};

// Gets an OMF 2097 localization string
const char *lang_get(unsigned int id);
// Gets an openomf localization string
const char *lang_get2(unsigned int id);

// Gets an openomf localization string (from one of the tables)
#define lang_get2_offset(LANG2_STR, offset) lang_get2_offset_impl(LANG2_STR, LANG2_STR##_COUNT, (offset))
const char *lang_get2_offset_impl(unsigned int id, unsigned int count, unsigned int offset);

#endif // LANGUAGES_H
