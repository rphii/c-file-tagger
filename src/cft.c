#include "cft.h"
#include "file.h"
#include "lut.h"
#include "str.h"
#include "info.h"
#include "vector.h"
#include "platform.h"

#define CFT_LUT_1   18
#define CFT_LUT_2   6

ErrDecl cft_init(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    if(cft->options.expand_paths) {
        TRYC(platform_fmt_home(&cft->misc.homedir));
        TRYC(platform_fmt_cwd(&cft->misc.current_dir));
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_arg(Cft *cft, Arg *arg) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(arg);
    /* bools */
    cft->options.decorate = (arg->parsed.decorate == SPECIFY_OPTION_YES || arg->parsed.decorate == SPECIFY_OPTION_TRUE || arg->parsed.decorate == SPECIFY_OPTION_Y);
    cft->options.query = (str_length(&arg->parsed.find_and) || str_length(&arg->parsed.find_any) || str_length(&arg->parsed.find_not));
    cft->options.modify = (str_length(&arg->parsed.tags_add) || str_length(&arg->parsed.tags_del) || str_length(&arg->parsed.tags_re));
    cft->options.merge = arg->parsed.merge;
    cft->options.list_tags = arg->parsed.list_tags;
    cft->options.list_files = arg->parsed.list_files;
    cft->options.title = arg->parsed.title;
    cft->options.expand_paths = arg->parsed.expand_paths;
    cft->options.compact = arg->parsed.compact;
    cft->options.recursive = arg->parsed.recursive;
    cft->options.extensions = arg->parsed.extensions;
    /* TODO ::: THIS IS SO RETARDED MAKE THIS BETTER */
    /* error checking */
    return 0;
error:
    return -1;
} //}}}

#define ERR_cft_find_by_str(...)    "failed adding string to table"
ErrDecl cft_find_by_str(Cft *cft, TTrStr *base, TTrStrItem **table, const Str *tag, bool create_if_nonexist) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(tag);
    ASSERT_ARG(base);
    ASSERT_ARG(table);
    /* prepare to search */
    Str find = *tag;
    str_trim(&find);
    if(!str_length(&find)) return 0;
    /* now search */
    info(INFO_tag_search, "Searching '%.*s'", STR_F(&find));
    *table = ttrstr_get_kv(base, &find);
    Str temp = {0};
    /* first add the tag to the file-specific table */
    if(!*table) {
        if(!create_if_nonexist) return 0;
        info(INFO_tag_create, "Creating '%.*s'", STR_F(&find));
        str_zero(&temp);
        TRYC(str_copy(&temp, &find));
        TRYG(ttrstr_set(base, &temp, 0));
        *table = ttrstr_get_kv(base, &temp);
        ASSERT(*table, ERR_UNREACHABLE);
        info_check(INFO_tag_create, true);
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_add(Cft *cft, const Str *filename, const Str *tag) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(filename);
    ASSERT_ARG(tag);
    /* prepare to search */
    Str find = *tag;
    /* search if we have a matching file ... */
    TTrStrItem *file_tags = 0;
    TRYC(cft_find_by_str(cft, &cft->base.file_tags, &file_tags, filename, true));
    if(!file_tags) return 0; //THROW(ERR_UNREACHABLE "; should have created/found a table");
    for(;;) {
        /* prepare string */
        str_trim(&find);
        if(!str_length(&find)) break;
        /* find entry */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, &find, true));
        if(!tag_files) return 0; //THROW(ERR_UNREACHABLE "; should have created/found a table");
        /* cross-reference */
        trstr_set(file_tags->val, tag_files->key, 0);
        trstr_set(tag_files->val, file_tags->key, 0);
        /* check next : */
        size_t iE = str_rch(&find, ':', 0);
        if(iE >= str_length(&find)) break;
        find.last = find.first + iE;
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_retag(Cft *cft, const Str *filename, const Str *from, const Str *to) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(from);
    ASSERT_ARG(to);
    THROW("TODO");
    return 0;
error:
    return -1;
} //}}}

#if 0
ErrDecl cft_del(Cft *cft, const Str *filename, const Str *tag) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(filename);
    ASSERT_ARG(tag);
