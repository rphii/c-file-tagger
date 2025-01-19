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
    cft->options.query = (str_length(arg->parsed.find_and) || str_length(arg->parsed.find_any) || str_length(arg->parsed.find_not));
    cft->options.modify = (str_length(arg->parsed.tags_add) || str_length(arg->parsed.tags_del) || str_length(arg->parsed.tags_re));
    cft->options.merge = arg->parsed.merge;
    cft->options.list_tags = arg->parsed.list_tags;
    cft->options.list_files = arg->parsed.list_files;
    cft->options.title = arg->parsed.title;
    cft->options.expand_paths = arg->parsed.expand_paths;
    cft->options.compact = arg->parsed.compact;
    cft->options.recursive = arg->parsed.recursive;
    cft->options.extensions = arg->parsed.extensions;
    cft->options.partial = arg->parsed.partial;
    /* TODO ::: THIS IS SO RETARDED MAKE THIS BETTER */
    /* error checking */
    return 0;
error:
    return -1;
} //}}}

#define ERR_cft_find_by_str(...)    "failed adding string to table"
ErrDecl cft_find_by_str(Cft *cft, TTrStr *base, TTrStrItem **table, const Str tag, bool create_if_nonexist, bool partial) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(base);
    ASSERT_ARG(table);
    /* prepare to search */
    Str find = RSTR_STR(str_trim(tag));
    if(!str_length(find)) return 0;
    const Str *findx = &STR_LL(str_iter_begin(find), str_length(find));
    /* now search */
    info(INFO_tag_search, "Searching '%.*s'", STR_F(find));
    if(!partial) {
        *table = ttrstr_get_kv(base, findx);
        Str temp = {0};
        /* first add the tag to the file-specific table */
        if(!*table) {
            if(!create_if_nonexist) return 0;
            info(INFO_tag_create, "Creating '%.*s'", STR_F(find));
            str_zero(&temp);
            TRYG(str_copy(&temp, &find));
            TRYG(ttrstr_set(base, &temp, 0));
            *table = ttrstr_get_kv(base, findx);
            ASSERT(*table, ERR_UNREACHABLE);
            info_check(INFO_tag_create, true);
        }
    } else {
        for(TTrStrItem **iter = ttrstr_iter_all(base, 0); iter; iter = ttrstr_iter_all(base, iter)) {
            Str *item = (*iter)->key;
            if(str_find_substring(item, &tag) < str_length(*item)) {
                printff("%.*s", STR_F(*item));
            }
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_add(Cft *cft, const Str filename, const RStr tag) { //{{{
    ASSERT_ARG(cft);
    /* prepare to search */
    Str find = RSTR_STR(rstr_trim(tag));
    /* search if we have a matching file ... */
    TTrStrItem *file_tags = 0;
    TRYC(cft_find_by_str(cft, &cft->base.file_tags, &file_tags, filename, true, false));
    if(!file_tags) return 0; //THROW(ERR_UNREACHABLE "; should have created/found a table");
    for(;;) {
        /* prepare string */
        if(!str_length(find)) break;
        /* find entry */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, find, true, false));
        if(!tag_files) return 0; //THROW(ERR_UNREACHABLE "; should have created/found a table");
        /* cross-reference */
        trstr_set(file_tags->val, tag_files->key, 0);
        trstr_set(tag_files->val, file_tags->key, 0);
        /* check next : */
        size_t iE = str_rfind_ch(find, ':', 0);
        if(iE >= str_length(find)) break;
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

ErrDecl cft_parse(Cft *cft, const RStr input, const Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
    int err = 0;
#if 1
    size_t line_number = 0;
    info(INFO_parsing, "Parsing");
    //printff("str_length %zu", str_length(str));
    Str filename_real = {0};
    Str prepend = {0};
    TRYC(str_fmt(&prepend, "%.*s", RSTR_F(input)));
    TRYC(platform_expand_path(&prepend, &cft->misc.current_dir, &cft->misc.homedir));
    for(RStr iter = {0}, line = {0}; iter.first < str->last; line.s ? ++line_number : line_number, iter = str_splice(*str, &iter, '\n'), line = rstr_trim(iter)) {
        RStr filename = {0};
        RStr tag = {0};
        for(;;) {
            tag = rstr_splice(line, &tag, ',');
            //printff("TAG %zu..%zu:'%.*s'", tag.first, tag.last, STR_F(&tag));
            //printff("[%zu..%zu] %.*s", line.first, line.last, STR_F(&line));getchar();
            if(!filename.s) {
                /* first entry is the filename */
                filename = rstr_trim(tag);
                if(rstr_length(filename)) {
                    if(rstr_get_front(&filename) == '#') {
                        /* TODO: only read comments when we plan to write out to the file again !!! */
                        /* OTHER TODO: the order will get messed up... because we sort the rest
                         * again, for now... so fix that somehow (but later) */
                        TRYC(str_fmt(&cft->comments, "%.*s\n", RSTR_F(line)));
                        break;
                    }
                    /* if we expand, fix filename */
                    if(cft->options.expand_paths && !cft->options.modify) {
                        str_clear(&filename_real);
                        TRYC(str_fmt(&filename_real, "%.*s", RSTR_F(filename)));
                        TRYC(platform_expand_path(&filename_real, &prepend, &cft->misc.homedir));
                    } else {
                        filename_real = RSTR_STR(filename);
                    }
                }
            } else {
                //printff(">>> TAG [%.*s] with [%.*s]", STR_F(filename_real), RSTR_F(tag));
                TRYC(cft_add(cft, filename_real, tag));
            }
            if(tag.first >= line.last) { break;}
        }
        line.first = 1 + line.last;
        if(line.first >= str_length(*str)) break;
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


#define ERR_cft_file_prepare(cft, filename)     "failed preparing '%.*s'", RSTR_F(filename)
ErrDecl cft_file_prepare(Cft *cft, RStr filename) //{{{
{
    ASSERT_ARG(cft);

    str_clear(&cft->parse.content);
    rstr_clear(&cft->parse.extension);
    cft->parse.extension = rstr_get_ext(filename);
    //TRYC(str_fmt_ext(&cft->parse.extension, filename));

    if(file_is_dir(filename)) {
        THROW("don't expect dir!");
        TRYC(file_dir_read(filename, &cft->parse.dirfiles));
        info(INFO_parsing_directory, "Entering directory '%.*s'", RSTR_F(filename));
    } else {
        bool parse = false;
        RStr split = {0};
        Str extensions = cft->options.extensions;
        while(split = str_splice(extensions, &split, ','), rstr_iter_begin(split) < str_iter_end(&extensions)) {
            if(!rstr_cmp_ci(&cft->parse.extension, &split)) {
                // TODO make a flag for this?
                parse = true;
                break;
            }
        }
        if(parse) {
            size_t size = 0;
            //size_t max_file_size = 1ULL << 20; // TODO:make this an argument
            size_t max_file_size = 0;
            if(!max_file_size || (size = file_size(filename)) <= max_file_size) {
                info(INFO_parsing_file, "parsing '%.*s'", RSTR_F(filename));
                TRYC(file_str_read(filename, &cft->parse.content));
            } else {
                info(INFO_parsing_skip_too_large, "file too large: %zu bytes, not parsing '%.*s'", size, RSTR_F(filename));
            }
        } else {
            info(INFO_parsing_skip_incorrect_extension, "incorrect extension '%.*s', not parsing '%.*s'", RSTR_F(cft->parse.extension), RSTR_F(filename));
        }
    }

    return 0;
error:
    return -1;
} //}}}


ErrDecl cft_parse_file(RStr filename, void *cft_void) //{{{
{
    ASSERT_ARG(cft_void);
    int err = 0;
    Cft *cft = cft_void;

    TRYC(cft_file_prepare(cft, filename));
    if(str_length(cft->parse.content)) {
        //printff("FILE %.*s", RSTR_F(filename));return 0;
        TRYC(cft_parse(cft, filename, &cft->parse.content));
        info_check(INFO_parsing_file, true);
    }
clean:
    return err;
error:
    ERR_CLEAN;
    return -1;
} //}}}

ErrDecl cft_parse_exec(RStr filename, void *args) {/*{{{*/
    ASSERT_ARG(args);
    Cft *cft = (Cft *)args;
    TRYC(cft_parse_file(filename, cft));
    return 0;
error:
    return -1;
}/*}}}*/

ErrDecl cft_del_duplicate_folders(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    int err = 0;
    for(size_t i = 0; i < LUT_CAP(cft->base.tag_files.width); ++i) {
        TTrStrItem *item = cft->base.tag_files.buckets[i];
        if(!item) continue;
        if(item->hash == LUT_EMPTY) continue;
        RStr tag = str_rstr(*item->key);
        TrStr *sub = item->val;
        size_t iE = rstr_rfind_ch(tag, ':', 0);
        if(iE >= rstr_length(tag)) continue;
        Str remove = RSTR_STR(rstr_trim(RSTR_IE(tag, iE)));
        if(!str_length(remove)) continue;
        for(size_t j = 0; j < LUT_CAP(sub->width); ++j) {
            TrStrItem *item2 = sub->buckets[j];
            if(!item2) continue;
            if(item2->hash == LUT_EMPTY) continue;
            Str *file = item2->key;
            Str filex = STR_LL(str_iter_begin(*file), str_length(*file));
            TrStr *modify = ttrstr_get(&cft->base.file_tags, &filex);
            if(!modify) THROW(ERR_UNREACHABLE);
            trstr_del(modify, &remove);
        }
    }
clean:
    return err;
error:
    ERR_CLEAN;
} //}}}

/* TODO: re-work how I add tags. talking folders, specifically... don't expand them there, make some
 * other table ??? maybe ????? */
ErrDecl cft_fmt(Cft *cft, Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
    int err = 0;
    info(INFO_formatting, "Formatting");
    VrTTrStrItem dump_files = {0};
    VrTrStrItem dump_tags = {0};
    TRYC(cft_del_duplicate_folders(cft));
    TRYG(ttrstr_dump(&cft->base.file_tags, &dump_files.items, &dump_files.last));
    vrttrstritem_sort(&dump_files);
    for(size_t i = 0; i < vrttrstritem_length(dump_files); ++i) {
        TTrStrItem *item = vrttrstritem_get_at(&dump_files, i);
        RStr file = str_rstr(*item->key);
        TrStr *sub = item->val;
        TRYC(str_fmt(str, "%.*s", RSTR_F(file)));
        TRYG(trstr_dump(sub, &dump_tags.items, &dump_tags.last));
        vrtrstritem_sort(&dump_tags);
        for(size_t j = 0; j < vrtrstritem_length(dump_tags); ++j) {
            TrStrItem *tag = vrtrstritem_get_at(&dump_tags, j);
            TRYC(str_fmt(str, ",%.*s", STR_F(*tag->key)));
        }
        vrtrstritem_free(&dump_tags);
        TRYC(str_fmt(str, "\n"));
    }
    if(str_length(cft->comments)) {
        TRYC(str_fmt(str, "%.*s", STR_F(cft->comments)));
    }
    info_check(INFO_formatting, true);
clean:
    vrttrstritem_free(&dump_files);
    vrtrstritem_free(&dump_tags);
    return err;
error: ERR_CLEAN;
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
    //str_free(&cft->parse.extension);
    vstr_free(&cft->parse.dirfiles);
} //}}}

ErrDecl cft_find_any(Cft *cft, TrTrStr *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
    RStr tag = {0};
    for(;;) {
        if(rstr_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(*find, &tag, ',');
        /* search */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, RSTR_STR(tag), false, cft->options.partial));
        if(!tag_files) continue;
        TrStr *sub = tag_files->val;
        for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
            TrStrItem *item = sub->buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            Str *file = item->key;
            Str filex = STR_LL(str_iter_begin(*file), str_length(*file));
            TrStr *data = ttrstr_get(&cft->base.file_tags, &filex);
            TRYG(trtrstr_set(found, file, data));
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_find_and(Cft *cft, TrTrStr *found, Str *find, bool first_query) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    TrTrStr temp = {0};
    TrTrStr temp_swap = {0};
    int err = 0;
    RStr tag = {0};
    size_t iteration = (size_t)(!first_query);
    for(;;) {
        if(rstr_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(*find, &tag, ',');
        /* search */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, RSTR_STR(tag), false, cft->options.partial));
        if(!tag_files) continue;
        TrStr *sub = tag_files->val;
        for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
            TrStrItem *item = sub->buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            Str *file = item->key;
            Str filex = STR_LL(str_iter_begin(*file), str_length(*file));
            if(iteration == 0) {
                TrStr *data = ttrstr_get(&cft->base.file_tags, &filex);
                TRYG(trtrstr_set(found, file, data));
            } else {
                TrStr *data = trtrstr_get(found, file);
                if(data) {
                    TRYG(trtrstr_set(&temp, file, data));
                }
            }
        }
        if(iteration) {
            trtrstr_clear(found);
            temp_swap = *found;
            *found = temp;
            temp = temp_swap;
        }
        ++iteration;
    }
clean:
    trtrstr_free(&temp_swap);
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_find_not(Cft *cft, TrTrStr *found, Str *find, bool first_query) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
    RStr tag = {0};
    if(first_query) {
        /* add all tags! */
        for(size_t i = 0; i < LUT_CAP(cft->base.file_tags.width); ++i) {
            TTrStrItem *item = cft->base.file_tags.buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            RStr file = str_rstr(*item->key);
            TrStr *data = item->val;
            TRYG(trtrstr_set(found, &RSTR_STR(file), data));
        }
    }
    /* search... */
    for(;;) {
        if(rstr_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(*find, &tag, ',');
        /* search */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, RSTR_STR(tag), false, cft->options.partial));
        if(!tag_files) continue;
        TrStr *sub = tag_files->val;
        for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
            TrStrItem *item = sub->buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            Str *file = item->key;
            trtrstr_del(found, file);
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_tags_add(Cft *cft, VrStr *files, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(files);
    ASSERT_ARG(tags);
    if(!str_length(*tags)) return 0;
    for(size_t i = 0; i < vrstr_length(*files); ++i) {
        Str *file = vrstr_get_at(files, i);
        RStr tag = {0};
        for(;;) {
            if(rstr_iter_end(&tag) >= str_iter_end(tags)) break;
            tag = str_splice(*tags, &tag, ',');
            TRYC(cft_add(cft, *file, tag));
            //printff(">>> TAG [%.*s] with [%.*s]", STR_F(*file), RSTR_F(tag));
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_tags_re(Cft *cft, VrStr *files, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(files);
    ASSERT_ARG(tags);
    THROW("implementation missing");
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
    VrTrTrStrItem dump_tags = {0};
    VrTrStrItem dump_files = {0};

    /* filter only matching files */
    TrTrStr base = {0};
    bool base_has_to_be_freed = false;
    if(vrstr_length(*files)) {
        for(size_t i = 0; i < vrstr_length(*files); ++i) {
            Str *file = vrstr_get_at(files, i);
            RStr splice = {0};
            for(;;) {
                splice = str_splice(*file, &splice, ',');
                if(splice.first >= file->last) { break;}
                RStr search = rstr_trim(splice);
                Str searchx = STR_LL(rstr_iter_begin(search), rstr_length(search));
                TrStr *file_tag = ttrstr_get(&cft->base.file_tags, &searchx);
                if(!file_tag) continue;
                for(size_t j = 0; j < LUT_CAP(file_tag->width); ++j) {
                    TrStrItem *tag = file_tag->buckets[j];
                    if(!tag) continue; // TODO: this is ugly -> make iterator for lookup table
                    if(tag->hash == LUT_EMPTY) continue; // TODO: this is ugly -> make iterator for lookup table
                    TrStr *associated = ttrstr_get(&cft->base.tag_files, tag->key);
                    if(!associated) THROW(ERR_UNREACHABLE);
                    TRYG(trtrstr_set(&base, tag->key, associated));
                }
            }
        }
        base_has_to_be_freed = true;
    } else {
        memcpy(&base, &cft->base.tag_files, sizeof(base)); // TODO: probably should not memcpy, even if the struct in theory should be the exact same
        //base = cft->base.tag_files;
    }

    /* format output */
    TRYG(trtrstr_dump(&base, &dump_tags.items, &dump_tags.last));
    vrtrtrstritem_sort(&dump_tags);
    /* optional title */
    if(cft->options.title) {
        TRYC(str_fmt(out, "Total Tags: %zu\n", vrtrtrstritem_length(dump_tags)));
    }
    /* all tags */
    for(size_t i = 0; i < vrtrtrstritem_length(dump_tags); ++i) {
        TrTrStrItem *item = vrtrtrstritem_get_at(&dump_tags, i);
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
        TRYC(str_fmt(out, "%.*s", STR_F(*tag)));
        /* how many associated files there are */
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", (sub->used)));
        }
        /* associated files */
        if(cft->options.list_files) {
            TRYG(trstr_dump(sub, &dump_files.items, &dump_files.last));
            vrtrstritem_sort(&dump_files);
            for(size_t j = 0; j < vrtrstritem_length(dump_files); ++j) {
                TrStrItem *file = vrtrstritem_get_at(&dump_files, j);
                TRYC(str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",", STR_F(*file->key)));
            }
            vrtrstritem_free(&dump_files);
        }
        /* compact output */
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(dump_tags) ? ',' : '\n')); // TODO: this ',' is not ideal
        } else {
            TRYC(str_fmt(out, "\n"));
        }
    }

clean:
    if(base_has_to_be_freed) {
        trtrstr_free(&base);
    }
    vrtrtrstritem_free(&dump_tags);
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
    int err = 0;
    VrTrTrStrItem dump_files = {0};
    VrTrStrItem dump_tags = {0};

    /* filter only matching files */
    TrTrStr base = {0};
    bool base_has_to_be_freed = false;
    if(vrstr_length(*files)) {
        for(size_t i = 0; i < vrstr_length(*files); ++i) {
            Str *file = vrstr_get_at(files, i);
            RStr splice = {0};
            for(;;) {
                splice = str_splice(*file, &splice, ',');
                if(splice.first >= file->last) { break;}
                RStr search = rstr_trim(splice);
                TrStr *file_tag = ttrstr_get(&cft->base.file_tags, &RSTR_STR(search));
                if(!file_tag) continue;
                TRYG(trtrstr_set(&base, &RSTR_STR(search), file_tag));
            }
        }
        base_has_to_be_freed = true;
    } else {
        memcpy(&base, &cft->base.file_tags, sizeof(base)); // TODO: probably should not memcpy, even if the struct in theory should be the exact same
        //base = cft->base.file_tags;
    }

    /* format output */
    TRYG(trtrstr_dump(&base, &dump_files.items, &dump_files.last));
    vrtrtrstritem_sort(&dump_files);
    /* optional title */
    if(cft->options.title) {
        TRYC(str_fmt(out, "Total Files: %zu\n", vrtrtrstritem_length(dump_files)));
    }
    /* all files */
    for(size_t i = 0; i < vrtrtrstritem_length(dump_files); ++i) {
        TrTrStrItem *item = vrtrtrstritem_get_at(&dump_files, i);
        Str *file = item->key;
        TrStr *sub = item->val;
        /* optional decoration & compact */
        if(cft->options.decorate) {
            if(cft->options.compact) {
                TRYC(str_fmt(out, "%s[%zu] ", i ? " " : "", i));
            } else {
                TRYC(str_fmt(out, "[%zu] ", i));
            }
        }
        /* the actual file */
        TRYC(str_fmt(out, "%.*s", STR_F(*file)));
        /* how many associated files there are */
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", (sub->used)));
        }
        /* associated tags */
        if(cft->options.list_tags) {
            TRYG(trstr_dump(sub, &dump_tags.items, &dump_tags.last));
            vrtrstritem_sort(&dump_tags);
            for(size_t j = 0; j < vrtrstritem_length(dump_tags); ++j) {
                TrStrItem *tag = vrtrstritem_get_at(&dump_tags, j);
                TRYC(str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",", *tag->key));
            }
            vrtrstritem_free(&dump_tags);
        }
        /* compact output */
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(dump_files) ? ',' : '\n')); // TODO: this ',' is not ideal
        } else {
            TRYC(str_fmt(out, "\n"));
        }
    }

