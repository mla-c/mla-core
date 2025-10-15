//
// Created by chris on 10/15/2025.
//

#ifndef COREOS_MLA_FILE_SYSTEM_TEST_H
#define COREOS_MLA_FILE_SYSTEM_TEST_H

#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os/filesystem/mla_file_system.h"

void FileSystemIsDirectoryPathTest() {
    // Valid directory paths (must start and end with slash)
    assert_true(mla_fs_is_directory_path(mla_string("/")),
                "Root directory should be valid");
    assert_true(mla_fs_is_directory_path(mla_string("/test/")),
                "Simple directory should be valid");
    assert_true(mla_fs_is_directory_path(mla_string("/test/nested/")),
                "Nested directory should be valid");

    // Invalid directory paths
    assert_false(mla_fs_is_directory_path(mla_string("test/")),
                 "Path without leading slash should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("/test")),
                 "Path without trailing slash should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("test")),
                 "Path without slashes should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("")),
                 "Empty path should be invalid");
    assert_false(mla_fs_is_directory_path(mla_string("test/dummy.txt")),
                 "File path should be invalid");
}

void FileSystemGetParentDirectoryTest() {
    // Standard cases
    mla_string_t parent = mla_fs_get_parent_directory(mla_string("/test/dummy.txt"));
    assert_true(mla_string_equals(parent, mla_string("/test/")),
                "Parent of 'test/dummy.txt' should be 'test/'");

    parent = mla_fs_get_parent_directory(mla_string("/test/nested/file.txt"));
    assert_true(mla_string_equals(parent, mla_string("/test/nested/")),
                "Parent of 'test/nested/file.txt' should be '/test/nested/'");

    parent = mla_fs_get_parent_directory(mla_string("/root/file.txt"));
    assert_true(mla_string_equals(parent, mla_string("/root/")),
                "Parent of '/root/file.txt' should be '/root/'");

    // Edge cases
    parent = mla_fs_get_parent_directory(mla_string("file.txt"));
    assert_true(mla_string_equals(parent, mla_string_empty()),
                "Parent of 'file.txt' should be empty");

    parent = mla_fs_get_parent_directory(mla_string(""));
    assert_true(mla_string_equals(parent, mla_string_empty()),
                "Parent of empty string should be empty");

    parent = mla_fs_get_parent_directory(mla_string("/"));
    assert_true(mla_string_equals(parent, mla_string("/")),
                "Parent of root '/' should be '/'");
}

void FileSystemGetFileNameTest() {
    // Standard cases
    mla_string_t filename = mla_fs_get_file_name(mla_string("/test/dummy.txt"));
    assert_true(mla_string_equals(filename, mla_string("dummy.txt")),
                "Filename of 'test/dummy.txt' should be 'dummy.txt'");

    filename = mla_fs_get_file_name(mla_string("/test/nested/file.bin"));
    assert_true(mla_string_equals(filename, mla_string("file.bin")),
                "Filename of 'test/nested/file.bin' should be 'file.bin'");

    filename = mla_fs_get_file_name(mla_string("/root/document.pdf"));
    assert_true(mla_string_equals(filename, mla_string("document.pdf")),
                "Filename of '/root/document.pdf' should be 'document.pdf'");

    // Edge cases
    filename = mla_fs_get_file_name(mla_string("/standalone.txt"));
    assert_true(mla_string_equals(filename, mla_string("standalone.txt")),
                "Filename with root path should return whole string");

    filename = mla_fs_get_file_name(mla_string("standalone.txt"));
    assert_true(mla_string_equals(filename, mla_string("standalone.txt")),
                "Filename without path should return whole string");

    filename = mla_fs_get_file_name(mla_string(""));
    assert_true(mla_string_equals(filename, mla_string_empty()),
                "Filename of empty string should be empty");
}