#if 0
    /* search if we have a matching file ... */
    Str tag_search = *tag;
    Tag *found = 0;
    TRYC(cft_find_by_filename(cft, &found, filename, true));
    if(!found) return 0;
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
        /* remove */
        if(!presentr) {
            printff("REMOVE FILENAME %.*s", STR_F(&found->filename));
            //TRY(vrstr_push_back(&foundr->filenames, &found->filename), ERR_VEC_PUSH_BACK);
            //printff("added filename '%.*s' to tag '%.*s'", STR_F(&found->filename), STR_F(&foundr->tag));
        }
        if(!present) {
            printff("REMOVE TAG %.*s", STR_F(&foundr->tag));
            //TRY(vrstr_push_back(&found->tags, &foundr->tag), ERR_VEC_PUSH_BACK);
            //printff("added tag '%.*s' to filename '%.*s'", STR_F(&foundr->tag), STR_F(&found->filename));
        }
        //info(tag_done, "Tagged '%.*s' with '%.*s'", STR_F(&found->filename), STR_F(&foundr->tag));
        break;
        /* check next : */
        //// size_t iE = str_rch(&tag_search, ':', 0);
        //// if(iE >= str_length(&tag_search)) break;
        //// tag_search.last = tag_search.first + iE;
    }
#endif
    return 0;
error:
    return -1;
} //}}}
#endif

ErrDecl cft_parse(Cft *cft, const Str *input, const Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
    int err = 0;
#if 1
    size_t line_number = 0;
    info(INFO_parsing, "Parsing");
    Str line = *str;
    //printff("str_length %zu", str_length(str));
    Str filename_real = {0};
    Str prepend = {0};
    TRYC(str_fmt(&prepend, "%.*s", STR_F(input)));
    TRYC(str_expand_path(&prepend, &cft->misc.current_dir, &cft->misc.homedir));
    for(;;) {
        str_get_line(str, &line.first, &line.last);
        ++line_number;
        //printf("%zu\r", line_number);
        Str filename = {0};
        Str tag = {0};
        for(;;) {
            tag = str_splice(&line, &tag, ',');
            //printff("TAG %zu..%zu:'%.*s'", tag.first, tag.last, STR_F(&tag));
            //printff("[%zu..%zu] %.*s", line.first, line.last, STR_F(&line));getchar();
            if(!filename.s) {
                /* first entry is the filename */
                filename = tag;
                str_trim(&filename);
                if(str_length(&filename)) {
                    if(str_get_front(&filename) == '#') {
                        /* TODO: only read comments when we plan to write out to the file again !!! */
                        /* OTHER TODO: the order will get messed up... because we sort the rest
                         * again, for now... so fix that somehow (but later) */
                        TRYC(str_fmt(&cft->comments, "%.*s\n", STR_F(&line)));
                        break;
                    }
                    /* if we expand, fix filename */
                    if(cft->options.expand_paths && !cft->options.modify) {
                        str_clear(&filename_real);
                        TRYC(str_fmt(&filename_real, "%.*s", STR_F(&filename)));
                        TRYC(str_expand_path(&filename_real, &prepend, &cft->misc.homedir));
                    } else {
                        filename_real = filename;
                    }
                }
            } else {
                //printff("add %.*s to %.*s", STR_F(&tag), STR_F(&filename));
                TRYC(cft_add(cft, &filename_real, &tag));
            }
            if(tag.first >= line.last) { break;}
        }
        line.first = 1 + line.last;
        if(line.first >= str_length(str)) break;
    }
    info_check(INFO_parsing, true);
#endif
clean:
    str_free(&prepend);
    if(cft->options.expand_paths && !cft->options.modify) {
        str_free(&filename_real);
    }
    return err;
error:
    ERR_CLEAN;
} //}}}


