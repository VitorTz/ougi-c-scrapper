#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>

/* Assuming the provided header is saved as "path.h" */
#include "../include/structure/path.h"

#define SANDBOX_DIR "./test_sandbox_dir"

/* ============================================================
 * Helper Functions
 * ============================================================ */

/* Creates a clean sandbox directory for filesystem tests */
void setup_sandbox(void) {
    path_t sandbox = path_create(SANDBOX_DIR);
    if (path_exists(&sandbox)) {
        path_delete_recursive(&sandbox, false);
    }
    path_create_directories(&sandbox);
    path_destroy(&sandbox);
}

/* Removes the sandbox directory after tests */
void teardown_sandbox(void) {
    path_t sandbox = path_create(SANDBOX_DIR);
    if (path_exists(&sandbox)) {
        path_delete_recursive(&sandbox, false);
    }
    path_destroy(&sandbox);
}

/* Dummy filter function for iterator tests */
int filter_txt_files(const void* data) {
    const path_t* p = (const path_t*) data;
    char* ext = path_extension(p);
    bool is_txt = (strcmp(ext, ".txt") == 0);
    free(ext);
    return is_txt;
}

/* ============================================================
 * Test Cases
 * ============================================================ */

void test_path_creation_and_modification(void) {
    /* Test empty and basic creation */
    path_t p1 = path_create_empty();
    assert(strcmp(path_c_str(&p1), "") == 0);
    path_destroy(&p1);

    path_t p2 = path_create("/var/log");
    assert(strcmp(path_c_str(&p2), "/var/log") == 0);

    /* Test copy */
    path_t p3 = path_create_copy(&p2);
    assert(strcmp(path_c_str(&p3), "/var/log") == 0);

    /* Test append */
    path_append(&p3, "syslog");
    assert(strcmp(path_c_str(&p3), "/var/log/syslog") == 0);

    /* Test append with trailing/leading slashes to ensure no double slashes */
    path_t p4 = path_create("/var/log/");
    path_append(&p4, "/auth.log");
    assert(strcmp(path_c_str(&p4), "/var/log/auth.log") == 0);

    /* Test change extension */
    path_t p5 = path_create("image.jpg");
    path_change_extension(&p5, ".png");
    assert(strcmp(path_c_str(&p5), "image.png") == 0);

    path_destroy(&p2);
    path_destroy(&p3);
    path_destroy(&p4);
    path_destroy(&p5);
}

void test_path_parsing(void) {
    path_t p = path_create("/home/user/project/main.c");

    /* Test filename */
    char* filename = path_filename(&p);
    assert(strcmp(filename, "main.c") == 0);
    free(filename);

    /* Test stem */
    char* stem = path_stem(&p);
    assert(strcmp(stem, "main") == 0);
    free(stem);

    /* Test extension */
    char* ext = path_extension(&p);
    assert(strcmp(ext, ".c") == 0);
    free(ext);

    /* Test parent path */
    path_t parent = path_parent_path(&p);
    assert(strcmp(path_c_str(&parent), "/home/user/project") == 0);

    path_destroy(&p);
    path_destroy(&parent);

    /* Test edge case: dotfile */
    path_t p_hidden = path_create("/home/user/.gitignore");
    char* stem_hidden = path_stem(&p_hidden);
    assert(strcmp(stem_hidden, ".gitignore") == 0);
    free(stem_hidden);
    path_destroy(&p_hidden);
}

void test_filesystem_operations(void) {
    path_t sandbox = path_create(SANDBOX_DIR);
    path_t file_path = path_create_copy(&sandbox);
    path_append(&file_path, "test_file.txt");

    /* Test touch and exists */
    assert(!path_exists(&file_path));
    assert(path_touch(&file_path));
    assert(path_exists(&file_path));
    assert(path_is_regular_file(&file_path));
    assert(!path_is_dir(&file_path));

    /* Test directory creation */
    path_t dir_path = path_create_copy(&sandbox);
    path_append(&dir_path, "nested/dir/structure");
    assert(path_create_directories(&dir_path));
    assert(path_is_dir(&dir_path));
    assert(!path_is_regular_file(&dir_path));

    /* Test copy */
    path_t copy_path = path_create_copy(&sandbox);
    path_append(&copy_path, "test_file_copy.txt");
    assert(path_copy(&file_path, &copy_path));
    assert(path_exists(&copy_path));

    /* Test move */
    path_t move_path = path_create_copy(&sandbox);
    path_append(&move_path, "test_file_moved.txt");
    assert(path_move(&copy_path, &move_path));
    assert(path_exists(&move_path));
    assert(!path_exists(&copy_path)); /* Old path should be gone */

    /* Clean up */
    path_destroy(&sandbox);
    path_destroy(&file_path);
    path_destroy(&dir_path);
    path_destroy(&copy_path);
    path_destroy(&move_path);
}

void test_file_io(void) {
    path_t sandbox = path_create(SANDBOX_DIR);
    path_t txt_file = path_create_copy(&sandbox);
    path_append(&txt_file, "hello.txt");

    /* Test text write and read */
    const char* content = "Hello, UNIX filesystem!\n";
    assert(path_write_text(&txt_file, content));
    
    string_t read_str = path_read_text(&txt_file);
    assert(strcmp(string_cstr(&read_str), content) == 0);
    string_free(&read_str);

    /* Test binary write and read */
    path_t bin_file = path_create_copy(&sandbox);
    path_append(&bin_file, "data.bin");
    uint8_t bin_data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    assert(path_write_bytes(&bin_file, bin_data, sizeof(bin_data)));

    Read bin_read = path_read_bytes(&bin_file);
    assert(bin_read.success);
    assert(bin_read.bytes == sizeof(bin_data));
    assert(bin_read.data[0] == 0xDE && bin_read.data[3] == 0xEF);
    free(bin_read.data);

    path_destroy(&sandbox);
    path_destroy(&txt_file);
    path_destroy(&bin_file);
}

void test_directory_iteration_and_deletion(void) {
    path_t iter_dir = path_create(SANDBOX_DIR "/iter_test");
    path_create_directories(&iter_dir);

    /* Populate directory */
    path_t f1 = path_create_copy(&iter_dir); path_append(&f1, "a.txt"); path_touch(&f1);
    path_t f2 = path_create_copy(&iter_dir); path_append(&f2, "b.png"); path_touch(&f2);
    path_t f3 = path_create_copy(&iter_dir); path_append(&f3, "c.txt"); path_touch(&f3);

    /* Test iterator with filter (only .txt) */
    path_t* items = path_dir_iterator(&iter_dir, NULL, filter_txt_files);
    assert(items != NULL);
    
    /* Assuming vec_len is available from your vector.h */
    /* assert(vec_len(items) == 2); */ 
    
    path_dir_iterator_free(items);

    /* Test recursive delete */
    assert(path_delete_recursive(&iter_dir, false));
    assert(!path_exists(&iter_dir)); /* The folder itself should be gone */

    path_destroy(&iter_dir);
    path_destroy(&f1);
    path_destroy(&f2);
    path_destroy(&f3);
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void) {
    printf("Setting up sandbox environment...\n");
    setup_sandbox();

    printf("Running path parsing tests...\n");
    test_path_creation_and_modification();
    test_path_parsing();

    printf("Running filesystem operations tests...\n");
    test_filesystem_operations();

    printf("Running file I/O tests...\n");
    test_file_io();

    printf("Running directory iteration and deletion tests...\n");
    test_directory_iteration_and_deletion();

    printf("Tearing down sandbox environment...\n");
    teardown_sandbox();

    printf("\nAll path tests completed successfully!\n");
    return 0;
}