//usr/bin/env tcc -DPROC_COUNT=4 $(ls *.c | grep -v main.c) -run "$0" "$@" ; exit $?

#include <stdio.h>

#include "file.h"
#include "lookup.h"
#include "platform.h"
#include "err.h"
#include "arg.h"
#include "screen.h"
//#include "colorprint.h"
#include "str.h"
#include "info.h"
#include "cft.h"


#include <linux/limits.h>

//#include <ctype.h>

int main(int argc, const char **argv)
{
#if 0
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }
#endif

    int err = 0;

    info_disable_all(INFO_LEVEL_ALL);
    //info_enable_all(INFO_LEVEL_ID | INFO_LEVEL_TEXT);
    TRY(platform_colorprint_init(), ERR_PLATFORM_COLORPRINT_INIT);

    Arg arg = {0};
    Cft cft = {0};
#if 1
    //Str parse = STR("../data/test.txt");
    //Str parse = STR("../data/tags.txt.bkp");
    Str parse = STR("/home/hp/.config/cft/tags.cft");

    TRY(arg_parse(&arg, argc, argv), ERR_ARG_PARSE);
    if(arg.exit_early) goto clean;
    //screen_enter();

    TRYC(cft_init(&cft));
    Str content = {0};
    TRYC(file_str_read(&parse, &content));
    TRYC(cft_parse(&cft, &content));

    /* print all tags */
    if(arg.parsed.list) {
        VrTagRef all = {0};
        size_t *counts = 0;
        TRY(trrtagref_dump(&cft.all, &all.items, &counts, &all.last), ERR_LUTD_DUMP);
        vrtagref_sort(&all, counts);
        printf("TOTAL TAGS: %zu\n", vrtagref_length(&all));
        for(size_t i = 0; i < vrtagref_length(&all); ++i) {
            TagRef *tag = vrtagref_get_at(&all, i);
            printf("  [%zu] %.*s (%zu)\n", i, STR_F(&tag->tag), counts[i]);
        }
        vrtagref_free(&all);
        free(counts);
    }

    //str_free(&content);

    if(0) {
        str_clear(&content);
        TRYC(cft_fmt(&cft, &content));
        Str output = STR("../data/tags-output.txt");
        TRYC(file_str_write(&output, &content));
    }

    /* test search */
#if 1
    TrrTag found = {0};
    TRY(trrtag_init(&found, 10), ERR_LUTD_INIT);
    Str tags = STR("anime,boy");
    if(str_length(&arg.parsed.find_any)) {
        TRYC(cft_find_any(&cft, &found, &arg.parsed.find_any));
    }
    if(str_length(&arg.parsed.find_and)) {
        TRYC(cft_find_and(&cft, &found, &arg.parsed.find_and));
    }
    if(str_length(&arg.parsed.find_not)) {
        TRYC(cft_find_not(&cft, &found, &arg.parsed.find_not));
    }
    VrTag results = {0};
    TRY(trrtag_dump(&found, &results.items, 0, &results.last), ERR_LUTD_DUMP);
    vrtag_sort(&results);
    for(size_t i = 0; i < vrtag_length(&results); ++i) {
        Tag *file = vrtag_get_at(&results, i);
        printf("%.*s\n", STR_F(&file->filename));
    }
#endif

#if 0
    TagRef *foundr = {0};
    Str tag = STR("scenery");
    TRYC(cft_find_by_tag(&cft, &foundr, &tag, false));
    if(foundr) {
        info(tag_found_count, "Found %zu:", vrstr_length(&foundr->filenames));
        for(size_t i = 0; i < vrstr_length(&foundr->filenames); ++i) {
            Str *filename = vrstr_get_at(&foundr->filenames, i);
            printf("%.*s\n", STR_F(filename));
        }
    } else {
        info(tag_found_count, "Nothing found.\n");
    }
#endif

    str_free(&content);
#endif

    //cft_add(&cft, &STR("A"), &STR("1"));
    //cft_add(&cft, &STR("A"), &STR("2"));
    //cft_add(&cft, &STR("A"), &STR("3"));
    //cft_add(&cft, &STR("B"), &STR("1"));
    //cft_add(&cft, &STR("B"), &STR("2"));
    //cft_add(&cft, &STR("B"), &STR("3"));

clean:
    cft_free(&cft);
    arg_free(&arg);
    info_handle_abort();
    //screen_leave();
    return err;
error:
    ERR_CLEAN;
}

