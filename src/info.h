#ifndef INFO_H

#include <stdbool.h>
#include "err.h"
#include "str.h"

typedef enum {
    INFO_NONE, /* ids below */

    INFO_syscmd_failed,
    INFO_skipping_nofile_nodir,
    INFO_parsing,
    INFO_input,
    INFO_output,
    INFO_tag_search,
    INFO_tag_created,
    INFO_filename_search,
    INFO_filename_created,
    INFO_tag_done,
    INFO_tag_found_count,
    INFO_formatting,
    INFO_parsing_directory,
    INFO_parsing_file,
    INFO_parsing_skip_too_large,
    INFO_parsing_skip_incorrect_extension,

    INFO__COUNT /* ids above */
} InfoList;

typedef enum {
    INFO_LEVEL_TEXT         = 0x01,
    INFO_LEVEL_IS_INFO      = 0x02,
    INFO_LEVEL_FILE_LINE    = 0x04,
    INFO_LEVEL_FUNCTION     = 0x08,
    INFO_LEVEL_ID           = 0x10,
    /* handy stuff */
    INFO_LEVEL_ALL          = 0xFFFF
} InfoLevelField;

typedef enum {
    INFO_TYPE_TEXT,
    INFO_TYPE_CHECK,
    INFO_TYPE_LOADING,
} InfoTypeList;

typedef enum {
    INFO_STATUS_NONE,
    INFO_STATUS_PENDING,
    INFO_STATUS_SUCCESS,
    INFO_STATUS_FAILURE,
} InfoStatusList;

typedef struct Info {
    InfoLevelField disabled[INFO__COUNT];
    InfoStatusList status[INFO__COUNT];
    Str info_last[INFO__COUNT];
    InfoList id_prev;
} Info;

void info_handle_abort(void);
void info_handle_prev(InfoList current);
void info_handle_end(InfoList id);
Str *info_query_last(InfoList id);

/* The # operator converts symbol 'v' into a string */
#define STRINGIFY0(v) #v
#define STRINGIFY(v) STRINGIFY0(v)

#define info(id, str, ...)  do {\
        InfoLevelField disabled = info_query_disabled(id); \
        if(disabled & INFO_LEVEL_TEXT) break; /* like.. if no text -> break entirely */ \
        info_handle_prev(id); \
        Str *last = info_query_last(id); \
        str_clear(last); \
        bool decorators = false; \
        if(~disabled & INFO_LEVEL_IS_INFO) { \
            (void)(str_fmt(last, F("[INFO] ", FG_YL_B BOLD))); \
        } \
        if(~disabled & INFO_LEVEL_ID) { \
            (void)(str_fmt(last, F("<%s> ", FG_BL_B BOLD), STRINGIFY(id))); \
        } \
        if(~disabled & INFO_LEVEL_FILE_LINE) { \
            (void)(str_fmt(last, F("%s%s:%i", FG_WT_B), decorators ? ":" : "", __FILE__, __LINE__)); \
            decorators = true; \
        } \
        if(~disabled & INFO_LEVEL_FUNCTION) { \
            (void)(str_fmt(last, F("%s%s", FG_WT_B), decorators ? ":" : "", __func__)); \
            decorators = true; \
        } \
        (void)(str_fmt(last, "%s", decorators ? " " : "")); \
        decorators = false; \
        (void)(str_fmt(last, str, ##__VA_ARGS__)); \
        ERR_PRINTF("%.*s", STR_F(last)); \
        info_handle_end(id); \
    } while(0)

InfoLevelField info_query_disabled(InfoList id);
void info_check(InfoList id, bool status);
void info_disable(InfoList id, InfoLevelField field);
void info_disable_all(InfoLevelField field);
void info_enable_all(InfoLevelField field);

void info_enable(InfoList id, InfoLevelField field);

#define INFO_H
#endif

