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

/*! \brief OMF 2097 String ID
 *
 * These string IDs match OMFv2.1 (Epic Challenge Arena)
 */
enum
{
    // there are 10 HARs
    LANG_STR_HAR = 31,
    LANG_STR_NEWSROOM_NEWCHAMPION = 79,
    // there are 3*2 pronouns
    LANG_STR_PRONOUN = 81,
    // there are 24*2 newsroom texts
    LANG_STR_NEWSROOM_TEXT = 87,

    LANG_STR_COUNT = 1013,
};

enum
{
    // OMF versions before 2.1 shipped 990 strings.
    // (GERMAN.DAT, and old ENGLISH.DAT have this many strings)
    OLD_LANG_STR_COUNT = 990,
};

// Gets an OMF 2097 localization string
const char *lang_get(unsigned int id);
// Gets an openomf localization string
const char *lang_get2(unsigned int id);

#endif // LANGUAGES_H