void FileSystemGetFileExtensionTest() {
    // Standard cases
    mla_string_t ext = mla_fs_get_file_extension(mla_string("/test/dummy.txt"));
    assert_true(mla_string_equals(ext, mla_string("txt")),
                "Extension of 'test/dummy.txt' should be 'txt'");

    ext = mla_fs_get_file_extension(mla_string("/test/file.doc.bak"));
    assert_true(mla_string_equals(ext, mla_string("bak")),
                "Extension of 'test/file.doc.bak' should be 'bak'");

    ext = mla_fs_get_file_extension(mla_string("/root/image.PNG"));
    assert_true(mla_string_equals(ext, mla_string("PNG")),
                "Extension of '/root/image.PNG' should be 'PNG'");

    // Edge cases
    ext = mla_fs_get_file_extension(mla_string("/test/noextension"));
    assert_true(mla_string_equals(ext, mla_string_empty()),
                "File without extension should return empty");

    ext = mla_fs_get_file_extension(mla_string("/test/.hidden"));
    assert_true(mla_string_equals(ext, mla_string("hidden")),
                "Hidden file '.hidden' should return 'hidden' as extension");

    ext = mla_fs_get_file_extension(mla_string(""));
    assert_true(mla_string_equals(ext, mla_string_empty()),
                "Empty path should return empty extension");
}

void FileSystemChangeFileExtensionTest() {
    // Standard cases
    mla_string_t newPath = mla_fs_change_file_extension(mla_string("/test/dummy.txt"), mla_string("md"));
    assert_true(mla_string_equals(newPath, mla_string("/test/dummy.md")),
                "Changing 'test/dummy.txt' to 'md' should give '/test/dummy.md'");
    mla_string_destroy(newPath);

    newPath = mla_fs_change_file_extension(mla_string("/test/file.doc.bak"), mla_string("tmp"));
    assert_true(mla_string_equals(newPath, mla_string("/test/file.doc.tmp")),
                "Changing 'test/file.doc.bak' to 'tmp' should give '/test/file.doc.tmp'");
    mla_string_destroy(newPath);

    // Remove extension
    newPath = mla_fs_change_file_extension(mla_string("/test/dummy.txt"), mla_string_empty());
    assert_true(mla_string_equals(newPath, mla_string("/test/dummy")),
                "Removing extension from 'test/dummy.txt' should give 'test/dummy'");
    mla_string_destroy(newPath);

    // File without extension
    newPath = mla_fs_change_file_extension(mla_string("/test/noext"), mla_string("bin"));
    assert_true(mla_string_equals(newPath, mla_string("/test/noext.bin")),
                "Adding extension to 'test/noext' should give 'test/noext.bin'");
    mla_string_destroy(newPath);

    // No parent directory
    newPath = mla_fs_change_file_extension(mla_string("/file.txt"), mla_string("log"));
    assert_true(mla_string_equals(newPath, mla_string("/file.log")),
                "Changing 'file.txt' to 'log' should give 'file.log'");
    mla_string_destroy(newPath);
}

void FileSystemCombinePathsTest() {
    // Standard cases
    mla_string_t combined = mla_fs_combine_paths(mla_string("test"), mla_string("dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test' and 'dummy.txt' should give '/test/dummy.txt'");
    mla_string_destroy(combined);

    combined = mla_fs_combine_paths(mla_string("test/"), mla_string("dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test/' and 'dummy.txt' should give '/test/dummy.txt'");
    mla_string_destroy(combined);

    combined = mla_fs_combine_paths(mla_string("test"), mla_string("/dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test' and '/dummy.txt' should give '/test/dummy.txt'");
    mla_string_destroy(combined);

    combined = mla_fs_combine_paths(mla_string("test/"), mla_string("/dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/test/dummy.txt")),
                "Combining 'test/' and '/dummy.txt' should give '/test/dummy.txt'");
    mla_string_destroy(combined);

    // Edge cases
    combined = mla_fs_combine_paths(mla_string(""), mla_string("dummy.txt"));
    assert_true(mla_string_equals(combined, mla_string("/dummy.txt")),
                "Combining empty and 'dummy.txt' should give '/dummy.txt'");
    mla_string_destroy(combined);

    combined = mla_fs_combine_paths(mla_string("test"), mla_string(""));
    assert_true(mla_string_equals(combined, mla_string("/test")),
                "Combining 'test' and empty should give 'test'");
    mla_string_destroy(combined);

    // Multiple levels
    combined = mla_fs_combine_paths(mla_string("root/nested"), mla_string("file.txt"));
    assert_true(mla_string_equals(combined, mla_string("/root/nested/file.txt")),
                "Combining 'root/nested' and 'file.txt' should give 'root/nested/file.txt'");
    mla_string_destroy(combined);
}

void FileSystemFileExistsTest() {
    // Test non-existent file
    assert_false(mla_fs_file_exists(mla_string("/test/nonexistent.txt")),
                 "Non-existent file should return false");

    // Create a directory and file for testing
    assert_true(mla_fs_create_directory(mla_string("/testdir/")),
                "Should create test directory");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/testdir/test.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream),
                "Should create test file");

    // Test existing file
    assert_true(mla_fs_file_exists(mla_string("/testdir/test.txt")),
                "Created file should exist");

    // Cleanup
    mla_fs_delete_file(mla_string("/testdir/test.txt"));
    mla_fs_delete_directory(mla_string("/testdir/"));
}