#define ERR_cft_file_prepare(cft, filename)     "failed preparing '%.*s'", STR_F(filename)
ErrDecl cft_file_prepare(Cft *cft, Str *filename) //{{{
{
    ASSERT_ARG(cft);
    ASSERT_ARG(filename);
    //ASSERT_ARG(btw);

#if 0
    ++btw->stats.attempts;
    btw->filename = filename;
    str_clear(&btw->ext);
    str_clear(&btw->content);
    str_clear(&btw->basename);
    vbtwlex_clear(&btw->items);
    TRYC(str_fmt_basename(&btw->basename, filename));
#endif

    str_clear(&cft->parse.content);
    str_clear(&cft->parse.extension);
    TRYC(str_fmt_ext(&cft->parse.extension, filename));

    if(file_is_dir(filename)) {
        THROW("don't expect dir!");
        TRYC(file_dir_read(filename, &cft->parse.dirfiles));
        info(INFO_parsing_directory, "Entering directory '%.*s'", STR_F(filename));
    } else {
        bool parse = false;
        Str split = {0};
        Str extensions = cft->options.extensions;
        while(split = str_splice(&extensions, &split, ','), str_iter_begin(&split) < str_iter_end(&extensions)) {
            if(!str_cmp_ci(&cft->parse.extension, &split)) {
                // TODO make a flag for this?
                parse = true;
                break;
            }
        }
        if(parse) {
            size_t size = 0;
            size_t max_file_size = 1ULL << 20;
            //size_t max_file_size = 0; // TODO:make this an argument
            if(!max_file_size || (size = file_size(filename)) <= max_file_size) {
                info(INFO_parsing_file, "parsing '%.*s'", STR_F(filename));
                TRYC(file_str_read(filename, &cft->parse.content));
                //str_trim(&btw->content);
            } else {
                info(INFO_parsing_skip_too_large, "file too large: %zu bytes, not parsing '%.*s'", size, STR_F(filename));
            }
        } else {
            info(INFO_parsing_skip_incorrect_extension, "incorrect extension '%.*s', not parsing '%.*s'", STR_F(&cft->parse.extension), STR_F(filename));
        }
    }

    return 0;
error:
    return -1;
} //}}}


ErrDecl cft_parse_file(Str *filename, void *cft_void) //{{{
{
    ASSERT_ARG(cft_void);
    ASSERT_ARG(filename);
    int err = 0;
    Cft *cft = cft_void;

    TRYC(cft_file_prepare(cft, filename));
    if(str_length(&cft->parse.content)) {
        TRYC(cft_parse(cft, filename, &cft->parse.content));
        info_check(INFO_parsing_file, true);
        //++btw->stats.success;
    }
clean:
    //btw_free(btw); // TODO: do I need to free here or not??? (asking myself in case I process multiple files in a row -> parse_exec )
    return err;
error:
    ERR_CLEAN;
    return -1;
} //}}}

ErrDecl cft_parse_exec(Str *filename, void *args) {/*{{{*/
    ASSERT_ARG(filename);
    ASSERT_ARG(args);
    Cft *cft = (Cft *)args;
    TRYC(cft_parse_file(filename, filename));
    return 0;
error:
    return -1;
}/*}}}*/

