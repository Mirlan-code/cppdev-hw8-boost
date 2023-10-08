#include "lib.h"
#include "files_finder.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <regex>

#include <boost/program_options.hpp>


using namespace std;
namespace po = boost::program_options;

int main (int argc, char** argv) {
    po::options_description desc("General options");

    desc.add_options()
        ("help,H", "produce help message")
        ("include_dir,I", po::value<vector<string>>(), "Include dir")
        ("exclude_dir,E", po::value<vector<string>>(), "Exclude dir")
        ("scan_level,L", po::value<int>()->default_value(0), "Level of scan")
        ("min_file_size,F", po::value<size_t>()->default_value(1), "Min file size in bytes")
        ("file_mask,M", po::value<string>(), "File mask regex")
        ("block_size,S", po::value<size_t>(), "Block size to read")
        ("hash_type,H", po::value<string>(), "Hash type");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    int scanLevel = vm["scan_level"].as<int>();
    size_t fileSize = vm["min_file_size"].as<size_t>();
    regex fileMask(".*");
    if (!vm["file_mask"].empty()) {
        fileMask = regex(vm["file_mask"].as<string>());
    }

    unordered_set<string> dirsToExclude;
    if (!vm["exclude_dir"].empty()) {
        for (const auto& path: vm["exclude_dir"].as<vector<string>>()) {
            dirsToExclude.insert(path);
        }
    }


    vector<string> files = FilesFinder::ListAllFilesInDirs(vm["include_dir"].as<vector<string>>(), dirsToExclude, scanLevel, fileSize, fileMask);


    HashType hashType;
    if (!vm["hash_type"].empty()) {
        string hashTypeStr = vm["hash_type"].as<string>();

        if (hashTypeStr == "MD5") {
            hashType = HASH_TYPE_MD5;
        } else if (hashTypeStr == "CRC32") {
            hashType = HASH_TYPE_CRC32;
        }
    }
    auto groups = FilesFinder::FindDuplicatesCarefully(files, vm["block_size"].as<size_t>(), hashType);

    for (const auto& group : groups) {
        for (const auto& file : group) {
            cout << file << endl;
        }
        cout << endl;
    }

    return 0;
}
