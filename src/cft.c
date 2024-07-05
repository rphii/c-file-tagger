#include "cft.h"
#include "str.h"
#include "info.h"

ErrDecl cft_init(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    // TODO: make the table sizes some switches
    TRY(trtag_init(&cft->tags, 10), ERR_LUTD_INIT);
    TRY(trtagref_init(&cft->reverse, 10), ERR_LUTD_INIT);
    TRY(trrtagref_init(&cft->all, 10), ERR_LUTD_INIT); // TODO: skip if not needed
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_arg(Cft *cft, Arg *arg) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(arg);
    cft->options.decorate = (arg->parsed.decorate == SPECIFY_OPTION_YES || arg->parsed.decorate == SPECIFY_OPTION_TRUE);
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_find_by_tag(Cft *cft, TagRef **foundr, const Str *tag, bool create_if_nonexist) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(tag);
    TagRef searchr = {.tag = *tag};
    str_trim(&searchr.tag);
    if(!str_length(&searchr.tag)) return 0;
    info(tag_search, "Searching tag '%.*s'", STR_F(&searchr.tag));
    /* first add the tag to the file-specific table */
    if(!trtagref_has(&cft->reverse, &searchr)) {
        if(!create_if_nonexist) return 0;
        str_zero(&searchr.tag);
        TRYC(str_fmt(&searchr.tag, "%.*s", STR_F(tag)));
        TRY(trtagref_add(&cft->reverse, &searchr), ERR_LUTD_ADD);
        info(tag_created, "Created tag '%.*s'", STR_F(&searchr.tag));
    }
    size_t iir, jjr;
    TRY(trtagref_find(&cft->reverse, &searchr, &iir, &jjr), ERR_LUTD_ADD);
    *foundr = cft->reverse.buckets[iir].items[jjr];
    /* then add the tag to the global table ... TODO: make it an optional flag! increase
     * performance! */
    TRY(trrtagref_add(&cft->all, &searchr), ERR_LUTD_ADD); // TODO: if some file has 1000 equal
                                                           // entries, don't count...
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_find_by_filename(Cft *cft, Tag **found, const Str *filename, bool create_if_nonexist) { //{{{
    size_t ii, jj;
    Tag search = {.filename = *filename};
    str_trim(&search.filename);
    info(filename_search, "Searching filename '%.*s'", STR_F(&search.filename));
    if(!str_length(&search.filename)) return 0;
    if(str_get_front(&search.filename) == '#') return 0;
    if(!trtag_has(&cft->tags, &search)) {
        if(!create_if_nonexist) return 0;
        str_zero(&search.filename);
        //TRYC(str_copy(&search.filename, filename));
        TRYC(str_fmt(&search.filename, "%.*s", STR_F(filename)));
        TRY(trtag_add(&cft->tags, &search), ERR_LUTD_ADD);
        info(filename_created, "Created filename '%.*s'", STR_F(&search.filename));
    }
    TRY(trtag_find(&cft->tags, &search, &ii, &jj), ERR_LUTD_ADD);
    *found = cft->tags.buckets[ii].items[jj];
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_add(Cft *cft, const Str *filename, const Str *tag) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(filename);
    ASSERT_ARG(tag);
    /* search if we have a matching file ... */
    Str tag_search = *tag;
    Tag *found = 0;
    TRYC(cft_find_by_filename(cft, &found, filename, true));
    if(!found) return 0;
    //printff("filename retrieved '%.*s'", STR_F(&found->filename));
    for(;;) {
        /* check if the tag is present */
        bool present = false;
        for(size_t i = 0; i < vrstr_length(&found->tags); ++i) {
            Str *cmp = vrstr_get_at(&found->tags, i);
            if(str_cmp(cmp, &tag_search)) continue;
            present = true;
            break;
        }
        /* search for a file tagged with such tag */
        TagRef *foundr = 0;
        TRYC(cft_find_by_tag(cft, &foundr, &tag_search, true));
        if(!foundr) return 0;
        /* check if the file is present */
        bool presentr = false;
        for(size_t i = 0; i < vrstr_length(&foundr->filenames); ++i) {
            Str *cmp = vrstr_get_at(&foundr->filenames, i);
            if(str_cmp(cmp, filename)) continue;
            presentr = true;
            break;
        }
        /* add if necessary */
        if(!presentr) {
            TRY(vrstr_push_back(&foundr->filenames, &found->filename), ERR_VEC_PUSH_BACK);
            //printff("added filename '%.*s' to tag '%.*s'", STR_F(&found->filename), STR_F(&foundr->tag));
        }
        if(!present) {
            TRY(vrstr_push_back(&found->tags, &foundr->tag), ERR_VEC_PUSH_BACK);
            //printff("added tag '%.*s' to filename '%.*s'", STR_F(&foundr->tag), STR_F(&found->filename));
        }
        info(tag_done, "Tagged '%.*s' with '%.*s'", STR_F(&found->filename), STR_F(&foundr->tag));
        /* check next : */
        size_t iE = str_rch(&tag_search, ':', 0);
        if(iE >= str_length(&tag_search)) break;
        tag_search.last = tag_search.first + iE;
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_parse(Cft *cft, const Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
    info(parsing, "Parsing");
    Str line = *str;
    //printff("str_length %zu", str_length(str));
    for(;;) {
        str_get_line(str, &line.first, &line.last);
        Str filename = {0};
        Str tag = {0};
        for(;;) {
            tag = str_splice(&line, &tag, ',');
            //printff("TAG %zu..%zu:'%.*s'", tag.first, tag.last, STR_F(&tag));
            //printff("[%zu..%zu] %.*s", line.first, line.last, STR_F(&line));getchar();
            if(!filename.s) {
                filename = tag;
            } else {
                //printff("add %.*s to %.*s", STR_F(&tag), STR_F(&filename));
                TRYC(cft_add(cft, &filename, &tag));
            }
            if(tag.first >= line.last) { break;}
        }
        line.first = 1 + line.last;
        if(line.first >= str_length(str)) break;
    }
    info_check(INFO_parsing, true);
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_fmt(Cft *cft, Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
    info(formatting, "Formatting");
    int err = 0;
    VrTag tags = {0};
    trtag_dump(&cft->tags, &tags.items, 0, &tags.last);
    vrtag_sort(&tags); // TODO add switch
    for(size_t i = 0; i < vrtag_length(&tags); ++i) {
        Tag *tag = vrtag_get_at(&tags, i);
        TRYC(str_fmt(str, "%.*s", STR_F(&tag->filename)));
        for(size_t j = 0; j < vrstr_length(&tag->tags); ++j) {
            Str *tagg = vrstr_get_at(&tag->tags, j);
            TRYC(str_fmt(str, ",%.*s", STR_F(tagg)));
        }
        TRYC(str_fmt(str, "\n"));
    }
    info_check(INFO_formatting, true);
clean:
    vrtag_free(&tags);
    return err;
error: ERR_CLEAN;
} //}}}

void cft_free(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    trtag_free(&cft->tags);
    trtagref_free(&cft->reverse);
    trrtagref_free(&cft->all);
} //}}}

ErrDecl cft_find_any(Cft *cft, TrrTag *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
    Str tag = {0};
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TagRef *foundr = {0};
        TRYC(cft_find_by_tag(cft, &foundr, &tag, false));
        if(foundr) {
            for(size_t i = 0; i < vrstr_length(&foundr->filenames); ++i) {
                Str *filename = vrstr_get_at(&foundr->filenames, i);
                Tag hit = {.filename = *filename};
                TRY(trrtag_add(found, &hit), ERR_LUTD_ADD);
                //printf("added:%.*s\n", STR_F(filename));
            }
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_find_and(Cft *cft, TrrTag *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    int err = 0;
    TrrTag temp = {0};
    TrrTag temp_swap = {0};
    TRY(trrtag_init(&temp, found->width), ERR_LUTD_INIT);
    /* search... */
    Str tag = {0};
    size_t iteration = 0;
    if(!trrtag_empty(found)) {
        ++iteration;
    }
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        //printf("[%.*s]\n", STR_F(&tag));
        /* search */
        TagRef *foundr = {0};
        TRYC(cft_find_by_tag(cft, &foundr, &tag, false));
        if(foundr) {
            for(size_t i = 0; i < vrstr_length(&foundr->filenames); ++i) {
                Str *filename = vrstr_get_at(&foundr->filenames, i);
                Tag hit = {.filename = *filename};
                //printf("%zu:%.*s\n", iteration, STR_F(filename));
                if(iteration == 0) {
                    TRY(trrtag_add(found, &hit), ERR_LUTD_ADD);
                } else {
                    if(trrtag_has(found, &hit)) {
                        TRY(trrtag_add(&temp, &hit), ERR_LUTD_ADD);
                    }
                    //if(!trrtag_has(found,
                }
                //printf("added:%.*s\n", STR_F(filename));
            }
        }
        if(iteration) {
            temp_swap = *found;
            trrtag_clear(found);
            *found = temp;
            temp = temp_swap;
        } else {
            //temp_swap =
        }
        ++iteration;
    }
clean:
    trrtag_free(&temp_swap);
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_find_not(Cft *cft, TrrTag *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
    Str tag = {0};
    if(trrtag_empty(found)) {
        /* add all tags! */
        for(size_t ii = 0; ii < (1ULL << (cft->tags.width - 1)); ++ii) {
            for(size_t jj = 0; jj < cft->tags.buckets[ii].len; ++jj) {
                Tag *hit = cft->tags.buckets[ii].items[jj];
                TRY(trrtag_add(found, hit), ERR_LUTD_ADD);
            }
        }
    }
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TagRef *foundr = {0};
        TRYC(cft_find_by_tag(cft, &foundr, &tag, false));
        if(foundr) {
            for(size_t i = 0; i < vrstr_length(&foundr->filenames); ++i) {
                Str *filename = vrstr_get_at(&foundr->filenames, i);
                Tag hit = {.filename = *filename};
                trrtag_del(found, &hit);
                //printf("removed:%.*s\n", STR_F(filename));
            }
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_list_fmt(Cft *cft, Str *out, VrStr *files) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(files);
    int err = 0;
    VrTagRef all = {0};
    TrrTag filtered = {0};
    size_t *counts = 0;
    /* search */
    if(vrstr_length(files)) {
        //TRY(trrtag_init(&filtered, cft->tags.width), ERR_LUTD_INIT);
        //for(size_t i = 0; i < vrstr_length(files); ++i) {
        //    Str *file = vrstr_get_at(files, i);
        //}
    } else {
        TRY(trrtagref_dump(&cft->all, &all.items, &counts, &all.last), ERR_LUTD_DUMP);
    }
    vrtagref_sort(&all, counts);
    TRYC(str_fmt(out, "Total Tags: %zu\n", vrtagref_length(&all)));
    //printf("TOTAL TAGS: %zu\n", vrtagref_length(&all));
    for(size_t i = 0; i < vrtagref_length(&all); ++i) {
        TagRef *tag = vrtagref_get_at(&all, i);
        //printf("  [%zu] %.*s (%zu)\n", i, STR_F(&tag->tag), counts[i]);
        if(cft->options.decorate) {
            TRYC(str_fmt(out, "  [%zu] ", i));
        }
        TRYC(str_fmt(out, "%.*s", STR_F(&tag->tag)));
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", counts[i]));
        }
        TRYC(str_fmt(out, "\n"));
    }
clean:
    trrtag_free(&filtered);
    vrtagref_free(&all);
    free(counts);
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_find_fmt(Cft *cft, Str *out, Str *find_any, Str *find_and, Str *find_not, bool list_tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(find_any);
    ASSERT_ARG(find_and);
    ASSERT_ARG(find_not);
    int err = 0;
    TrrTag found = {0};
    VrTag results = {0};
    TRY(trrtag_init(&found, 10), ERR_LUTD_INIT); // TODO switch
    if(str_length(find_any)) {
        TRYC(cft_find_any(cft, &found, find_any));
    }
    if(str_length(find_and)) {
        TRYC(cft_find_and(cft, &found, find_and));
    }
    if(str_length(find_not)) {
        TRYC(cft_find_not(cft, &found, find_not));
    }
    TRY(trrtag_dump(&found, &results.items, 0, &results.last), ERR_LUTD_DUMP);
    vrtag_sort(&results);
    for(size_t i = 0; i < vrtag_length(&results); ++i) {
        Tag *file = vrtag_get_at(&results, i);
        if(cft->options.decorate) {
            TRYC(str_fmt(out, "  [%zu] ", i));
        }
        TRYC(str_fmt(out, "%.*s", STR_F(&file->filename)));
        size_t ii, jj;
        Tag *found = 0;
        if(list_tags || cft->options.decorate) {
            TRY(trtag_find(&cft->tags, file, &ii, &jj), ERR_LUTD_FIND);
            found = cft->tags.buckets[ii].items[jj];
        }
        if(list_tags) {
            for(size_t j = 0; j < vrstr_length(&found->tags); ++j) {
                Str *tag = vrstr_get_at(&found->tags, j);
                TRYC(str_fmt(out, ",%.*s", STR_F(tag)));
            }
        }
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", vrstr_length(&found->tags)));
        }
        TRYC(str_fmt(out, "\n"));
        //printf("%.*s\n", STR_F(&file->filename));
    }
clean:
    trrtag_free(&found);
    vrtag_free(&results);
    return err;
error:
    ERR_CLEAN;

} //}}}

