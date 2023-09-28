#ifndef VIS_SHORT_TERM_MEMORY_H
#define VIS_SHORT_TERM_MEMORY_H

#include <filesystem>
#include <sys/stat.h>

#include "global.h"

class ShortTermMemory {
public:
    ShortTermMemory() {
        struct stat sb;
        //memset(sb, 0, sizeof(sb));
        if (stat(visDirPath.c_str(), &sb) != 0)
            mkdir(visDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        new std::string[]{"", "shapes", "y", "u", "v"};
        //for ()
    }
};

#endif //VIS_SHORT_TERM_MEMORY_H