void FileSystemDeleteFileTest() {
    // Create test file
    assert_true(mla_fs_create_directory(mla_string("/deltest/")),
                "Should create test directory");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/deltest/todelete.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream),
                "Should create file to delete");

    // Verify file exists
    assert_true(mla_fs_file_exists(mla_string("/deltest/todelete.txt")),
                "File should exist before deletion");

    // Delete file
    assert_true(mla_fs_delete_file(mla_string("/deltest/todelete.txt")),
                "Should successfully delete file");

    // Verify file no longer exists
    assert_false(mla_fs_file_exists(mla_string("/deltest/todelete.txt")),
                 "File should not exist after deletion");

    // Try deleting non-existent file
    assert_false(mla_fs_delete_file(mla_string("/deltest/nonexistent.txt")),
                 "Deleting non-existent file should fail");

    // Cleanup
    mla_fs_delete_directory(mla_string("/deltest/"));
}

void FileSystemDirectoryExistsTest() {
    // Test non-existent directory
    assert_false(mla_fs_directory_exists(mla_string("/nonexistentdir/")),
                 "Non-existent directory should return false");

    // Create directory
    assert_true(mla_fs_create_directory(mla_string("/existingdir/")),
                "Should create test directory");

    // Test existing directory
    assert_true(mla_fs_directory_exists(mla_string("/existingdir/")),
                "Created directory should exist");

    // Cleanup
    mla_fs_delete_directory(mla_string("/existingdir/"));
}

void FileSystemCreateDirectoryTest() {
    // Create simple directory
    assert_true(mla_fs_create_directory(mla_string("/newdir/")),
                "Should create new directory");
    assert_true(mla_fs_directory_exists(mla_string("/newdir/")),
                "Created directory should exist");

    // Create nested directory
    assert_true(mla_fs_create_directory(mla_string("/parent/")),
                "Should create parent directory");
    assert_true(mla_fs_create_directory(mla_string("/parent/child/")),
                "Should create child directory");
    assert_true(mla_fs_directory_exists(mla_string("/parent/child/")),
                "Nested directory should exist");

    // Cleanup
    mla_fs_delete_directory(mla_string("/parent/child/"));
    mla_fs_delete_directory(mla_string("/parent/"));
    mla_fs_delete_directory(mla_string("/newdir/"));
}

void FileSystemDeleteDirectoryTest() {
    // Create test directory
    assert_true(mla_fs_create_directory(mla_string("/toremove/")),
                "Should create directory to delete");
    assert_true(mla_fs_directory_exists(mla_string("/toremove/")),
                "Directory should exist before deletion");

    // Delete directory
    assert_true(mla_fs_delete_directory(mla_string("/toremove/")),
                "Should successfully delete directory");
    assert_false(mla_fs_directory_exists(mla_string("/toremove/")),
                 "Directory should not exist after deletion");

    // Try deleting non-existent directory
    assert_false(mla_fs_delete_directory(mla_string("/nonexistent/")),
                 "Deleting non-existent directory should fail");
}

