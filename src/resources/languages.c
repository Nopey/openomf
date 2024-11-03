#include "resources/languages.h"
#include "formats/error.h"
#include "formats/language.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/str.h"
#include <assert.h>
#include <string.h>

static char **language_compact;
static char **language2_compact;

static char **lang_compact(sd_language *lang) {
    // calculate in-memory size of language's data
    unsigned int data_byte_count = 0;
    for(unsigned int i = 0; i < lang->count; i++) {
        char const *data = lang->strings[i].data;
        data_byte_count += (data ? strlen(data) : 0) + 1;
    }

    char **allocation = omf_malloc(lang->count * sizeof(char *) + data_byte_count);
    char *strings = (char *)(allocation + lang->count);

    // fill the allocation
    char **ptr_iter = allocation;
    char *strings_iter = strings;
    for(unsigned int i = 0; i < lang->count; i++) {
        *(ptr_iter++) = strings_iter;

        char const *data = lang->strings[i].data;
        if(data) {
            size_t data_size = strlen(data) + 1;
            memcpy(strings_iter, data, data_size);
            strings_iter += data_size;
        } else {
            strings_iter[0] = '\0';
            strings_iter += 1;
        }
    }

    assert(ptr_iter == allocation + lang->count);
    assert(strings_iter == strings + data_byte_count);

    return allocation;
}

bool lang_init(void) {
    language_compact = NULL;
    language2_compact = NULL;
    sd_language *language = NULL;
    sd_language *language2 = NULL;

    str filename_str;
    const char *dirname = pm_get_local_path(RESOURCE_PATH);
    const char *lang = settings_get()->language.language;
    str_from_format(&filename_str, "%s%s", dirname, lang);
    char const *filename = str_c(&filename_str);

    // Load up language file
    sd_language language_real;
    language = &language_real;
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language, filename)) {
        PERROR("Unable to load language file '%s'!", filename);
        goto error_0;
    }

    // OMF GERMAN.DAT and old versions of ENGLISH.DAT have only 990 strings
    unsigned int const old_language_count = 990;

    if(language->count == old_language_count) {
        // XXX this 990->1013 fixup is currently duplicated between languages.c and languagetool, but will be removed
        // from languages.c soon

        // OMF 2.1 added netplay, and with it 23 new localization strings
        unsigned new_ids[] = {149, 150, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                              182, 183, 184, 185, 267, 269, 270, 271, 284, 295, 305};
        unsigned *new_ids_end = new_ids + sizeof(new_ids) / sizeof(new_ids[0]);

        // insert dummy entries
        sd_lang_string *expanded_strings = omf_malloc(LANG_STR_COUNT * sizeof(sd_lang_string));
        unsigned next = 0;
        unsigned next_from = 0;
        for(unsigned *id = new_ids; id < new_ids_end; id++) {
            unsigned copy_count = *id - next;
            memcpy(expanded_strings + next, language->strings + next_from, copy_count * sizeof(sd_lang_string));
            next += copy_count;
            next_from += copy_count;

            expanded_strings[next].data = NULL;
            memcpy(expanded_strings[next].description, "dummy", 6);
            next++;
            language->count++;
        }
        memcpy(expanded_strings + next, language->strings + next_from,
               (LANG_STR_COUNT - next) * sizeof(sd_lang_string));
        omf_free(language->strings);
        language->strings = expanded_strings;
    }
    if(language->count != LANG_STR_COUNT) {
        PERROR("Unable to load language file '%s', unsupported or corrupt file!", filename);
        goto error_0;
    }

    INFO("Loaded language file '%s'.", filename);

    // Load up language2 file (OpenOMF)
    str_append_c(&filename_str, "2");
    filename = str_c(&filename_str);

    sd_language language2_data;
    language2 = &language2_data;
    if(sd_language_create(language2) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language2, filename)) {
        PERROR("Unable to load OpenOMF language file '%s'!", filename);
        goto error_0;
    }
    if(language2->count != LANG2_STR_COUNT) {
        PERROR("Unable to load OpenOMF language file '%s', unsupported or corrupt file!", filename);
        goto error_0;
    }

    INFO("Loaded OpenOMF language file '%s'.", filename);

    str_free(&filename_str);

    language_compact = lang_compact(language);
    language2_compact = lang_compact(language2);

    sd_language_free(language);
    sd_language_free(language2);

    return true;

error_0:
    str_free(&filename_str);
    sd_language_free(language);
    sd_language_free(language2);
    lang_close();
    return false;
}

void lang_close(void) {
    omf_free(language_compact);
    omf_free(language2_compact);
}

const char *lang_get(unsigned int id) {
    assert(id < LANG_STR_COUNT);
    return language_compact[id];
}

const char *lang_get2(unsigned int id) {
    assert(id < LANG2_STR_COUNT);
    return language2_compact[id];
}

const char *lang_get2_offset_impl(unsigned int id, unsigned int count, unsigned int offset) {
    assert(offset <= count);
    return lang_get2(id + offset);
}
