#ifndef VIS_SEQUENCE_FILES_H
#define VIS_SEQUENCE_FILES_H

/** Reads an entire Sequence File. */
static std::list<uint16_t> ReadIndex(const char *path);

/** Save an index in non-volatile memory and clear its data from RAM. */
template<class INT>
void SaveIndexes(std::unordered_map<INT, std::list<uint16_t>> *indexes, std::string *dir);

#endif //VIS_SEQUENCE_FILES_H
