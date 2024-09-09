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
            //size_t max_file_size = 1ULL << 20; // TODO:make this an argument
            size_t max_file_size = 0;
            if(!max_file_size || (size = file_size(filename)) <= max_file_size) {
                info(INFO_parsing_file, "parsing '%.*s'", STR_F(filename));
                TRYC(file_str_read(filename, &cft->parse.content));
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
    TrTrStr asdf = {0};
    for(size_t i = 0; i < LUT_CAP(cft->base.tag_files.width); ++i) {
        TTrStrItem *item = cft->base.tag_files.buckets[i];
        if(!item) continue;
        if(item->hash == LUT_EMPTY) continue;
        Str *tag = item->key;
        TrStr *sub = item->val;
        size_t iE = str_rch(tag, ':', 0);
        if(iE >= str_length(tag)) continue;
        Str remove = STR_IE(*tag, iE);
        str_trim(&remove);
        if(!str_length(&remove)) continue;
        for(size_t j = 0; j < LUT_CAP(sub->width); ++j) {
            TrStrItem *item2 = sub->buckets[j];
            if(!item2) continue;
            if(item2->hash == LUT_EMPTY) continue;
            Str *file = item2->key;
            TrStr *modify = ttrstr_get(&cft->base.file_tags, file);
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
    for(size_t i = 0; i < vrttrstritem_length(&dump_files); ++i) {
        TTrStrItem *item = vrttrstritem_get_at(&dump_files, i);
        Str *file = item->key;
        TrStr *sub = item->val;
        TRYC(str_fmt(str, "%.*s", STR_F(file)));
        TRYG(trstr_dump(sub, &dump_tags.items, &dump_tags.last));
        vrtrstritem_sort(&dump_tags);
        for(size_t j = 0; j < vrtrstritem_length(&dump_tags); ++j) {
            TrStrItem *tag = vrtrstritem_get_at(&dump_tags, j);
            TRYC(str_fmt(str, ",%.*s", STR_F(tag->key)));
        }
        vrtrstritem_free(&dump_tags);
        TRYC(str_fmt(str, "\n"));
    }
    if(str_length(&cft->comments)) {
        TRYC(str_fmt(str, "%.*s", STR_F(&cft->comments)));
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
    str_free(&cft->parse.extension);
    vstr_free(&cft->parse.dirfiles);
} //}}}

ErrDecl cft_find_any(Cft *cft, TrTrStr *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
    Str tag = {0};
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, &tag, false));
        if(!tag_files) continue;
        TrStr *sub = tag_files->val;
        for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
            TrStrItem *item = sub->buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            Str *file = item->key;
            TrStr *data = ttrstr_get(&cft->base.file_tags, file);
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
    Str tag = {0};
    size_t iteration = (size_t)(!first_query);
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, &tag, false));
        if(!tag_files) continue;
        TrStr *sub = tag_files->val;
        for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
            TrStrItem *item = sub->buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            Str *file = item->key;
            if(iteration == 0) {
                TrStr *data = ttrstr_get(&cft->base.file_tags, file);
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
    Str tag = {0};
    if(first_query) {
        /* add all tags! */
        for(size_t i = 0; i < LUT_CAP(cft->base.file_tags.width); ++i) {
            TTrStrItem *item = cft->base.file_tags.buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            Str *file = item->key;
            TrStr *data = item->val;
            TRYG(trtrstr_set(found, file, data));
        }
    }
    /* search... */
    for(;;) {
        if(str_iter_end(&tag) >= str_iter_end(find)) break;
        tag = str_splice(find, &tag, ',');
        /* search */
        TTrStrItem *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, &tag, false));
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
        TRYC(str_fmt(out, "Total Tags: %zu\n", vrtrtrstritem_length(&dump_tags)));
    }
    /* all tags */
    for(size_t i = 0; i < vrtrtrstritem_length(&dump_tags); ++i) {
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
            TRYC(str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(&dump_tags) ? ',' : '\n')); // TODO: this ',' is not ideal
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
                TRYG(trtrstr_set(&base, &search, file_tag));
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
        TRYC(str_fmt(out, "Total Files: %zu\n", vrtrtrstritem_length(&dump_files)));
    }
    /* all files */
    for(size_t i = 0; i < vrtrtrstritem_length(&dump_files); ++i) {
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
        TRYC(str_fmt(out, "%.*s", STR_F(file)));
        /* how many associated files there are */
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", (sub->used)));
        }
        /* associated tags */
        if(cft->options.list_tags) {
            TRYG(trstr_dump(sub, &dump_tags.items, &dump_tags.last));
            vrtrstritem_sort(&dump_tags);
            for(size_t j = 0; j < vrtrstritem_length(&dump_tags); ++j) {
                TrStrItem *tag = vrtrstritem_get_at(&dump_tags, j);
                TRYC(str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",",  STR_F(tag->key)));
            }
            vrtrstritem_free(&dump_tags);
        }
        /* compact output */
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(&dump_files) ? ',' : '\n')); // TODO: this ',' is not ideal
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
    TRYG(trtrstr_dump(&found, &results.items, &results.last));
    vrtrtrstritem_sort(&results);
    for(size_t i = 0; i < vrtrtrstritem_length(&results); ++i) {
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
        TRYC(str_fmt(out, "%.*s", STR_F(file)));
        size_t ii, jj;
        if(cft->options.list_tags) {
            for(size_t j = 0; j < LUT_CAP(sub->width); ++j) {
                TrStrItem *tag = sub->buckets[j];
                if(!tag) continue;
                if(tag->hash == LUT_EMPTY) continue;
                TRYC(str_fmt(out, ",%.*s", STR_F(tag->key)));
            }
        }
        if(cft->options.decorate) {
            TRYC(str_fmt(out, " (%zu)", sub->used));
        }
        if(cft->options.compact) {
            TRYC(str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(&results) ? ',' : '\n'));
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

