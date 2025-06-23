#include "cft.h"
#include <rphii/file.h>
#include <rphii/lut.h>
#include <rphii/str.h>
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

ErrDecl cft_arg(Cft *cft, struct Arg *arg) { //{{{
#if 0
    ASSERT_ARG(cft);
    ASSERT_ARG(arg);
    /* bools */
    cft->options.decorate = (arg->parsed.decorate == SPECIFY_OPTION_YES || arg->parsed.decorate == SPECIFY_OPTION_TRUE || arg->parsed.decorate == SPECIFY_OPTION_Y);
    cft->options.query = (str_len_raw(arg->parsed.find_and) || str_len_raw(arg->parsed.find_any) || str_len_raw(arg->parsed.find_not));
    cft->options.modify = (str_len_raw(arg->parsed.tags_add) || str_len_raw(arg->parsed.tags_del) || str_len_raw(arg->parsed.tags_re));
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
#endif
    return 0;
error:
    return -1;
} //}}}

#define ERR_cft_create_if_nonexist(...) "failed creating"
ErrDecl cft_create_if_nonexist(Cft *cft, TTPStr *base, TTPStrKV **table, const Str add, bool create) { /*{{{*/
    ASSERT_ARG(cft);
    ASSERT_ARG(base);
    ASSERT_ARG(table);
    //printff("create if nonexist: [%.*s]", STR_F(add));getchar();

    TStrKV *str = tstr_get_kv(&cft->base.strings, &add);
    if(!str) {
        Str alloc = {0};
        //printff("ALLOC [%.*s]", STR_F(add));
        str_copy(&alloc, add);
        str = tstr_once(&cft->base.strings, &alloc, 0);
    }

    *table = ttpstr_get_kv(base, str->key);
    if(!*table) {
        //printff("TABLE [%.*s]", STR_F(*str->key));
        *table = ttpstr_once(base, str->key, &(TPStr){0});
    }

    //if(*table) printff("GOT [%.*s]", STR_F(add));

#if 1
#if 0
    TTStrKV *kv = ttpstr_get_kv(&cft->base.file_tags, &add);
    if(!kv) {
        printff("CREATE [%.*s]", STR_F(add));
        kv = ttpstr_once(&cft->base.file_tags, &add, 0);
    }

    TStrKV *str = tstr_get_kv(&cft->base.strings, &RSTR_STR(tag));
    if(!str) {
        Str alloc = {0};
        TRYG(rstr_copy(&alloc, &tag));
        str = tstr_once(&cft->base.strings, &alloc, 0);
    }

    tpstr_set(kv->val, str->key, 0);
    printff("TAGGED [%.*s] WITH [%.*s]", STR_F(filename), STR_F(*str->key));

#endif
#else
    /* do it */
    *table = ttpstr_get_kv(base, &add);
    printff("%p : found for '%.*s'", *table, STR_F(add));
    //printff("%p : %.*s", table, STR_F(*(*table)->key));
    Str temp = {0};
    /* first add the tag to the file-specific table */
    if(!*table) {
        if(!create) return 0;
        info(INFO_tag_create, "Creating '%.*s'", STR_F(add));
        TRYG(rstr_copy(&temp, &add));
        TRYG(ttpstr_set(base, &add, 0));
        *table = ttpstr_get_kv(base, &add);
        ASSERT(*table, ERR_UNREACHABLE);
        info_check(INFO_tag_create, true);
    }
#endif
    return 0;
error:
    return -1;
} /*}}}*/

