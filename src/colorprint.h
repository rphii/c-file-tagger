#ifndef COLORPRHINT_H

#ifndef COLORPRINT_DISABLE
#define F(msg,fmt) "\033[" fmt "m" msg "\033[0m"

#define FG3(r,g,b)  ";38;2;" r ";" g ";" b
#define BG3(r,g,b)  ";48;2;" r ";" g ";" b

#define FS_BEG  "\033["
#define FS_FG3  ";38;2;%u;%u;%u"
#define FS_BG3  ";48;2;%u;%u;%u"
#define FS_FMT  "%s"
#define FS_END  "m%.*s\033[0m"

#define BOLD    ";1"     /* bold */
#define IT      ";3"     /* italic */
#define UL      ";4"     /* underline */

/* === foreground colors === */
#define FG_BK   ";30"    /* black */
#define FG_RD   ";31"    /* red */
#define FG_GN   ";32"    /* green */
#define FG_YL   ";33"    /* yellow */
#define FG_BL   ";34"    /* blue */
#define FG_MG   ";35"    /* magenta */
#define FG_CY   ";36"    /* cyan */
#define FG_WT   ";37"    /* white */

/* === background colors === */
#define BG_BK   ";40"    /* black */
#define BG_RD   ";41"    /* red */
#define BG_GN   ";42"    /* green */
#define BG_YL   ";43"    /* yellow */
#define BG_BL   ";44"    /* blue */
#define BG_MG   ";45"    /* magenta */
#define BG_CY   ";46"    /* cyan */
#define BG_WT   ";47"    /* white */

/* === bright foreground colors === */
#define FG_BK_B ";90"    /* black */
#define FG_RD_B ";91"    /* red */
#define FG_GN_B ";92"    /* green */
#define FG_YL_B ";93"    /* yellow */
#define FG_BL_B ";94"    /* blue */
#define FG_MG_B ";95"    /* magenta */
#define FG_CY_B ";96"    /* cyan */
#define FG_WT_B ";97"    /* white */

/* === bright background colors === */
#define BG_BK_B ";100"    /* black */
#define BG_RD_B ";101"    /* red */
#define BG_GN_B ";102"    /* green */
#define BG_YL_B ";103"    /* yellow */
#define BG_BL_B ";104"    /* blue */
#define BG_MG_B ";105"    /* magenta */
#define BG_CY_B ";106"    /* cyan */
#define BG_WT_B ";107"    /* white */

#else
#define F(msg,fmt) msg

#define FS_BEG  ""
#define FS_FG3  ""
#define FS_BG3  ""
#define FS_FMT  ""
#define FS_END  ""

#define BOLD    ""
#define IT      ""
#define UL      ""

/* === foreground colors === */
#define FG_BK
#define FG_RD
#define FG_GN
#define FG_YL
#define FG_BL
#define FG_MG
#define FG_CY
#define FG_WT

/* === background colors === */
#define BG_BK
#define BG_RD
#define BG_GN
#define BG_YL
#define BG_BL
#define BG_MG
#define BG_CY
#define BG_WT

/* === bright foreground colors === */
#define FG_BK_B
#define FG_RD_B
#define FG_GN_B
#define FG_YL_B
#define FG_BL_B
#define FG_MG_B
#define FG_CY_B
#define FG_WT_B

/* === bright background colors === */
#define BG_BK_B
#define BG_RD_B
#define BG_GN_B
#define BG_YL_B
#define BG_BL_B
#define BG_MG_B
#define BG_CY_B
#define BG_WT_B
#endif

#define COLORPRINT_H
#endif

