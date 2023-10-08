#include "lib.h"
#include "files_finder.h"

#include <gtest/gtest.h>

using namespace std;

namespace {

const std::regex anyStringRegex(".*");

}

TEST(TestsFilesFinder, TestFilesInDir) {
    vector<string> result = FilesFinder::ListAllFilesInDirs({"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1"}, {}, 1, 1, anyStringRegex);
    ASSERT_EQ(result.size(), (size_t)4);
}

TEST(TestsFilesFinder, TestFilesWithExcludeDirs) {
    vector<string> result = FilesFinder::ListAllFilesInDirs({"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1"}, {"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/dir1.1"}, 1, 1, anyStringRegex);
    ASSERT_EQ(result.size(), (size_t)2);
    ASSERT_EQ(result[0], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/afile1.2.txt");
    ASSERT_EQ(result[1], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/file1.1.txt");
}

TEST(TestsFilesFinder, TestFilesWithBigMinSize) {
    vector<string> result = FilesFinder::ListAllFilesInDirs({"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1"}, {"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/dir1.1"}, 1, 100, anyStringRegex);
    ASSERT_EQ(result.size(), (size_t)0);
}

TEST(TestsFilesFinder, TestFilesWithRegex) {
    const std::regex regex("(file)+.*");
    vector<string> result = FilesFinder::ListAllFilesInDirs({"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1"}, {}, 1, 1, regex);
    ASSERT_EQ(result.size(), (size_t)3);
    // this one is ignored
    ASSERT_NE(result[0], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/afile1.2.txt");
    ASSERT_EQ(result[0], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/file1.1.txt");
    ASSERT_EQ(result[1], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/dir1.1/file1.1.2.txt");
    ASSERT_EQ(result[2], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/dir1.1/file1.1.1.txt");
}

TEST(TestsFilesFinder, FindDuplicates) {
    vector<string> files = {"/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/afile1.2.txt",
                            "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/file1.1.txt"};
    vector<vector<string>> result = FilesFinder::FindDuplicatesCarefully(files, 5, FilesFinder::HASH_TYPE_MD5);
    ASSERT_EQ(result.size(), (size_t)1);
    ASSERT_EQ(result[0][0], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/afile1.2.txt");
    ASSERT_EQ(result[0][1], "/Users/amullanurov/CLionProjects/cppdev-hw8-boost/dir1/file1.1.txt");
}