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

    if(language->count == 1013){
        // OMF 2.1 added netplay, and 23 new localization strings.
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
        int gapless_count = new_ids[0];
        int first_id_after_gap = new_ids[0] + 1;
        for(int *to_remove = new_ids + 1;; to_remove++) {
            int end = *to_remove;
            if(end == -1){
                end = language->count;
            } else {
                omf_free(language->strings[*to_remove].data);
            }
            int count = end - first_id_after_gap;
            memmove(&language->strings[gapless_count], &language->strings[first_id_after_gap], count * sizeof(sd_lang_string));
            gapless_count += count;
            first_id_after_gap += count + 1;
            if(*to_remove == -1)
                break;
        }
        memset(&language->strings[gapless_count], 0, language->count - gapless_count);
        language->count = gapless_count;
    } else if (language->count != 990) {
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