ErrDecl cft_del_duplicate_folders(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    int err = 0;
    /* .. this might sound stupid, but I'll just make a NEW lookup table, just for this! */
#if 0
    TrrTag duplicates = {0};
    //TrrTagRef duplicates = {0};
    // TODO maybe add options for all lookup table sizes ?!? I don't think the table for THIS has to
    // be SO LARGE ??? -> edit... magic number for now
    // or just use lut.h ??? pepethink
    bool clear = false;
    //TRY(trrtagref_init(&duplicates, CFT_LUT_2), ERR_LUTD_INIT);
    for(size_t ii = 0; ii < (1ULL << (cft->base.tags.width - 1)); ++ii) {
        for(size_t jj = 0; jj < cft->base.tags.buckets[ii].len; ++jj) {
            /* clear if we previously used the table */
            if(clear) {
                trrtag_clear(&duplicates);
                clear = false;
            }
            /* add tags to lookup table */
            Tag *tag = cft->base.tags.buckets[ii].items[jj];
#if 0
            //if(!tag->tags.width) TRY(trstr_init(&tag->tags, CFT_LUT_1), ERR_LUTD_INIT);
#else
            //for(size_t i = 0; i < vrstr_length(&tag->tags); ++i) {
            //    Str *tagg = vrstr_get_at(&tag->tags, i);
            if(!tag->tags.width) continue;
            for(size_t i2 = 0; i2 < (1ULL << (tag->tags.width - 1)); ++i2) {
                for(size_t j2 = 0; j2 < tag->tags.buckets[i2].len; ++j2) {
                    Str *tagg = tag->tags.buckets[i2].items[j2];
                    Str tag_search = *tagg;
                    for(;;) {
                        str_trim(&tag_search); // TODO: is this and the line below actually correct + needed ?
                        if(!str_length(&tag_search)) break;
                        TagRef ref = { .tag = tag_search };
                        TRY(trrtagref_add(&duplicates, &ref), ERR_LUTD_ADD);
                        clear = true;
                        /* check next : */
                        size_t iE = str_rch(&tag_search, ':', 0);
                        if(iE >= str_length(&tag_search)) break;
                        tag_search.last = tag_search.first + iE;
                    }
                }
            }
            /* now go over duplicates and add if not one. simple! */
            trstr_clear(&tag->tags);
            for(size_t ii = 0; ii < (1ULL << (duplicates.width - 1)); ++ii) {
                for(size_t jj = 0; jj < duplicates.buckets[ii].len; ++jj) {
                    TagRef *ref = duplicates.buckets[ii].items[jj];
                    size_t count = duplicates.buckets[ii].count[jj];
                    if(count > 1) {
                        continue;
                    }
                    TRY(trstr_add(&tag->tags, &ref->tag), ERR_VEC_PUSH_BACK);
                }
            }
#endif
        }
    }
#endif
clean:
    //trrtag_free(&duplicates);
    return err;
error:
    ERR_CLEAN;
} //}}}

/* TODO: re-work how I add tags. talking folders, specifically... don't expand them there, make some
 * other table ??? maybe ????? */
ErrDecl cft_fmt(Cft *cft, Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
#if 0
    info(INFO_formatting, "Formatting");
    int err = 0;
    VrStr tags = {0};
    VrTag filenames = {0};
    //size_t *tags_counts = 0;
    TRYC(cft_del_duplicate_folders(cft));
    trtag_dump(&cft->base.tags, &filenames.items, &filenames.last);
    vrtag_sort(&filenames); // TODO add switch
    for(size_t i = 0; i < vrtag_length(&filenames); ++i) {
        Tag *tag = vrtag_get_at(&filenames, i);
#if 1
        TRY(trstr_dump(&tag->tags, &tags.items, &tags.last), ERR_LUTD_DUMP);
        vrstr_sort(&tags, 0);
        TRYC(str_fmt(str, "%.*s", STR_F(&tag->filename)));
        for(size_t j = 0; j < vrstr_length(&tags); ++j) {
            Str *tagg = vrstr_get_at(&tags, j);
            //TRYC(str_fmt(str, "%s%.*s", cft->options.decorate && !j ? " " : ",",  STR_F(tagg)));
            TRYC(str_fmt(str, ",%.*s", STR_F(tagg)));
        }
        vrstr_free(&tags);
#else
                    size_t ii, jj;
                    TRY(trtagref_find(&cft->reverse, tag, &ii, &jj), ERR_LUTD_FIND);
                    TagRef *dump = cft->reverse.buckets[ii].items[jj];
        vrstr_sort(&tag->tags, 0);
        TRYC(str_fmt(str, "%.*s", STR_F(&tag->filename)));
        for(size_t j = 0; j < vrstr_length(&tag->tags); ++j) {
            Str *tagg = vrstr_get_at(&tag->tags, j);
            TRYC(str_fmt(str, ",%.*s", STR_F(tagg)));
        }
#endif
        TRYC(str_fmt(str, "\n"));
    }
    if(str_length(&cft->comments)) {
        TRYC(str_fmt(str, "%.*s", STR_F(&cft->comments)));
    }
    info_check(INFO_formatting, true);
clean:
    vrstr_free(&tags);
    vrtag_free(&filenames);
    return err;
error: ERR_CLEAN;
#endif
    return 0;
} //}}}

