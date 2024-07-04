#ifndef PARSE_H

#include "err.h"
#include "str.h"
#include "cft.h"

ErrDecl parse(Cft *cft, const Str *filename);

#define PARSE_H
#endif

