#include "info.h"
#include "platform.h"
#include "str.h"
#include <stdarg.h>

static Info s_info; /* I hate public variables ... */

InfoTypeList info_query_type(InfoList id) {
    switch(id) {
        case INFO_formatting: return INFO_TYPE_CHECK;
        case INFO_parsing: return INFO_TYPE_CHECK;
        case INFO_parsing_file: return INFO_TYPE_CHECK;
        default: return INFO_TYPE_TEXT;
    }
}

Str *info_query_last(InfoList id) {
    ASSERT(id < INFO__COUNT, "id (%u) > COUNT (%u)", id, INFO__COUNT);
    return &s_info.info_last[id];
}

void info_handle_end(InfoList id) {
    if(id != INFO_NONE) {
        switch(info_query_type(id)) {
            case INFO_TYPE_TEXT: { ERR_PRINTF("\n"); } break;
            case INFO_TYPE_CHECK: {
                s_info.status[id] = INFO_STATUS_PENDING;
                ERR_PRINTF(" " F("..", FG_BK_B) " ");
            } break;
            case INFO_TYPE_LOADING: { ABORT("not implemented"); }
        }
    }
    s_info.id_prev = id;
}

void info_handle_abort(void) {
    info_handle_prev(INFO_NONE);
    for(size_t id = 0; id < INFO__COUNT; ++id) {
        InfoTypeList type = info_query_type(id);
        if(type == INFO_TYPE_CHECK && s_info.status[id] != INFO_STATUS_NONE) {
            info_check(id, false);
        }
    }
    info_handle_end(INFO_NONE);
}

void info_handle_prev(InfoList id) {
    bool output = true;
    if((info_query_disabled(s_info.id_prev) & INFO_LEVEL_TEXT)) output = false;
    switch(info_query_type(s_info.id_prev)) {
        case INFO_TYPE_TEXT: break;
        case INFO_TYPE_CHECK: {
            if(s_info.id_prev == id) {
                //info_check(id, false);
            } else if(s_info.status[s_info.id_prev] == INFO_STATUS_PENDING) {
                if(output) ERR_PRINTF(F("(?)", FG_BL_B) "\n");
            }
        } break;
        case INFO_TYPE_LOADING: { ABORT("not implemented"); }
    }
    switch(info_query_type(id)) {
        case INFO_TYPE_TEXT: break;
        case INFO_TYPE_CHECK: {
            //s_info.status[id] = INFO_STATUS_PENDING;
            if(s_info.status[id] == INFO_STATUS_PENDING) {
                info_check(id, false);
            }
        } break;
        case INFO_TYPE_LOADING: { ABORT("not implemented"); }
    }
}

void info_check(InfoList id, bool status) {
    //InfoTypeList now = info_query_type(id);
    bool output = true;
    if((info_query_disabled(id) & INFO_LEVEL_TEXT)) output = false;
    //platform_trace();
    InfoTypeList type = info_query_type(id);
    switch(type) {
        case INFO_TYPE_CHECK: {
            //if(s_info.status[id] != INFO_STATUS_NONE) {
                s_info.status[id] = status ? INFO_STATUS_SUCCESS : INFO_STATUS_FAILURE;
                if(output) {
                    char *buf = status ? F("ok", FG_GN_B) : F("fail", FG_RD_B);
                    if(s_info.id_prev != id) {
                        ERR_PRINTF("%.*s " F("..", FG_BK_B) " " F("(!)", FG_BL_B) " ", STR_F(info_query_last(id)));
                    }
                    ERR_PRINTF("%s\n", buf);
                }
                s_info.status[id] = INFO_STATUS_NONE;
            //}
        } break;
        default: break;
    }
}

InfoLevelField info_query_disabled(InfoList id) {
    InfoLevelField result = {0};
    if(id < INFO__COUNT) {
        result = s_info.disabled[id];
    }
    return result;
}

void info_disable(InfoList id, InfoLevelField field) {
    if(id < INFO__COUNT) {
        s_info.disabled[id] |= field;
    }
}

void info_disable_all(InfoLevelField field) {
    for(InfoList id = 0; id < INFO__COUNT; ++id) {
        s_info.disabled[id] |= field;
    }
}

void info_enable(InfoList id, InfoLevelField field) {
    if(id < INFO__COUNT) {
        s_info.disabled[id] &= ~field;
    }
}

void info_enable_all(InfoLevelField field) {
    for(InfoList id = 0; id < INFO__COUNT; ++id) {
        s_info.disabled[id] &= ~field;
    }
}