void cft_free(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    ttrstr_free(&cft->base.file_tags);
    ttrstr_free(&cft->base.tag_files);
    str_free(&cft->comments);
    str_free(&cft->misc.homedir);
    str_free(&cft->misc.current_dir);
    str_free(&cft->parse.content);
    str_free(&cft->parse.extension);
    vstr_free(&cft->parse.dirfiles);
    //trrtagref_free(&cft->all);
} //}}}

#if 0
ErrDecl cft_find_any(Cft *cft, TrrTag *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
#if 0
    Str tag = {0};
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TagRef *foundr = {0};
#if 0
        TRYC(cft_find_by_tag(cft, &foundr, &tag, false));
        if(foundr) {
            if(!foundr->filenames.width) continue;
            for(size_t ii = 0; ii < (1ULL << (foundr->filenames.width - 1)); ++ii) {
                for(size_t jj = 0; jj < foundr->filenames.buckets[ii].len; ++jj) {
                    Str *filename = foundr->filenames.buckets[ii].items[jj];
                    Tag hit = {.filename = *filename};
                    TRY(trrtag_add(found, &hit), ERR_LUTD_ADD);
                    //printf("added:%.*s\n", STR_F(filename));
                }
            }
        }
#endif
    }
#endif
    return 0;
error:
    return -1;
} //}}}
#endif

#if 0
ErrDecl cft_find_and(Cft *cft, TrrTag *found, Str *find, bool first_query) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    int err = 0;
#if 0
    TrrTag temp = {0};
    TrrTag temp_swap = {0};
    /////TRY(trrtag_init(&temp, found->width), ERR_LUTD_INIT);
    /* search... */
    Str tag = {0};
    size_t iteration = (size_t)(!first_query);
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        //printf("[%.*s]\n", STR_F(&tag));
        /* search */
        TagRef *foundr = {0};
        TRYC(cft_find_by_tag(cft, &foundr, &tag, false));
        if(foundr) {
            if(!foundr->filenames.width) continue;
#if 0
            for(size_t ii = 0; ii < (1ULL << (foundr->filenames.width - 1)); ++ii) {
                for(size_t jj = 0; jj < foundr->filenames.buckets[ii].len; ++jj) {
                    Str *filename = foundr->filenames.buckets[ii].items[jj];
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
#endif
        }
        if(iteration) {
            temp_swap = *found;
            ///////trrtag_clear(found);
            *found = temp;
            temp = temp_swap;
        } else {
            //temp_swap =
        }
        ++iteration;
    }
#endif
clean:
    //////trrtag_free(&temp_swap);
    return err;
error:
    ERR_CLEAN;
} //}}}
#endif

#if 0
ErrDecl cft_find_not(Cft *cft, TrrTag *found, Str *find, bool first_query) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
#if 0
    /* search... */
    Str tag = {0};
    if(first_query) {
        /* add all tags! */
#if 0
        for(size_t ii = 0; ii < (1ULL << (cft->base.tags.width - 1)); ++ii) {
            for(size_t jj = 0; jj < cft->base.tags.buckets[ii].len; ++jj) {
                Tag *hit = cft->base.tags.buckets[ii].items[jj];
                TRY(trrtag_add(found, hit), ERR_LUTD_ADD);
            }
        }
#endif
    }
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TagRef *foundr = {0};
        TRYC(cft_find_by_tag(cft, &foundr, &tag, false));
        if(foundr) {
            //for(size_t i = 0; i < vrstr_length(&foundr->filenames); ++i) {
#if 0
            if(!foundr->filenames.width) continue;
            for(size_t ii = 0; ii < (1ULL << (foundr->filenames.width - 1)); ++ii) {
                for(size_t jj = 0; jj < foundr->filenames.buckets[ii].len; ++jj) {
                    Str *filename = foundr->filenames.buckets[ii].items[jj];
                    Tag hit = {.filename = *filename};
                    trrtag_del(found, &hit);
                    //printf("removed:%.*s\n", STR_F(filename));
                }
            }
#endif
        }
    }
