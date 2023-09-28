#ifndef VIS_GLOBAL_H
#define VIS_GLOBAL_H

#include "../global.h"

#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })

static const std::string visDirPath("/data/data/ir.mahdiparastesh.mergen/files/vis/");

#endif //VIS_GLOBAL_H
