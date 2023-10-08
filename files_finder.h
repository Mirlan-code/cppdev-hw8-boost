#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <regex>

namespace FilesFinder {

enum HashType {
    HASH_TYPE_MD5 = 0,
    HASH_TYPE_CRC32 = 1,
};

std::vector<std::string> ListAllFilesInDirs(
    const std::vector<std::string>& dirs,
    const std::unordered_set<std::string>& dirsToExclude,
    int scanLevel, size_t minFileSize, const std::regex& regex);

std::vector<std::vector<std::string>> FindDuplicatesCarefully(
    const std::vector<std::string>& files,
    size_t blockSize, HashType hashType);

};