#endif
    return 0;
error:
    return -1;
} //}}}
#endif

ErrDecl cft_tags_add(Cft *cft, VrStr *files, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(files);
    ASSERT_ARG(tags);
#if 0
    if(!str_length(tags)) return 0;
    for(size_t i = 0; i < vrstr_length(files); ++i) {
        Str *file = vrstr_get_at(files, i);
        Str tag = {0};
        for(;;) {
            if(str_iter_end(&tag) >= str_iter_end(tags)) break;
            tag = str_splice(tags, &tag, ',');
            TRYC(cft_add(cft, file, &tag));
            //printff("tag [%.*s] with [%.*s]", STR_F(file), STR_F(&tag));
        }
    }
#endif
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_tags_re(Cft *cft, VrStr *files, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(files);
    ASSERT_ARG(tags);
#if 0
    //printff("HELLO");
    /* prepare retagging */
    if(!str_length(tags)) return 0;
    size_t iE = str_rch(tags, ',', 0);
    if(iE >= str_length(tags)) {
        /* TODO maybe add info here, informing that we got nothing to rename! */
        return 0;
    }
    Str tag_from = {0};
    Str tag_to = STR_I0(*tags, iE + 1);
    str_trim(&tag_to);
    if(!str_length(&tag_to)) {
        /* TODO maybe add info here, nothing gets changed */
        return 0;
    }
    /* specific files only or globally */
    if(vrstr_length(files)) {
        for(size_t i = 0; i < vrstr_length(files); ++i) {
            Str *file = vrstr_get_at(files, i);
            for(;;) {
                if(str_iter_end(&tag_from) >= str_iter_at(tags, iE)) break;
                tag_from = str_splice(tags, &tag_from, ',');
                printff("retag [%.*s] -> [%.*s]", STR_F(&tag_from), STR_F(&tag_to));
                TRYC(cft_retag(cft, file, &tag_from, &tag_to));
                //TRYC(cft_add(cft, file, &tag));
                //printff("tag [%.*s] with [%.*s]", STR_F(file), STR_F(&tag));
            }
        }
    } else {
        for(;;) {
            if(str_iter_end(&tag_from) >= str_iter_at(tags, iE)) break;
            tag_from = str_splice(tags, &tag_from, ',');
            printff("retag [%.*s] -> [%.*s]", STR_F(&tag_from), STR_F(&tag_to));
            TRYC(cft_retag(cft, 0, &tag_from, &tag_to));
        }
    }
#endif
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_fmt_substring_tags(Cft *cft, Str *out, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(tags);
    int err = 0;
    THROW("implementation missing");
clean:
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_tags_fmt(Cft *cft, Str *out, VrStr *files) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(files);
    int err = 0;

    VrTTrStrItem dump_tags = {0};
    VrTrStrItem dump_files = {0};

    /* filter only matching files */
    TTrStr base = {0};
    if(vrstr_length(files)) {
        for(size_t i = 0; i < vrstr_length(files); ++i) {
            Str *file = vrstr_get_at(files, i);
            Str splice = {0};
            for(;;) {
                splice = str_splice(file, &splice, ',');
                if(splice.first >= file->last) { break;}
                Str search = splice;
                str_trim(&search);
                TrStr *file_tag = ttrstr_get(&cft->base.file_tags, &search);
                if(!file_tag) continue;
                for(size_t j = 0; j < LUT_CAP(file_tag->width); ++j) {
                    TrStrItem *tag = file_tag->buckets[j];
                    if(!tag) continue; // TODO: this is ugly -> make iterator for lookup table
                    if(tag->hash == LUT_EMPTY) continue; // TODO: this is ugly -> make iterator for lookup table
                    TrStr *associated = ttrstr_get(&cft->base.tag_files, tag->key);
                    if(!associated) THROW(ERR_UNREACHABLE);
                    TRYG(ttrstr_set(&base, tag->key, associated));
                }
            }
        }
    } else {
        base = cft->base.tag_files;
    }

    /* format output */
    TRYG(ttrstr_dump(&base, &dump_tags.items, &dump_tags.last));
    vrttrstritem_sort(&dump_tags);
    /* optional title */
    if(cft->options.title) {
        TRYC(str_fmt(out, "Total Tags: %zu\n", vrttrstritem_length(&dump_tags)));
    }
    /* all tags */
    for(size_t i = 0; i < vrttrstritem_length(&dump_tags); ++i) {
        TTrStrItem *item = vrttrstritem_get_at(&dump_tags, i);
        Str *tag = item->key;
        TrStr *sub = item->val;
        /* optional decoration & compact */
        if(cft->options.decorate) {
            if(cft->options.compact) {
                TRYC(str_fmt(out, "%s[%zu] ", i ? " " : "", i));
            } else {
                TRYC(str_fmt(out, "[%zu] ", i));
            }
        }
        /* the actual tag */
        TRYC(str_fmt(out, "%.*s", STR_F(tag)));
        /* how many associated files there are */
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", (sub->used)));
        }
        /* associated files */
        if(cft->options.list_files) {
            TRYG(trstr_dump(sub, &dump_files.items, &dump_files.last));
            vrtrstritem_sort(&dump_files);
            for(size_t j = 0; j < vrtrstritem_length(&dump_files); ++j) {
                TrStrItem *file = vrtrstritem_get_at(&dump_files, j);
                TRYC(str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",",  STR_F(file->key)));
            }
            vrtrstritem_free(&dump_files);
        }
        /* compact output */
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrttrstritem_length(&dump_tags) ? ' ' : '\n'));
        } else {
            TRYC(str_fmt(out, "\n"));
        }
    }

