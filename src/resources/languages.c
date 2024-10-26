#include "resources/languages.h"
#include "formats/error.h"
#include "formats/language.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <string.h>

static sd_language *language;

int lang_init(void) {
    const char *filename = pm_get_resource_path(DAT_ENGLISH);

    // Load up language file
    language = omf_calloc(1, sizeof(sd_language));
    if(sd_language_create(language) != SD_SUCCESS) {
        goto error_0;
    }
    if(sd_language_load(language, filename)) {
        PERROR("Unable to load language file '%s'!", filename);
        goto error_0;
    }

    int const lang_count_old = 990;
    int const lang_count_new = 1013;
    if(language->count == lang_count_old){
        // OMF 2.1 added netplay, and with it 23 new localization strings
        int new_ids[]={
            149,
            150,
            172,
            173,
            174,
            175,
            176,
            177,
            178,
            179,
            180,
            181,
            182,
            183,
            184,
            185,
            267,
            269,
            270,
            271,
            284,
            295,
            305,
            -1 // end sentinel
        };

        // insert dummy entries
        sd_lang_string *expanded_strings = omf_malloc(lang_count_new * sizeof(sd_lang_string));
        int next = 0;
        int next_from = 0;
        for(int *id = new_ids; *id != -1; id++) {
            int copy_count = *id - next;
            memcpy(expanded_strings + next, language->strings + next_from, copy_count * sizeof(sd_lang_string));
            next += copy_count;
            next_from += copy_count;

            expanded_strings[next].data = NULL;
            memcpy(expanded_strings[next].description, "dummy", 6);
            next++;
        }
        memcpy(expanded_strings + next, language->strings + next_from, (lang_count_new - next) * sizeof(sd_lang_string));
        omf_free(language->strings);
        language->strings = expanded_strings;
        language->count = lang_count_new;
    }
    if (language->count != lang_count_new) {
        PERROR("Unable to load language file '%s', unsupported file version!", filename);
        goto error_0;
    }

    INFO("Loaded language file '%s'.", filename);

    return 0;

error_0:
    sd_language_free(language);
    omf_free(language);
    return 1;
}

void lang_close(void) {
    sd_language_free(language);
    omf_free(language);
}

const char *lang_get(unsigned int id) {
    if(id > language->count) {
        PERROR("out of range lang id %u!", id);
        return "!INVALID!";
    }
    return language->strings[id].data;
}