#define ERR_cft_find_by_str(...)    "failed adding string to table"
ErrDecl cft_find_by_str(Cft *cft, TTPStr *base, TTPStrKV **table, const Str tag, bool create_if_nonexist, bool partial) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(base);
    ASSERT_ARG(table);
    /* prepare to search */
    if(!str_len_raw(tag)) return 0;
    /* now search */
    info(INFO_tag_search, "Searching '%.*s'", STR_F(tag));
    if(!partial) {
        //printff("TAG %.*s",STR_F(tag));getchar();
        TRYC(cft_create_if_nonexist(cft, base, table, tag, create_if_nonexist));
    } else {
        for(TTPStrKV **iter = ttpstr_iter_all(base, 0); iter; iter = ttpstr_iter_all(base, iter)) {
            Str *item = (*iter)->key;
            if(str_find_substr(*item, tag, cft->options.partial) < str_len_raw(*item)) {
                //printff(">> [%.*s]", STR_F(*item));
                TRYC(cft_create_if_nonexist(cft, base, table, *item, create_if_nonexist));
            }
        }
    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_add(Cft *cft, const Str filename, const Str tag) { //{{{
    //printff(">>> TAG [%.*s] with [%.*s]", STR_F(filename), STR_F(tag));
    ASSERT_ARG(cft);
    /* prepare to search */
    Str find = str_trim(tag);
    /* search if we have a matching file ... */
    TTPStrKV *file_tags = 0;
    TRYC(cft_find_by_str(cft, &cft->base.file_tags, &file_tags, filename, true, false));
    if(!file_tags) return 0; //THROW(ERR_UNREACHABLE "; should have created/found a table");
    for(;;) {
        /* prepare string */
        if(!str_len_raw(find)) break;
        /* find entry */
        TTPStrKV *tag_files = 0;
        TRYC(cft_find_by_str(cft, &cft->base.tag_files, &tag_files, find, true, false));
        if(!tag_files) return 0;
        /* cross-reference */
        //printff("TAG [%.*s] WITH [%.*s]", STR_F(*file_tags->key), STR_F(*str_tag->key));
        if(!tpstr_get(file_tags->val, tag_files->key)) {
            tpstr_set(file_tags->val, tag_files->key, 0);
        }
        //printff("TAGGED [%.*s] WITH [%.*s] (%zu)", STR_F(*file_tags->key), STR_F(*tag_files->key), cft->base.file_tags.used);
        //printff("TAG [%.*s] WITH [%.*s]", STR_F(*kv_tag->key), STR_F(*str_file->key));
        if(!tpstr_get(tag_files->val, file_tags->key)) {
            tpstr_set(tag_files->val, file_tags->key, 0);
        }
        //printff("TAGGED [%.*s] WITH [%.*s] (%zu)", STR_F(*tag_files->key), STR_F(*file_tags->key), cft->base.tag_files.used);
        /* check next : */
        size_t iE = str_rfind_ch(find, ':');
        if(iE >= str_len_raw(find)) break;
        //find.last = find.first + iE;
        find.len = iE;
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

ErrDecl cft_parse(Cft *cft, const Str input, const Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
    int err = 0;
#if 1
    size_t line_number = 0;
    info(INFO_parsing, "Parsing");
    //printff("str_len_raw %zu", str_len_raw(str));
    Str filename_real = {0};
    Str prepend = {0};
    str_fmt(&prepend, "%.*s", STR_F(input));
    TRYC(platform_expand_path(&prepend, &cft->misc.current_dir, &cft->misc.homedir));
    for(Str iter = {0}, line = {0}; str_splice(*str, &iter, '\n'); line.str ? ++line_number : line_number) {
        line = str_trim(iter);
        //str_print(line);getchar();
        for(Str csv = {0}, val = {0}, filename = {0}; str_splice(iter, &csv, ','); ) {
            val = str_trim(csv);
            //str_print(csv);getchar();
            if(!val.str) continue;
            if(!filename.str && val.str) {
                filename = val;
                if(cft->options.expand_paths && !cft->options.modify) {
                    str_clear(&filename_real);
                    str_copy(&filename_real, val);
                    TRYC(platform_expand_path(&filename_real, &prepend, &cft->misc.homedir));
                    filename = filename_real;
                }
                continue;
            }
            if(!str_len_raw(val)) continue;
            if(str_at(filename, 0) == '#') {
                /* TODO: only read comments when we plan to write out to the file again !!! */
                /* OTHER TODO: the order will get messed up... because we sort the rest
                 * again, for now... so fix that somehow (but later) */
                str_fmt(&cft->comments, "%.*s\n", STR_F(iter));
            }
            //printff(">>> TAG [%.*s] with [%.*s]", STR_F(filename), STR_F(val));getchar();
            TRYC(cft_add(cft, filename, val));
            //fflush(stdin);
            //getchar();
        }
    }
    info_check(INFO_parsing, true);
#endif
clean:
    str_free(&prepend);
    str_free(&filename_real);
    return err;
error:
    ERR_CLEAN;
} //}}}


#define ERR_cft_file_prepare(cft, filename)     "failed preparing '%.*s'", STR_F(filename)
ErrDecl cft_file_prepare(Cft *cft, Str filename) //{{{
{
    ASSERT_ARG(cft);

    str_clear(&cft->parse.content);
    str_clear(&cft->parse.extension);
    cft->parse.extension = str_get_ext(filename);
    //str_fmt_ext(&cft->parse.extension, filename);

    if(file_is_dir(filename)) {
        THROW("don't expect dir!");
        TRYC(file_dir_read(filename, &cft->parse.dirfiles));
        info(INFO_parsing_directory, "Entering directory '%.*s'", STR_F(filename));
    } else {
        bool parse = false;
        Str split = {0};
        Str extensions = cft->options.extensions;
        while(str_splice(extensions, &split, ',')) {
            if(!str_cmp_ci(cft->parse.extension, split)) {
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
            info(INFO_parsing_skip_incorrect_extension, "incorrect extension '%.*s', not parsing '%.*s'", STR_F(cft->parse.extension), STR_F(filename));
        }
    }

    return 0;
error:
    return -1;
} //}}}


ErrDecl cft_parse_file(Str filename, void *cft_void) //{{{
{
    ASSERT_ARG(cft_void);
    int err = 0;
    Cft *cft = cft_void;

    TRYC(cft_file_prepare(cft, filename));
    if(str_len_raw(cft->parse.content)) {
        //printff("FILE %.*s", STR_F(filename));return 0;
        TRYC(cft_parse(cft, filename, &cft->parse.content));
        info_check(INFO_parsing_file, true);
    }
clean:
    return err;
error:
    ERR_CLEAN;
    return -1;
} //}}}

ErrDecl cft_parse_exec(Str filename, void *args) {/*{{{*/
    ASSERT_ARG(args);
    Cft *cft = (Cft *)args;
    TRYC(cft_parse_file(filename, cft));
    return 0;
error:
    return -1;
}/*}}}*/

/* TODO: re-work how I add tags. talking folders, specifically... don't expand them there, make some
 * other table ??? maybe ????? */
ErrDecl cft_fmt(Cft *cft, Str *str) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(str);
#if 0
    int err = 0;
    info(INFO_formatting, "Formatting");
    VrTTrStrItem dump_files = {0};
    VrTrStrItem dump_tags = {0};
    TRYC(cft_del_duplicate_folders(cft));
    TRYG(ttpstr_dump(&cft->base.file_tags, &dump_files.items, &dump_files.last));
    vrttrstritem_sort(&dump_files);
    for(size_t i = 0; i < vrttrstritem_length(dump_files); ++i) {
        TTrStrItem *item = vrttrstritem_get_at(&dump_files, i);
        Str file = str_rstr(*item->key);
        TrStr *sub = item->val;
        str_fmt(str, "%.*s", STR_F(file));
        TRYG(trstr_dump(sub, &dump_tags.items, &dump_tags.last));
        vrtrstritem_sort(&dump_tags);
        for(size_t j = 0; j < vrtrstritem_length(dump_tags); ++j) {
            TrStrItem *tag = vrtrstritem_get_at(&dump_tags, j);
            printff("!!! %.*s", STR_F(*tag->key));
            str_fmt(str, ",%.*s", STR_F(*tag->key));
        }
        vrtrstritem_free(&dump_tags);
        str_fmt(str, "\n");
    }
    if(str_len_raw(cft->comments)) {
        str_fmt(str, "%.*s", STR_F(cft->comments));
    }
    info_check(INFO_formatting, true);
clean:
    vrttrstritem_free(&dump_files);
    vrtrstritem_free(&dump_tags);
    return err;
error: ERR_CLEAN;
    return 0;
#endif
} //}}}


ErrDecl cft_del_duplicate_folders(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    int err = 0;
    for(size_t i = 0; i < LUT_CAP(cft->base.tag_files.width); ++i) {
        TTPStrKV *item = cft->base.tag_files.buckets[i];
        if(!item) continue;
        if(item->hash == LUT_EMPTY) continue;
        Str tag = *item->key;
        TPStr *sub = item->val;
        size_t iE = str_rfind_ch(tag, ':');
        if(iE >= str_len_raw(tag)) continue;
        Str remove = str_trim(str_iE(tag, iE));
        if(!str_len_raw(remove)) continue;
        for(size_t j = 0; j < LUT_CAP(sub->width); ++j) {
            TPStrKV *item2 = sub->buckets[j];
            if(!item2) continue;
            if(item2->hash == LUT_EMPTY) continue;
            Str *file = item2->key;
            Str filex = str_ll(file->str, str_len_raw(*file));
            TPStr *modify = ttpstr_get(&cft->base.file_tags, &filex);
            if(!modify) THROW(ERR_UNREACHABLE);
            tpstr_del(modify, &remove);
        }
    }
clean:
    return err;
error:
    ERR_CLEAN;
} //}}}

void cft_free(Cft *cft) { //{{{
    ASSERT_ARG(cft);
    ttpstr_free(&cft->base.file_tags);
    ttpstr_free(&cft->base.tag_files);
    tstr_free(&cft->base.strings);
    str_free(&cft->comments);
    str_free(&cft->misc.homedir);
    str_free(&cft->misc.current_dir);
    str_free(&cft->parse.content);
    //str_free(&cft->parse.extension);
    array_free(cft->parse.dirfiles);
} //}}}

ErrDecl cft_find_any(Cft *cft, RTTPStr *found, Str *find) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
#if 1
    /* search... */
    Str tag = {0};
    for(tag.str = 0; str_splice(*find, &tag, ','); ) {
        if(!tag.str) continue;
        /* search */
        TTPStrKV *tag_files = 0;
        TTPStr *base = &cft->base.tag_files;
        if(!cft->options.partial) {
            TRYC(cft_create_if_nonexist(cft, base, &tag_files, tag, false));
        } else {
            for(TTPStrKV **iter = ttpstr_iter_all(base, 0); iter; iter = ttpstr_iter_all(base, iter)) {
                Str *item = (*iter)->key;
                if(str_find_substr(*item, tag, cft->options.partial) < str_len_raw(*item)) {
                    TRYC(cft_create_if_nonexist(cft, base, &tag_files, *item, false));
                    if(!tag_files) continue;
                    /* do the thing */
                    TPStr *sub = tag_files->val;
                    for(TPStrKV **iiter = tpstr_iter_all(sub, 0); iiter; iiter = tpstr_iter_all(sub, iiter)) {
                        Str *file = (*iiter)->key;
                        TPStr *data = ttpstr_get(&cft->base.file_tags, file);
                        rttpstr_once(found, file, data); // TODO: this is not ideal... (error checking)
                    }
                }
            }
        }

    }
#endif
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_find_and(Cft *cft, RTTPStr *found, Str *find, bool first_query) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    RTTPStr temp = {0};
    RTTPStr temp_swap = {0};
    int err = 0;
    Str tag = {0};
    size_t iteration = (size_t)(!first_query);
    for(tag.str = 0; str_splice(*find, &tag, ','); ) {
        if(!tag.str) continue;
        /* search */
        TTPStrKV *tag_files = 0;
        TTPStr *base = &cft->base.tag_files;
        if(!cft->options.partial) {
            TRYC(cft_create_if_nonexist(cft, base, &tag_files, tag, false));
        } else {
            for(TTPStrKV **iter = ttpstr_iter_all(base, 0); iter; iter = ttpstr_iter_all(base, iter)) {
                Str *item = (*iter)->key;
                if(str_find_substr(*item, tag, cft->options.partial) < str_len_raw(*item)) {
                    TRYC(cft_create_if_nonexist(cft, base, &tag_files, *item, false));
                    if(!tag_files) continue;
                    /* do the thing */
                    TPStr *sub = tag_files->val;
                    for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
                        TPStrKV *item = sub->buckets[i];
                        if(!item) continue;
                        if(item->hash == LUT_EMPTY) continue;
                        Str *file = item->key;
                        Str filex = str_ll(file->str, str_len_raw(*file));
                        if(iteration == 0) {
                            TPStr *data = ttpstr_get(&cft->base.file_tags, &filex);
                            rttpstr_once(found, file, data); // TODO: this is not ideal... (error checking)
                        } else {
                            TPStr *data = rttpstr_get(found, file);
                            if(data) {
                                //printff(" >> ADD [%.*s] it %zu", STR_F(*file), iteration);
                                rttpstr_once(&temp, file, data); // TODO: this is not ideal... (error checking)
                            }
                        }
                    }
                }
            }
        }
        if(iteration) {
            rttpstr_clear(found);
            temp_swap = *found;
            *found = temp;
            temp = temp_swap;
        }
        ++iteration;
    }
clean:
    rttpstr_free(&temp_swap);
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_find_not(Cft *cft, RTTPStr *found, Str *find, bool first_query) { //{{{
//ErrDecl cft_find_not(Cft *cft, TrTrStr *found, Str *find, bool first_query) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(found);
    ASSERT_ARG(find);
    /* search... */
    Str tag = {0};
    if(first_query) {
#if 0
        //TRYG(rttpstr_copy(found, &cft->base.file_tags));
#else
        /* add all tags! */
        for(size_t i = 0; i < LUT_CAP(cft->base.file_tags.width); ++i) {
            TTPStrKV *item = cft->base.file_tags.buckets[i];
            if(!item) continue;
            if(item->hash == LUT_EMPTY) continue;
            TPStr *data = item->val;
            TRYG(rttpstr_set(found, item->key, data));
        }
#endif
    }
    /* search... */
    for(tag.str = 0; str_splice(*find, &tag, ','); ) {
        if(!tag.str) continue;
        /* search */
        TTPStrKV *tag_files = 0;
        TTPStr *base = &cft->base.tag_files;
        if(!cft->options.partial) {
            TRYC(cft_create_if_nonexist(cft, base, &tag_files, tag, false));
        } else {
            for(TTPStrKV **iter = ttpstr_iter_all(base, 0); iter; iter = ttpstr_iter_all(base, iter)) {
                Str *item = (*iter)->key;
                if(str_find_substr(*item, tag, cft->options.partial) < str_len_raw(*item)) {
                    TRYC(cft_create_if_nonexist(cft, base, &tag_files, *item, false));
                    if(!tag_files) continue;
                    /* do the thing */
                    TPStr *sub = tag_files->val;
                    for(size_t i = 0; i < LUT_CAP(sub->width); ++i) {
                        TPStrKV *item = sub->buckets[i];
                        if(!item) continue;
                        if(item->hash == LUT_EMPTY) continue;
                        Str *file = item->key;
                        rttpstr_del(found, file);
                    }
                }
            }
        }

    }
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_tags_add(Cft *cft, VStr *files, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(files);
    ASSERT_ARG(tags);
#if 0
    if(!str_len_raw(*tags)) return 0;
    for(size_t i = 0; i < vstr_len(*files); ++i) {
        Str *file = vrstr_get_at(files, i);
        Str tag = {0};
        for(;;) {
            if(rstr_iter_end(&tag) >= str_iter_end(tags)) break;
            tag = str_splice(*tags, &tag, ',');
            TRYC(cft_add(cft, *file, tag));
            //printff(">>> TAG [%.*s] with [%.*s]", STR_F(*file), STR_F(tag));
        }
    }
#endif
    return 0;
error:
    return -1;
} //}}}

ErrDecl cft_tags_re(Cft *cft, VStr *files, Str *tags) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(files);
    ASSERT_ARG(tags);
    THROW("implementation missing");
    return 0;
error:
    return -1;
} //}}}

#define ERR_cft_fmt_ttpstr(...) "failed formatting table"
ErrDecl cft_fmt_ttpstr(Cft *cft, Str *out, RTTPStr *base, bool list_sub) {
    ASSERT_ARG(cft);
    ASSERT_ARG(base);
    int err = 0;
    VRTTPStrKV dump_main = {0};
    VRTPStrKV dump_sub = {0};
    bool base_has_to_be_freed = false;
    TRYG(rttpstr_dump(base, &dump_main.items, &dump_main.last));
    vrttpstrkv_sort(&dump_main);
    /* optional title */
    if(cft->options.title) {
        str_fmt(out, "Total Tags: %zu\n", vrttpstrkv_length(dump_main));
    }
    for(size_t i = 0; i < vrttpstrkv_length(dump_main); ++i) {
        RTTPStrKV *item = vrttpstrkv_get_at(&dump_main, i);
        Str *tag = item->key;
        TPStr *sub = item->val;
        /* optional decoration & compact */
        if(cft->options.decorate) {
            if(cft->options.compact) {
                str_fmt(out, "%s[%zu] ", i ? " " : "", i);
            } else {
                str_fmt(out, "[%zu] ", i);
            }
        }
        /* the actual tag */
        str_fmt(out, "%.*s", STR_F(*tag));
        /* how many associated files there are */
        if(cft->options.decorate) {
            str_fmt(out, " (%zu)", (sub->used));
        }
        /* associated files */
        if(list_sub) {
            TRYG(tpstr_dump(sub, &dump_sub.items, &dump_sub.last));
            vrtpstrkv_sort(&dump_sub);
            for(size_t j = 0; j < vrtpstrkv_length(dump_sub); ++j) {
                TPStrKV *file = vrtpstrkv_get_at(&dump_sub, j);
                str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",", STR_F(*file->key));
            }
            vrtpstrkv_free(&dump_sub);
        }
        /* compact output */
        if(cft->options.compact) {
            str_fmt(out, "%c", i + 1 < vrttpstrkv_length(dump_main) ? ',' : '\n'); // TODO: this ',' is not ideal
        } else {
            str_fmt(out, "\n");
        }
    }
clean:
    if(base_has_to_be_freed) {
        rttpstr_free(base);
    }
    vrttpstrkv_free(&dump_main);
    vrtpstrkv_free(&dump_sub);
    return err;
error:
    ERR_CLEAN;
}

ErrDecl cft_tags_fmt(Cft *cft, Str *out, VStr *files) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(files);
    int err = 0;
    RTTPStr base = {0};
    /* filter only matching files */
    if(array_len(*files)) {
        for(size_t i = 0; i < array_len(*files); ++i) {
            Str file = array_at(*files, i);
            TTPStrKV *file_tags = ttpstr_get_kv(&cft->base.file_tags, &file);
            if(!file_tags) continue;
            for(TPStrKV **iter = tpstr_iter_all(file_tags->val, 0); iter; iter = tpstr_iter_all(file_tags->val, iter)) {
                Str *tag = (*iter)->key;
                TTPStrKV *associated = ttpstr_get_kv(&cft->base.tag_files, tag);
                //printff("TAG %.*s : %zu", STR_F(tag), associated->val->used);
                TRYG(rttpstr_set(&base, associated->key, associated->val)); //tag_files->val));
            }
        }
    } else {
        base = *(RTTPStr *)&cft->base.tag_files;
    }
    TRYG(cft_fmt_ttpstr(cft, out, &base, cft->options.list_files));
clean:
    return err;
error:
    ERR_CLEAN;
} //}}}