clean:
    vrttrstritem_free(&dump_tags);
    vrtrstritem_free(&dump_files);
    return err;
error:
    ERR_CLEAN;
    return 0;
} //}}}

ErrDecl cft_files_fmt(Cft *cft, Str *out, VrStr *files) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(files);
#if 0
    int err = 0;
    VrTag all = {0};
    VrStr tags = {0};
    size_t *tags_counts = 0;
    TrrTag filtered = {0};
    TRYC(cft_del_duplicate_folders(cft));
    if(vrstr_length(files)) {
        //////7TRY(trrtag_init(&filtered, cft->base.tags.width), ERR_LUTD_INIT);
        for(size_t i = 0; i < vrstr_length(files); ++i) {
            Str *file = vrstr_get_at(files, i);
            Str splice = {0};
            for(;;) {
                splice = str_splice(file, &splice, ',');
                if(splice.first >= file->last) { break;}
                Tag search = { .filename = splice };
                size_t ii, jj;
                //////7if(trtag_find(&cft->base.tags, &search, &ii, &jj)) continue;
                //////7Tag *found = cft->base.tags.buckets[ii].items[jj];
                //////7TRY(trrtag_add(&filtered, found), ERR_LUTD_ADD);
            }
        }
        //////7TRY(trrtag_dump(&filtered, &all.items, 0, &all.last), ERR_LUTD_DUMP);
    } else {
        //////7TRY(trtag_dump(&cft->base.tags, &all.items, 0, &all.last), ERR_LUTD_DUMP);
    }
    /* generate output */
    vrtag_sort(&all);
    if(cft->options.title) {
        TRYC(str_fmt(out, "Total Files: %zu\n", vrtag_length(&all)));
    }
    //printf("TOTAL TAGS: %zu\n", vrtag_length(&all));
    for(size_t i = 0; i < vrtag_length(&all); ++i) {
        Tag *tag = vrtag_get_at(&all, i);
        /* format the name */
        if(cft->options.decorate) {
            if(cft->options.compact) {
                TRYC(str_fmt(out, "%s[%zu] ", i ? " " : "", i));
            } else {
                TRYC(str_fmt(out, "[%zu] ", i));
            }
        }
        TRYC(str_fmt(out, "%.*s", STR_F(&tag->filename)));
        if(cft->options.decorate) {
            //////7TRYC(str_fmt(out, " (%zu)", trstr_length(&tag->tags)));
        }
        /* format all associated tags */
        if(cft->options.list_tags) {
            //////7TRY(trstr_dump(&tag->tags, &tags.items, &tags_counts, &tags.last), ERR_LUTD_DUMP);
            vrstr_sort(&tags, tags_counts);
            for(size_t j = 0; j < vrstr_length(&tags); ++j) {
                Str *tagg = vrstr_get_at(&tags, j);
                TRYC(str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",",  STR_F(tagg)));
            }
            vrstr_free(&tags);
            free(tags_counts);
            tags_counts = 0;
        }
