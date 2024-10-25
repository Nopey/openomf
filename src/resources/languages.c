#include "resources/languages.h"
#include "formats/error.h"
#include "formats/language.h"
#include "resources/ids.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <string.h>

static sd_language *language;

int lang_init(void) {
    resource_id user_lang = DAT_GERMAN;
    sd_language *translation = NULL;
    // always load english first
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

    INFO("Loaded base language file '%s'.", filename);

    if(user_lang != DAT_ENGLISH) {
        // let translation override english strings
        filename = pm_get_resource_path(user_lang);

        // Load up language file
        translation = omf_calloc(1, sizeof(sd_language));
        if(sd_language_create(translation) != SD_SUCCESS) {
            goto error_0;
        }
        if(sd_language_load(translation, filename)) {
            PERROR("Unable to load language file '%s'!", filename);
            goto error_0;
        }

        int id = 0, t_id = 0;
        while(id < language->count && t_id < translation->count) {
            sd_lang_string *lang_str = &language->strings[id];
            sd_lang_string *t_str = &translation->strings[t_id];
            // XXX only check first 7 bytes of description, as german 465 has a stray e at offset 8.
            if(memcmp(lang_str->description, t_str->description, 7) == 0) {
                // swap english and translation string
                char *eng_str = lang_str->data;
                lang_str->data = t_str->data;
                t_str->data = eng_str;
                t_id++;
            }
            id++;
        }
        if(t_id < translation->count) {
            PERROR("couldn't find english string for %s string id %u (%s)!", get_resource_name(user_lang), t_id,
                   translation->strings[t_id].description);
            goto error_0;
        }
    }
    INFO("Loaded language file '%s'.", filename);

    sd_language_free(translation);
    omf_free(translation);
    return 0;

error_0:
    sd_language_free(language);
    omf_free(language);
    sd_language_free(translation);
    omf_free(translation);
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