void FileSystemListDirectoryTest() {
    // Create test directory structure
    assert_true(mla_fs_create_directory(mla_string("/listtest/")),
                "Should create test directory");
    assert_true(mla_fs_create_directory(mla_string("/listtest/subdir1/")),
                "Should create subdirectory 1");
    assert_true(mla_fs_create_directory(mla_string("/listtest/subdir2/")),
                "Should create subdirectory 2");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    mla_fs_open_file(mla_string("/listtest/file1.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    mla_fs_open_file(mla_string("/listtest/file2.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);

    // List directory contents
    mla_array_list_t<mla_string_t, mla_string_initializer> entries = mla_array_list_empty<mla_string_t, mla_string_initializer>();;
    assert_true(mla_fs_list_directory(mla_string("/listtest/"), entries),
                "Should list directory contents");

    assert_equal(mla_array_list_size(entries), (mla_size_t)4,
                 "Should have 4 entries (2 dirs + 2 files)");

    // Cleanup
    mla_fs_delete_file(mla_string("/listtest/file1.txt"));
    mla_fs_delete_file(mla_string("/listtest/file2.txt"));
    mla_fs_delete_directory(mla_string("/listtest/subdir1/"));
    mla_fs_delete_directory(mla_string("/listtest/subdir2/"));
    mla_fs_delete_directory(mla_string("/listtest/"));
}

void FileSystemListFilesTest() {
    // Create test directory with files
    assert_true(mla_fs_create_directory(mla_string("/filestest/")),
                "Should create test directory");

    mla_file_system_stream_t stream = mla_file_system_stream_empty();
    mla_fs_open_file(mla_string("/filestest/doc1.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    mla_fs_open_file(mla_string("/filestest/doc2.txt"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);
    mla_fs_open_file(mla_string("/filestest/image.png"),
                     MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream);

    // List only files
    mla_array_list_t<mla_string_t, mla_string_initializer> files = mla_array_list_empty<mla_string_t, mla_string_initializer>();
    assert_true(mla_fs_list_files(mla_string("/filestest/"), files),
                "Should list files");

    assert_equal(mla_array_list_size(files), (mla_size_t)3,
                 "Should have 3 files");

    // Cleanup
    mla_fs_delete_file(mla_string("/filestest/doc1.txt"));
    mla_fs_delete_file(mla_string("/filestest/doc2.txt"));
    mla_fs_delete_file(mla_string("/filestest/image.png"));
    mla_fs_delete_directory(mla_string("/filestest/"));
}

void FileSystemOpenFileTest() {
    assert_true(mla_fs_create_directory(mla_string("/opentest/")),
                "Should create test directory");

    // Test write mode
    mla_file_system_stream_t writeStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, writeStream),
                "Should open file in write mode");
    assert_true(mla_fs_file_exists(mla_string("/opentest/write.txt")),
                "File should exist after opening in write mode");

    // Test read mode
    mla_file_system_stream_t readStream = mla_file_system_stream_empty();;
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, readStream),
                "Should open existing file in read mode");

    // Test read/write mode
    mla_file_system_stream_t rwStream = mla_file_system_stream_empty();;
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE, rwStream),
                "Should open file in read/write mode");

    // Cleanup
    mla_fs_delete_file(mla_string("/opentest/write.txt"));
    mla_fs_delete_directory(mla_string("/opentest/"));
}

void RegisterFileSystemPathTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("IsDirectoryPath", test_category, FileSystemIsDirectoryPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetParentDirectory", test_category, FileSystemGetParentDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetFileName", test_category, FileSystemGetFileNameTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetFileExtension", test_category, FileSystemGetFileExtensionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ChangeFileExtension", test_category, FileSystemChangeFileExtensionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CombinePaths", test_category, FileSystemCombinePathsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FileExists", test_category, FileSystemFileExistsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeleteFile", test_category, FileSystemDeleteFileTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DirectoryExists", test_category, FileSystemDirectoryExistsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CreateDirectory", test_category, FileSystemCreateDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeleteDirectory", test_category, FileSystemDeleteDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ListDirectory", test_category, FileSystemListDirectoryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ListFiles", test_category, FileSystemListFilesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("OpenFile", test_category, FileSystemOpenFileTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}


#endif