#if 0
        //vrstr_sort(&tag->tags);
        //printf("  [%zu] %.*s (%zu)\n", i, STR_F(&tag->tag), counts[i]);
        if(cft->options.list_tags) {
            bool first = true;
            for(size_t ii = 0; ii < (1ULL << (tag->tags.width - 1)); ++ii) {
                for(size_t jj = 0; jj < tag->tags.buckets[ii].len; ++ii) {
                    Str *tagg = tag->tags.buckets[ii].items[jj];
                    //Str *tagg = vrstr_get_at(&tag->tags, j);
                    TRYC(str_fmt(out, "%s%.*s", cft->options.decorate && first ? " " : ",",  STR_F(tagg)));
                    first = false;
                }
            }
        }
#endif
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtag_length(&all) ? ',' : '\n'));
        } else {
            TRYC(str_fmt(out, "\n"));
        }
    }
clean:
    vrtag_free(&all);
    return err;
error:
    ERR_CLEAN;
#endif
} //}}}

ErrDecl cft_find_fmt(Cft *cft, Str *out, Str *find_any, Str *find_and, Str *find_not) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(find_any);
    ASSERT_ARG(find_and);
    ASSERT_ARG(find_not);
#if 0
    int err = 0;
    TrrTag found = {0};
    VrTag results = {0};
    //////7TRY(trrtag_init(&found, 10), ERR_LUTD_INIT); // TODO switch
    bool first = true;
    if(str_length(find_any)) {
        TRYC(cft_find_any(cft, &found, find_any));
        first = false;
    }
    if(str_length(find_and)) {
        TRYC(cft_find_and(cft, &found, find_and, first));
        first = false;
    }
    if(str_length(find_not)) {
        TRYC(cft_find_not(cft, &found, find_not, first));
        first = false;
    }
    //////7TRY(trrtag_dump(&found, &results.items, 0, &results.last), ERR_LUTD_DUMP);
    vrtag_sort(&results);
    for(size_t i = 0; i < vrtag_length(&results); ++i) {
        Tag *file = vrtag_get_at(&results, i);
        if(cft->options.decorate) {
            if(cft->options.compact) {
                TRYC(str_fmt(out, "%s[%zu] ", i ? " " : "", i));
            } else {
                TRYC(str_fmt(out, "  [%zu] ", i));
            }
        }
        TRYC(str_fmt(out, "%.*s", STR_F(&file->filename)));
        size_t ii, jj;
        Tag *found = 0;
#if 0
        if(cft->options.list_tags || cft->options.decorate) {
            //////7TRY(trtag_find(&cft->base.tags, file, &ii, &jj), ERR_LUTD_FIND);
            found = cft->base.tags.buckets[ii].items[jj];
        }
        if(cft->options.list_tags) {
            //for(size_t j = 0; j < vrstr_length(&found->tags); ++j) {
            for(size_t ii = 0; ii < (1ULL << (found->tags.width - 1)); ++ii) {
                for(size_t jj = 0; jj < found->tags.buckets[ii].len; ++ii) {
                    Str *tag = found->tags.buckets[ii].items[jj];
                    TRYC(str_fmt(out, ",%.*s", STR_F(tag)));
                }
            }
        }
#endif
        if(cft->options.decorate) {
            //////7TRYC(str_fmt(out, " (%zu)", trstr_length(&found->tags)));
        }
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtag_length(&results) ? ',' : '\n'));
        } else {
            TRYC(str_fmt(out, "\n"));
        }
        //printf("%.*s\n", STR_F(&file->filename));
    }
clean:
    trrtag_free(&found);
    vrtag_free(&results);
    return err;
error:
    ERR_CLEAN;
#endif

} //}}}