clean:
    if(base_has_to_be_freed) {
        trtrstr_free(&base);
    }
    vrtrtrstritem_free(&dump_files);
    vrtrstritem_free(&dump_tags);
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_find_fmt(Cft *cft, Str *out, Str *find_any, Str *find_and, Str *find_not) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(find_any);
    ASSERT_ARG(find_and);
    ASSERT_ARG(find_not);
    int err = 0;
    TrTrStr found = {0};
    VrTrTrStrItem results = {0};
    bool first = true;
    if(str_length(*find_any)) {
        TRYC(cft_find_any(cft, &found, find_any));
        first = false;
    }
    if(str_length(*find_and)) {
        TRYC(cft_find_and(cft, &found, find_and, first));
        first = false;
    }
    if(str_length(*find_not)) {
        TRYC(cft_find_not(cft, &found, find_not, first));
        first = false;
    }
    TRYG(trtrstr_dump(&found, &results.items, &results.last));
    vrtrtrstritem_sort(&results);
    for(size_t i = 0; i < vrtrtrstritem_length(results); ++i) {
        TrTrStrItem *item = vrtrtrstritem_get_at(&results, i);
        Str *file = item->key;
        TrStr *sub = item->val;
        if(cft->options.decorate) {
            if(cft->options.compact) {
                TRYC(str_fmt(out, "%s[%zu] ", i ? " " : "", i));
            } else {
                TRYC(str_fmt(out, "  [%zu] ", i));
            }
        }
        TRYC(str_fmt(out, "%.*s", STR_F(*file)));
        if(cft->options.list_tags) {
            for(size_t j = 0; j < LUT_CAP(sub->width); ++j) {
                TrStrItem *tag = sub->buckets[j];
                if(!tag) continue;
                if(tag->hash == LUT_EMPTY) continue;
                TRYC(str_fmt(out, ",%.*s", tag->key));
            }
        }
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", sub->used));
        }
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(results) ? ',' : '\n'));
        } else {
            TRYC(str_fmt(out, "\n"));
        }
    }
clean:
    vrtrtrstritem_free(&results);
    trtrstr_free(&found);
    return err;
error:
    ERR_CLEAN;
} //}}}

