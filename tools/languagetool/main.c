/** @file main.c
 * @brief Language file parser tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/language.h"
#include "utils/allocator.h"
#include "utils/cp437.h"
#include "utils/str.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#if ARGTABLE2_FOUND
#include <argtable2.h>
#elif ARGTABLE3_FOUND
#include <argtable3.h>
#endif

#define MAX_LINE 2048
#define MAX_DATA 8192 // Data field cannot exceed 32 bytes
#define MAX_TITLE 32

noreturn void error_exit(const char *message, int line_number) {
    fprintf(stderr, "Error on line %d: %s\n", line_number, message);
    exit(EXIT_FAILURE);
}

typedef enum
{
    skip_empty,
    no_skip,
} enum_skip_empty;

static char *lang_nextline(char *line, size_t sizeof_line, FILE *file, int *line_number, enum_skip_empty should_skip) {
    assert(sizeof_line > 1);
    while(true) {
        (*line_number)++;
        char *read_line = fgets(line, sizeof_line, file);
        if(!read_line) {
            // always return on EOF
            return read_line;
        }
        if(*read_line == '#') {
            // skip comments
            continue;
        }
        if(should_skip == skip_empty && (*read_line == '\n' || *read_line == '\0')) {
            continue;
        }
        return read_line;
    }
}

// Function to extract value after colon with validation
char *extract_value(char *line, const char *field_name, int line_number, bool allow_empty) {
    {
        size_t line_len = strlen(line);
        size_t field_name_len = strlen(field_name);
        if(field_name_len > line_len || memcmp(line, field_name, field_name_len) != 0) {
            char error[100];
            snprintf(error, sizeof(error), "Expected %s field", field_name);
            error_exit(error, line_number);
        }
        line += field_name_len;
    }

    if(':' != line[0]) {
        char error[100];
        snprintf(error, sizeof(error), "Missing colon in %s field", field_name);
        error_exit(error, line_number);
    }
    line++;

    if(' ' != line[0]) {
        char error[100];
        snprintf(error, sizeof(error), "Missing space following colon in %s field", field_name);
        error_exit(error, line_number);
    }
    line++;

    if(!allow_empty && line[0] == '\0') {
        char error[100];
        snprintf(error, sizeof(error), "Empty %s field", field_name);
        error_exit(error, line_number);
    }

    return line;
}

int read_entry(FILE *file, sd_language *language, sd_language const *base_language, int *line_number) {
    char line[MAX_LINE];
    if(!lang_nextline(line, sizeof(line), file, line_number, skip_empty)) {
        // EOF is ok here
        return 0;
    }

    long id;
    {
        str value;
        str_from_c(&value, extract_value(line, "ID", *line_number, false));
        str_strip(&value);
        if(strcmp(str_c(&value), "auto") == 0) {
            id = (long)language->count;
        } else if(!str_to_long(&value, &id)) {
            error_exit("ID must be a valid integer", *line_number);
        }
        str_free(&value);
    }

    if(language->count != (unsigned int)id) {
        char error[100];
        snprintf(error, sizeof error, "Nonsequential ID. Expected %u, got %ld.", language->count, id);
        error_exit(error, *line_number);
    }

    if(!lang_nextline(line, sizeof(line), file, line_number, no_skip)) {
        error_exit("Unexpected EOF while reading Title", *line_number);
    }

    str desc;
    str_from_c(&desc, extract_value(line, "Title", *line_number, true));
    str_strip(&desc);

    size_t max_desc_len = MAX_TITLE - 1;
    if(str_size(&desc) > max_desc_len) {
        fprintf(stderr, "Warning: truncating overlong 'Title:' of entry id %d. Length is %zu, max length is %zu\n",
                language->count, str_size(&desc), max_desc_len);
        str trunc;
        str_from_buf(&trunc, str_c(&desc), max_desc_len);
        str_free(&desc);
        desc = trunc;
    }

    // Read Data header
    if(!lang_nextline(line, sizeof(line), file, line_number, no_skip)) {
        error_exit("Unexpected EOF while reading Data", *line_number);
    }

    if(strncmp(line, "CopyBase:", 5) == 0) {
        if(base_language->count == 0) {
            error_exit("CopyBase used, but no base language loaded!", *line_number);
        }
        char *value = extract_value(line, "CopyBase", *line_number, true);
        char *range_separator;
        long range_start, range_end;
        str baseid;
        if((range_separator = strstr(value, "..="))) {
            *range_separator = '\0';
            char *cstr_range_end = range_separator + 3;

            str_from_c(&baseid, value);
            str_strip(&baseid);
            if(!str_to_long(&baseid, &range_start) || range_start > (long)base_language->count || range_start < 0) {
                str err;
                str_from_format(&err, "CopyBase range must start with a valid ID within limits 0..=%u, found '%s'\n",
                                base_language->count - 1, str_c(&baseid));
                error_exit(str_c(&err), *line_number);
            }
            str_free(&baseid);

            str_from_c(&baseid, cstr_range_end);
            str_strip(&baseid);
            if(!str_to_long(&baseid, &range_end) || range_end > (long)base_language->count || range_end < 0) {
                str err;
                str_from_format(&err, "CopyBase range must end with a valid ID within limits 0..=%u, found '%s'\n",
                                base_language->count - 1, str_c(&baseid));
                error_exit(str_c(&err), *line_number);
            }
            str_free(&baseid);
        } else {
            str_from_c(&baseid, value);
            str_strip(&baseid);
            if(!str_to_long(&baseid, &range_start) || range_start > (long)base_language->count || range_start < 0) {
                str err;
                str_from_format(&err, "CopyBase must specify a valid ID or range within limits 0..=%u, found '%s'\n",
                                base_language->count - 1, str_c(&baseid));
                error_exit(str_c(&err), *line_number);
            }
            str_free(&baseid);

            // range contains a single idx
            range_end = range_start;
        }

        for(long from_id = range_start; from_id <= range_end; from_id++) {
            sd_lang_string const *from = &base_language->strings[(unsigned int)from_id];
            sd_language_append(language, str_c(&desc), from->data);
        }
        str_free(&desc);
        return 1;
    }

    char *data = malloc(8192);
    memset(data, 0, 8192);
    char *data_end = data + 8192;
    char *value = extract_value(line, "Data", *line_number, true);
    char *data_iter = data;
    size_t value_len = strlen(value);
    if(data + value_len + 1 > data_end) {
        error_exit("Way too long 'Data:' field", *line_number);
    }
    memcpy(data_iter, value, value_len + 1);
    data_iter += value_len;

    // Read data body until next entry or EOF
    while(lang_nextline(line, sizeof(line), file, line_number, no_skip)) {
        // Check if this is the start of a new entry
        if(strncmp(line, "ID:", 3) == 0) {
            // Rewind to start of this line
            fseek(file, -strlen(line), SEEK_CUR);
            *line_number -= 1;
            break;
        }

        // Append to existing data
        value_len = strlen(line);
        if(data_iter + value_len + 1 > data_end) {
            error_exit("Way too long 'Data:' field", *line_number);
        }
        memcpy(data_iter, line, value_len + 1);
        data_iter += value_len;
    }

    if(data_iter > data && data_iter[-1] == '\n') {
        // trim a single trailing newline
        data_iter[-1] = '\0';
    }

    sd_language_append(language, str_c(&desc), data);
    free(data);
    str_free(&desc);
    return 1;
}

typedef struct conversion_result {
    cp437_result error_code;
    unsigned int string_index;
} conversion_result;

static conversion_result sd_language_to_utf8(sd_language *language) {
    assert(language);
    conversion_result result;
    for(unsigned int idx = 0; idx < language->count; idx++) {
        result.string_index = idx;
        char *old_data = language->strings[idx].data;
        size_t sizeof_old_data = strlen(old_data) + 1;
        char old_description[sizeof language->strings[idx].description];
        memcpy(old_description, language->strings[idx].description, sizeof old_description);

        // convert data to utf-8
        size_t sizeof_utf8_data;
        result.error_code = cp437_to_utf8(NULL, 0, &sizeof_utf8_data, (uint8_t const *)old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS)
            return result;
        language->strings[idx].data = omf_malloc(sizeof_utf8_data);
        result.error_code = cp437_to_utf8((unsigned char *)language->strings[idx].data, sizeof_utf8_data, NULL,
                                          (uint8_t const *)old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS) {
            assert(!"cp437_to_utf8 should have failed the first time or not the second");
            omf_free(language->strings[idx].data);
            language->strings[idx].data = old_data;
            return result;
        }
        omf_free(old_data);

        // convert description
        // TODO: Use strnlen_s here, check if description is missing its NUL terminator
        size_t sizeof_old_description = strlen(old_description) + 1;
        memset(language->strings[idx].description, '\0', sizeof language->strings[idx].description);
        result.error_code = cp437_to_utf8((unsigned char *)language->strings[idx].description,
                                          sizeof language->strings[idx].description, NULL,
                                          (uint8_t const *)old_description, sizeof_old_description);
        if(result.error_code != CP437_SUCCESS) {
            memcpy(language->strings[idx].description, old_description, sizeof old_description);
            return result;
        }
    }

    result.error_code = CP437_SUCCESS;
    result.string_index = 0;
    return result;
}

static conversion_result sd_language_from_utf8(sd_language *language) {
    assert(language);
    conversion_result result;
    for(size_t idx = 0; idx < language->count; idx++) {
        result.string_index = idx;
        char *old_data = language->strings[idx].data;
        size_t sizeof_old_data = strlen(old_data) + 1;
        char old_description[sizeof language->strings[idx].description];
        memcpy(old_description, language->strings[idx].description, sizeof old_description);

        // convert data to DOS CP 437
        size_t sizeof_cp437_data;
        result.error_code =
            cp437_from_utf8(NULL, 0, &sizeof_cp437_data, (unsigned char const *)old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS)
            return result;
        language->strings[idx].data = omf_malloc(sizeof_cp437_data);
        result.error_code = cp437_from_utf8((uint8_t *)language->strings[idx].data, sizeof_cp437_data, NULL,
                                            (unsigned char const *)old_data, sizeof_old_data);
        if(result.error_code != CP437_SUCCESS) {
            assert(!"cp437_from_utf8 should have failed the first time or not the second");
            omf_free(language->strings[idx].data);
            language->strings[idx].data = old_data;
            return result;
        }
        omf_free(old_data);

        // convert description
        // TODO: Use strnlen_s here, check if description is missing its NUL terminator
        size_t sizeof_old_description = strlen(old_description) + 1;
        memset(language->strings[idx].description, '\0', sizeof language->strings[idx].description);
        result.error_code =
            cp437_from_utf8((uint8_t *)language->strings[idx].description, sizeof language->strings[idx].description,
                            NULL, (unsigned char const *)old_description, sizeof_old_description);
        if(result.error_code != CP437_SUCCESS) {
            memcpy(language->strings[idx].description, old_description, sizeof old_description);
            return result;
        }
    }

    result.error_code = CP437_SUCCESS;
    result.string_index = 0;
    return result;
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file0("f", "file", "<file>", "load OMF language file");
    struct arg_file *input = arg_file0("i", "import", "<file>", "import UTF-8 .TXT file");
    struct arg_file *base = arg_file0("b", "base", "<file>", "load OMF language file as BASE");
    struct arg_int *str = arg_int0("s", "string", "<value>", "display language string number");
    struct arg_file *output = arg_file0("o", "output", "<file>", "compile output language file");
    struct arg_int *check_count =
        arg_int0("c", "check-count", "<NUM>", "Check that language file has this many entries, or bail.");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, input, base, output, str, check_count, end};
    const char *progname = "languagetool";

    bool language_is_utf8 = false;
    sd_language language;
    sd_language_create(&language);

    bool base_language_is_utf8 = false;
    sd_language base_language;
    sd_language_create(&base_language);

    // assume failure until success happens.
    int main_ret = EXIT_FAILURE;

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        fprintf(stderr, "%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        printf("%s v0.2\n", progname);
        printf("Command line One Must Fall 2097 Language file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013-2024 Tuomas Virtanen & Contributors\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stderr, end, progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    if(file->count + input->count > 1) {
        fprintf(stderr, "Too many inputs provided! please supply one --file or --import argument.\n");
        goto exit_0;
    } else if(file->count + input->count < 1) {
        fprintf(stderr, "Not enough inputs provided! please supply one --file or --import argument.\n");
        goto exit_0;
    }

    if(base->count > 0 && input->count == 0) {
        fprintf(stderr, "Unexpected --base argument: it is meaningless without --import.\n");
        goto exit_0;
    } else if(base->count > 1) {
        fprintf(stderr, "Too many --base arguments: please supply only one.\n");
        goto exit_0;
    }

    int ret;

    // Get strings from binary lang file
    if(file->count > 0) {
        ret = sd_language_load(&language, file->filename[0]);
        language_is_utf8 = false;
        if(ret != SD_SUCCESS) {
            fprintf(stderr, "Language file could not be loaded! Error [%d] %s\n", ret, sd_get_error(ret));
            goto exit_0;
        }
    }

    // Get strings from base lang file
    if(base->count > 0) {
        ret = sd_language_load(&base_language, base->filename[0]);
        base_language_is_utf8 = false;
        if(ret != SD_SUCCESS) {
            fprintf(stderr, "Base language file could not be loaded! Error [%d] %s\n", ret, sd_get_error(ret));
            goto exit_0;
        }
    }

    // Import TXT file
    if(input->count > 0) {
        char const *expected_ext = ".TXT";
        if(!input->extension[0] || strcmp(input->extension[0], expected_ext) != 0) {
            fprintf(stderr, "Refusing to open input file %s, does not have expected %s file extension.\n",
                    input->filename[0], expected_ext);
            goto exit_0;
        }
        // parse the supplied text file
        FILE *file = fopen(input->filename[0], "rb");
        if(!file) {
            fprintf(stderr, "Could not open %s\n", input->filename[0]);
            goto exit_0;
        }

        if(!base_language_is_utf8) {
            conversion_result result = sd_language_to_utf8(&base_language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting base language '%s' to UTF-8! Error %s on language entry %u\n",
                        base->filename[0], cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            base_language_is_utf8 = true;
        }

        // line is incremented prior to parsing each line
        int line = 0;
        language_is_utf8 = true;
        assert(base_language_is_utf8 == language_is_utf8);
        while(read_entry(file, &language, &base_language, &line)) {
        }
    }

    if(check_count->count > 0 && (unsigned)check_count->ival[0] != language.count) {
        fprintf(stderr, "Expected %u entries, got %d!\n", (unsigned)check_count->ival[0], language.count);
        goto exit_0;
    }

    // Print
    const sd_lang_string *ds;
    if(str->count > 0) {
        if(!language_is_utf8) {
            conversion_result result = sd_language_to_utf8(&language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting to UTF-8! Error %s on language entry %u\n",
                        cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            language_is_utf8 = true;
        }
        unsigned str_id = (unsigned)str->ival[0];
        ds = sd_language_get(&language, str_id);
        if(ds == NULL) {
            fprintf(stderr, "String %d not found!\n", str_id);
            goto exit_0;
        }

        printf("Title: %s\n", ds->description);
        printf("Data: %s\n", ds->data);
    } else if(output->count == 0) {
        if(!language_is_utf8) {
            conversion_result result = sd_language_to_utf8(&language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting to UTF-8! Error %s on language entry %u\n",
                        cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            language_is_utf8 = true;
        }
        for(unsigned i = 0; i < language.count; i++) {
            ds = sd_language_get(&language, i);
            if(ds != NULL) {
                printf("ID: %d\n", i);
                printf("Title: %s\n", ds->description);
                printf("Data: %s\n", ds->data);
            }
        }
    }

    // Save
    if(output->count > 0) {
        char const *expected_output_extensions[] = {".DAT", ".DAT2", ".LNG", ".LNG2"};
        bool unexpected_extension = true;
        for(size_t i = 0; i < (sizeof expected_output_extensions) / (sizeof expected_output_extensions[0]); i++) {
            if(output->extension[0] && strcmp(expected_output_extensions[i], output->extension[0]) == 0) {
                unexpected_extension = false;
                break;
            }
        }
        if(unexpected_extension) {
            fprintf(stderr, "Refusing to save language file to %s: unexpected file extension.\n", output->filename[0]);
            goto exit_0;
        }

        if(language_is_utf8) {
            conversion_result result = sd_language_from_utf8(&language);
            if(result.error_code != CP437_SUCCESS) {
                fprintf(stderr, "Error converting from UTF-8! Error %s on language entry %u\n",
                        cp437_result_to_string(result.error_code), result.string_index);
                goto exit_0;
            }
            language_is_utf8 = false;
            assert(!language_is_utf8); // silence dead store warning
        }

        ret = sd_language_save(&language, output->filename[0]);
        if(ret != SD_SUCCESS) {
            fprintf(stderr, "Failed saving language file to %s: %s\n", output->filename[0], sd_get_error(ret));
            goto exit_0;
        }
    }

    main_ret = EXIT_SUCCESS;
exit_0:
    sd_language_free(&language);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return main_ret;
}
