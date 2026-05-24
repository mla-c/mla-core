//
// Created by chris on 10/15/2025.
//

#ifndef MLA_FILE_SYSTEM_TEST_H
#define MLA_FILE_SYSTEM_TEST_H

#include "../core-test-support/mla_test_executor.h"
#include "../core-test-support/mla_benchmark_executor.h"
#include "../core/filesystem/mla_file_system.h"

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
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

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
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

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
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

    // List directory contents
    mla_array_list_t<mla_string_t, mla_string_initializer> entries = mla_array_list_empty<mla_string_t, mla_string_initializer>();;
    assert_true(mla_fs_list_directory(mla_string("/listtest/"), entries),
                "Should list directory contents");

    assert_equal(mla_array_list_size(entries), (mla_size_t)2,
                 "Should have 4 entries (2 dirs)");

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
    // Close the stream
    stream = mla_file_system_stream_empty();
    (void)stream;

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
    // Close the stream
    writeStream = mla_file_system_stream_empty();
    (void)writeStream;

    // Test read mode
    mla_file_system_stream_t readStream = mla_file_system_stream_empty();;
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, readStream),
                "Should open existing file in read mode");
    // Close the stream
    readStream = mla_file_system_stream_empty();
    (void)readStream;

    // Test read/write mode
    mla_file_system_stream_t rwStream = mla_file_system_stream_empty();;
    assert_true(mla_fs_open_file(mla_string("/opentest/write.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE, rwStream),
                "Should open file in read/write mode");
    // Close the stream
    rwStream = mla_file_system_stream_empty();
    (void)rwStream;

    // Cleanup
    mla_fs_delete_file(mla_string("/opentest/write.txt"));
    mla_fs_delete_directory(mla_string("/opentest/"));
}

void FileSystemReadWriteDataTest() {
    // Create test directory
    assert_true(mla_fs_create_directory(mla_string("/rwtest/")),
                "Should create test directory");

    // Test data to write
    const char* testData = "Hello, this is test data for file I/O!";
    mla_size_t dataLength = mla_strlen(testData);

    // Open file for writing
    mla_file_system_stream_t writeStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/rwtest/data.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, writeStream),
                "Should open file in write mode");

    // Write test data
    if (writeStream.write != nullptr) {
        mla_size_t bytesWritten = writeStream.write(writeStream, 0, dataLength,
                                               (const mla_byte_t*)testData);
        assert_equal(bytesWritten, dataLength,
                    "Should write all test data bytes");
    } else {
        assert_true(false, "Write function should not be null");
    }


    // Close write stream
    writeStream = mla_file_system_stream_empty();

    // Verify file exists
    assert_true(mla_fs_file_exists(mla_string("/rwtest/data.txt")),
                "File should exist after writing");

    // Open file for reading
    mla_file_system_stream_t readStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(mla_string("/rwtest/data.txt"),
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, readStream),
                "Should open file in read mode");

    // Get file length to verify
    mla_size_t fileLength = 0;
    if (readStream.length != nullptr) {
        fileLength = readStream.length(readStream);
        assert_equal(fileLength, dataLength,
                    "File length should match written data length");
    } else {
        assert_true(false, "Length function should not be null");
    }


    // Read data back
    mla_byte_t* readBuffer = (mla_byte_t*)mla_platform_malloc(fileLength + 1);

    if (readBuffer != nullptr && readStream.read != nullptr) {
        mla_size_t bytesRead = readStream.read(readStream, 0, fileLength, readBuffer);
        assert_equal(bytesRead, dataLength,
                    "Should read all data bytes");
        // Null-terminate for string comparison
        readBuffer[bytesRead] = 0;

        // Compare data
        assert_equal(mla_memcmp(readBuffer, testData, dataLength), 0,
                    "Read data should match written data");
    } else {
        assert_true(false, "Should allocate read buffer");
    }

    // Close read stream
    readStream = mla_file_system_stream_empty();

    // Clean up
    mla_platform_free(readBuffer);
    mla_fs_delete_file(mla_string("/rwtest/data.txt"));
    mla_fs_delete_directory(mla_string("/rwtest/"));
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

    test = mla_test("ReadWriteData", test_category, FileSystemReadWriteDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#if defined(_WIN32)

static const mla_string_t g_file_system_benchmark_directory = mla_string_const("/fsbench/");
static const mla_string_t g_file_system_benchmark_file_path = mla_string_const("/fsbench/read-throughput.bin");
static const mla_size_t g_file_system_benchmark_max_file_size = 8u * 1024u * 1024u;

static mla_file_system_stream_t g_file_system_benchmark_stream = mla_file_system_stream_empty();
static mla_byte_t* g_file_system_benchmark_write_buffer = nullptr;
static mla_byte_t* g_file_system_benchmark_read_buffer = nullptr;
static mla_size_t g_file_system_benchmark_read_size = 0;

void FileSystemBenchmarkSetup() {
    if (g_file_system_benchmark_write_buffer != nullptr &&
        g_file_system_benchmark_read_buffer != nullptr &&
        g_file_system_benchmark_stream.read != nullptr) {
        return;
    }

    mla_fs_delete_file(g_file_system_benchmark_file_path);
    mla_fs_delete_directory(g_file_system_benchmark_directory);
    assert_true(mla_fs_create_directory(g_file_system_benchmark_directory),
                "Benchmark setup should create directory");

    g_file_system_benchmark_write_buffer = static_cast<mla_byte_t*>(mla_platform_malloc(g_file_system_benchmark_max_file_size));
    g_file_system_benchmark_read_buffer = static_cast<mla_byte_t*>(mla_platform_malloc(g_file_system_benchmark_max_file_size));

    assert_true(g_file_system_benchmark_write_buffer != nullptr,
                "Benchmark setup should allocate write buffer");
    assert_true(g_file_system_benchmark_read_buffer != nullptr,
                "Benchmark setup should allocate read buffer");

    for (mla_size_t i = 0; i < g_file_system_benchmark_max_file_size; ++i) {
        g_file_system_benchmark_write_buffer[i] = static_cast<mla_byte_t>(i & 0xFFu);
    }

    mla_file_system_stream_t writeStream = mla_file_system_stream_empty();
    assert_true(mla_fs_open_file(g_file_system_benchmark_file_path,
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, writeStream),
                "Benchmark setup should open write stream");
    assert_true(writeStream.write != nullptr,
                "Benchmark setup write callback should exist");
    assert_equal(writeStream.write(writeStream, 0, g_file_system_benchmark_max_file_size, g_file_system_benchmark_write_buffer),
                g_file_system_benchmark_max_file_size,
                "Benchmark setup should write the benchmark file");
    writeStream = mla_file_system_stream_empty();

    assert_true(mla_fs_open_file(g_file_system_benchmark_file_path,
                MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, g_file_system_benchmark_stream),
                "Benchmark setup should open read stream");
    assert_true(g_file_system_benchmark_stream.read != nullptr,
                "Benchmark setup read callback should exist");
}

void FileSystemBenchmarkTearDown() {
    g_file_system_benchmark_stream = mla_file_system_stream_empty();

    if (g_file_system_benchmark_write_buffer != nullptr) {
        mla_platform_free(g_file_system_benchmark_write_buffer);
        g_file_system_benchmark_write_buffer = nullptr;
    }

    if (g_file_system_benchmark_read_buffer != nullptr) {
        mla_platform_free(g_file_system_benchmark_read_buffer);
        g_file_system_benchmark_read_buffer = nullptr;
    }

    mla_fs_delete_file(g_file_system_benchmark_file_path);
    mla_fs_delete_directory(g_file_system_benchmark_directory);
}

void FileSystemBenchmarkReadSync() {
    assert_true(g_file_system_benchmark_stream.seek != nullptr,
                "Sync benchmark seek callback should exist");
    assert_true(g_file_system_benchmark_stream.seek(g_file_system_benchmark_stream, 0),
                "Sync benchmark should seek to file start");

    mla_size_t bytesRead = __mla_file_system_native_open_file_read_sync(
        g_file_system_benchmark_stream,
        0,
        g_file_system_benchmark_read_size,
        g_file_system_benchmark_read_buffer
    );

    assert_equal(bytesRead, g_file_system_benchmark_read_size,
                "Sync benchmark should read the requested amount");

    volatile mla_byte_t firstByte = g_file_system_benchmark_read_buffer[0];
    volatile mla_byte_t lastByte = g_file_system_benchmark_read_buffer[g_file_system_benchmark_read_size - 1];
    (void)firstByte;
    (void)lastByte;
}

void FileSystemBenchmarkReadAuto() {
    assert_true(g_file_system_benchmark_stream.seek != nullptr,
                "Auto benchmark seek callback should exist");
    assert_true(g_file_system_benchmark_stream.seek(g_file_system_benchmark_stream, 0),
                "Auto benchmark should seek to file start");

    mla_size_t bytesRead = g_file_system_benchmark_stream.read(
        g_file_system_benchmark_stream,
        0,
        g_file_system_benchmark_read_size,
        g_file_system_benchmark_read_buffer
    );

    assert_equal(bytesRead, g_file_system_benchmark_read_size,
                "Auto benchmark should read the requested amount");

    volatile mla_byte_t firstByte = g_file_system_benchmark_read_buffer[0];
    volatile mla_byte_t lastByte = g_file_system_benchmark_read_buffer[g_file_system_benchmark_read_size - 1];
    (void)firstByte;
    (void)lastByte;
}

void RegisterFileSystemReadBenchmark(mla_benchmark_executor_t &p_BenchmarkExecutor,
                                     const mla_test_char_t* name,
                                     void (*run)(void)) {
    mla_benchmark_t benchmark = mla_benchmark(name, benchmark_category, run,
                                              FileSystemBenchmarkSetup, FileSystemBenchmarkTearDown);
    mla_benchmark_set_iteration_division(benchmark, 200000);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}

void FileSystemBenchmarkRead256KbSync() {
    g_file_system_benchmark_read_size = 256u * 1024u;
    FileSystemBenchmarkReadSync();
}

void FileSystemBenchmarkRead256KbAuto() {
    g_file_system_benchmark_read_size = 256u * 1024u;
    FileSystemBenchmarkReadAuto();
}

void FileSystemBenchmarkRead1MbSync() {
    g_file_system_benchmark_read_size = 1024u * 1024u;
    FileSystemBenchmarkReadSync();
}

void FileSystemBenchmarkRead1MbAuto() {
    g_file_system_benchmark_read_size = 1024u * 1024u;
    FileSystemBenchmarkReadAuto();
}

void FileSystemBenchmarkRead8MbSync() {
    g_file_system_benchmark_read_size = 8u * 1024u * 1024u;
    FileSystemBenchmarkReadSync();
}

void FileSystemBenchmarkRead8MbAuto() {
    g_file_system_benchmark_read_size = 8u * 1024u * 1024u;
    FileSystemBenchmarkReadAuto();
}

#endif

void RegisterFileSystemBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
#if defined(_WIN32)
    RegisterFileSystemReadBenchmark(p_BenchmarkExecutor, "Read256KbSync", FileSystemBenchmarkRead256KbSync);
    RegisterFileSystemReadBenchmark(p_BenchmarkExecutor, "Read256KbAuto", FileSystemBenchmarkRead256KbAuto);
    RegisterFileSystemReadBenchmark(p_BenchmarkExecutor, "Read1MbSync", FileSystemBenchmarkRead1MbSync);
    RegisterFileSystemReadBenchmark(p_BenchmarkExecutor, "Read1MbAuto", FileSystemBenchmarkRead1MbAuto);
    RegisterFileSystemReadBenchmark(p_BenchmarkExecutor, "Read8MbSync", FileSystemBenchmarkRead8MbSync);
    RegisterFileSystemReadBenchmark(p_BenchmarkExecutor, "Read8MbAuto", FileSystemBenchmarkRead8MbAuto);
#else
    (void)p_BenchmarkExecutor;
#endif
}


#endif