ErrDecl cft_files_fmt(Cft *cft, Str *out, VStr *files) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(files);
    int err = 0;
#if 1
    VRTTPStrKV dump_files = {0};
    VRTPStrKV dump_tags = {0};
    RTTPStr base = {0};
    /* filter only matching files */
    bool base_has_to_be_freed = false;
    if(array_len(*files)) {
        for(size_t i = 0; i < array_len(*files); ++i) {
            Str file = array_at(*files, i);
            TTPStrKV *file_tags = ttpstr_get_kv(&cft->base.file_tags, &file);
            if(!file_tags) continue;
            TRYG(rttpstr_set(&base, file_tags->key, file_tags->val));
        }
        //base_has_to_be_freed = true;
        //return 0;
    } else {
        base = *(RTTPStr *)&cft->base.file_tags; /* should probably not assign 1:1 even if it is the exact same structure */
    }
    TRYG(rttpstr_dump(&base, &dump_files.items, &dump_files.last));
    vrttpstrkv_sort(&dump_files);
    /* optional title */
    if(cft->options.title) {
        str_fmt(out, "Total Files: %zu\n", vrttpstrkv_length(dump_files));
    }
    TRYG(cft_fmt_ttpstr(cft, out, &base, cft->options.list_tags));

#else

    VrTrTrStrItem dump_files = {0};
    VrTrStrItem dump_tags = {0};

    /* filter only matching files */
    TrTrStr base = {0};
    bool base_has_to_be_freed = false;
    if(vstr_len(*files)) {
        for(size_t i = 0; i < vstr_len(*files); ++i) {
            Str *file = vrstr_get_at(files, i);
            Str splice = {0};
            for(;;) {
                splice = str_splice(*file, &splice, ',');
                if(splice.first >= file->last) { break;}
                Str search = rstr_trim(splice);
                TrStr *file_tag = ttpstr_get(&cft->base.file_tags, &RSTR_STR(search));
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
        str_fmt(out, "Total Files: %zu\n", vrtrtrstritem_length(dump_files));
    }
    /* all files */
    for(size_t i = 0; i < vrtrtrstritem_length(dump_files); ++i) {
        TrTrStrItem *item = vrtrtrstritem_get_at(&dump_files, i);
        Str *file = item->key;
        TrStr *sub = item->val;
        /* optional decoration & compact */
        if(cft->options.decorate) {
            if(cft->options.compact) {
                str_fmt(out, "%s[%zu] ", i ? " " : "", i);
            } else {
                str_fmt(out, "[%zu] ", i);
            }
        }
        /* the actual file */
        str_fmt(out, "%.*s", STR_F(*file));
        /* how many associated files there are */
        if(cft->options.decorate) {
            str_fmt(out, " (%zu)", (sub->used));
        }
        /* associated tags */
        if(cft->options.list_tags) {
            TRYG(trstr_dump(sub, &dump_tags.items, &dump_tags.last));
            vrtrstritem_sort(&dump_tags);
            for(size_t j = 0; j < vrtrstritem_length(dump_tags); ++j) {
                TrStrItem *tag = vrtrstritem_get_at(&dump_tags, j);
                str_fmt(out, "%s%.*s", cft->options.decorate && !j ? " " : ",", *tag->key);
            }
            vrtrstritem_free(&dump_tags);
        }
        /* compact output */
        if(cft->options.compact) {
            str_fmt(out, "%c", i + 1 < vrtrtrstritem_length(dump_files) ? ',' : '\n'); // TODO: this ',' is not ideal
        } else {
            str_fmt(out, "\n");
        }
    }

#endif
clean:
    if(base_has_to_be_freed) {
        rttpstr_free(&base);
    }
    vrttpstrkv_free(&dump_files);
    vrtpstrkv_free(&dump_tags);
    return err;
error:
    printff("ERROR");
    ERR_CLEAN;
} //}}}

ErrDecl cft_find_fmt(Cft *cft, Str *out, Str *find_any, Str *find_and, Str *find_not) { //{{{
    ASSERT_ARG(cft);
    ASSERT_ARG(out);
    ASSERT_ARG(find_any);
    ASSERT_ARG(find_and);
    ASSERT_ARG(find_not);
#if 0
#else
    int err = 0;
    RTTPStr found = {0};
    VRTTPStrKV results = {0};
    bool first = true;
    if(str_len_raw(*find_any)) {
        TRYC(cft_find_any(cft, &found, find_any));
        first = false;
    }
    if(str_len_raw(*find_and)) {
        TRYC(cft_find_and(cft, &found, find_and, first));
        first = false;
    }
    if(str_len_raw(*find_not)) {
        TRYC(cft_find_not(cft, &found, find_not, first));
        first = false;
    }
    TRYG(rttpstr_dump(&found, &results.items, &results.last));
    vrttpstrkv_sort(&results);
    for(size_t i = 0; i < vrttpstrkv_length(results); ++i) {
        RTTPStrKV *item = vrttpstrkv_get_at(&results, i);
        Str *file = item->key;
        TPStr *sub = item->val;
        if(cft->options.decorate) {
            if(cft->options.compact) {
                str_fmt(out, "%s[%zu] ", i ? " " : "", i);
            } else {
                str_fmt(out, "  [%zu] ", i);
            }
        }
        str_fmt(out, "%.*s", STR_F(*file));
        if(cft->options.list_tags) {
            for(size_t j = 0; j < LUT_CAP(sub->width); ++j) {
                TPStrKV *tag = sub->buckets[j];
                if(!tag) continue;
                if(tag->hash == LUT_EMPTY) continue;
                str_fmt(out, ",%.*s", STR_F(*tag->key));
            }
        }
        if(cft->options.decorate) {
            str_fmt(out, " (%zu)", sub->used);
        }
        if(cft->options.compact) {
            str_fmt(out, "%c", i + 1 < vrttpstrkv_length(results) ? ',' : '\n');
        } else {
            str_fmt(out, "\n");
        }
    }
clean:
    vrttpstrkv_free(&results);
    rttpstr_free(&found);
    return err;
error:
    ERR_CLEAN;
#endif
} //}}}

