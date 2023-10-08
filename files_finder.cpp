#include "files_finder.h"

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

using namespace std;
using boost::uuids::detail::md5;
namespace fs = boost::filesystem;
namespace bs = boost::system;

namespace {

string CalcHash(string s, FilesFinder::HashType hashType) {
    switch (hashType) {
        case FilesFinder::HASH_TYPE_MD5: {
            md5 hash;
            md5::digest_type digest;
            hash.process_bytes(s.data(), s.size());
            hash.get_digest(digest);

            const auto intDigest = reinterpret_cast<const int*>(&digest);
            std::string result;
            boost::algorithm::hex(intDigest, intDigest + (sizeof(md5::digest_type)/sizeof(int)), std::back_inserter(result));
            return result;
        }
        case FilesFinder::HASH_TYPE_CRC32: {
            boost::crc_32_type result;
            result.process_bytes(s.data(), s.length());
            return to_string(result.checksum());
        }

    }
}

}

std::vector<std::string> FilesFinder::ListAllFilesInDirs(
    const std::vector<std::string>& dirs,
    const std::unordered_set<std::string>& dirsToExclude,
    int scanLevel, size_t minFileSize, const std::regex& regex)
{
    vector<string> files;
    for (const auto& path : dirs) {
        assert(fs::exists(path));
        fs::recursive_directory_iterator iter(path);
        fs::recursive_directory_iterator end;
        while (iter != end) {
            if (iter.depth() > scanLevel || (fs::is_directory(iter->path()) && dirsToExclude.contains(iter->path().string()))) {
                iter.no_push_pending();
                iter.disable_recursion_pending();
            } else if (fs::is_regular_file(iter->path()) && fs::file_size(iter->path()) >= (size_t)minFileSize && std::regex_match(iter->path().filename().string(), regex)) {
                files.push_back(iter->path().string());
            }

            bs::error_code ec;
            // Increment the iterator to point to next entry in recursive iteration
            iter.increment(ec);
            if (ec) {
                throw ec;
            }
        }
    }

    return files;
}

vector<vector<string>> FilesFinder::FindDuplicatesCarefully(
    const vector<string>& files,
    size_t blockSize, HashType hashType)
{
    boost::bimap<boost::bimaps::multiset_of<string>, boost::bimaps::unordered_set_of<int>> bimapHashToIdx;
    vector<int> needToRead(files.size(), 1);
    int currentPosition = 0;
    while (true) {
        bool finishedAll = true;
        for (size_t i = 0; i < files.size(); i++) {
            if (needToRead[i]) {
                ifstream f(files[i], ifstream::binary);
                f.seekg(currentPosition);
                string s;
                s.resize(blockSize);
                f.read(&s[0], blockSize);


                string hash = "";
                if (bimapHashToIdx.right.count(i) != 0) {
                    hash = bimapHashToIdx.right.at(i);
                    bimapHashToIdx.right.erase(i);
                }


                string newHash = hash + CalcHash(s, hashType);
                bimapHashToIdx.insert({hash + s, (int)i});

                if (f.peek() == EOF) {
                    continue;
                } else {
                    finishedAll = false;
                }
            }
        }
        for (const auto& [hash, i] : bimapHashToIdx) {
            if (bimapHashToIdx.left.count(hash) == 1) {
                needToRead[i] = 0;
            }
        }
        if (finishedAll) {
            break;
        }
        currentPosition += blockSize;
    }

    vector<vector<string>> groupsOfIdentical;

    bool isNew = true;
    for (const auto& [hash, i] : bimapHashToIdx) {
        if (bimapHashToIdx.left.count(hash) > 1) {
            if (isNew) {
                isNew = false;
                groupsOfIdentical.push_back({files[i]});
            } else {
                groupsOfIdentical.back().push_back(files[i]);
            }
        } else {
            isNew = true;
        }
    }

    return groupsOfIdentical;
}