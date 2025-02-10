#include <string.h>
#include <stdio.h>

#include "log.h"
#include "user_auth.h"

bool is_admin(const uint8_t *user_token) {
    // Returns true if user_token is the ADMIN_TOKEN
    // ADMIN_TOKEN is defined in /CMakeLists.txt
    // Meant to be used as a lightweight permissions check without hitting the filesystem
    bool match = memcmp(user_token, ADMIN_TOKEN, TOKEN_LENGTH) == 0;
    if (!match) {
        logln_error("Failed to authenticate admin token!");
    }
    return match;
}

int32_t add_user(const char *user_name, const uint8_t* user_token) {
    // Check if username valid
    if (strlen(user_name) >= MAX_USERNAME_LEN + 1) {
        logln_error("Username \"%s\" too long for add_user!", user_name);
        return 1;
    }
    // Create users dir if it doesn't exist
    if (!dir_exists(USERS_DIR)) {
        logln_info("Users directory (/users) does not exist, creating...");
        make_dir(USERS_DIR);
    }
    // Create user dir
    char user_dir[USER_PATH_SIZE + 1];
    get_user_dir_path(user_name, user_dir, sizeof(user_dir));
    make_dir(user_dir);
    if (!dir_exists(user_dir)) {
        logln_error("Failed to create user dir for \"%s\", make sure \"%s\" is a directory!", user_name, USERS_DIR);
        return 1;
    }
    // Add token file
    char token_path[TOKEN_PATH_SIZE + 1];
    get_user_token_path(user_name, token_path, sizeof(token_path));
    write_file(token_path, user_token, TOKEN_LENGTH, false);
    if (!file_exists(token_path)) {
        logln_error("Failed to create token file for \"%s\", make sure \"%s\" is a directory!", user_name, user_dir);
        return 1;
    }
    return 0;
}

// gets a username from a token
// can be used to verify if a token is valid
// returns length of username on success, otherwise returns -1
int32_t get_user(const uint8_t *user_token, char *user_name_out, size_t user_name_bufsize) {
    // Iterate through all users
    DIR dir;
    open_dir(USERS_DIR, &dir);
    for (;;) {
        FILINFO file_info;
        if (read_dir(&dir, &file_info)) return -1; // actual read error
        if (file_info.fname[0] == 0) return -1; // read everything in dir
        // Check token
        const char *user_name_iter = file_info.fname; // name of each dir in users dir is a username
        char token_path[TOKEN_PATH_SIZE + 1];
        get_user_token_path(user_name_iter, token_path, sizeof(token_path));
        uint8_t token[TOKEN_LENGTH];
        read_file(token_path, token, sizeof(token));
        // Return string if matches
        if (memcmp(token, user_token, TOKEN_LENGTH) != 0) continue;
        strncpy(user_name_out, user_name_iter, user_name_bufsize - 1);
        // Calculate size and null-terminate
        uint32_t retsize = strnlen(user_name_iter, user_name_bufsize - 1);
        user_name_out[retsize] = '\x00';
        return retsize;
    }
    // Otherwise, error
    return -1;
}

int32_t get_user_dir_path(const char* user_name, char *user_dir_out, size_t user_dir_out_size) {
    return snprintf(user_dir_out, user_dir_out_size, "%s/%s", USERS_DIR, user_name);
}

int32_t get_user_token_path(const char* user_name, char *token_path_out, size_t token_path_out_size) {
    return snprintf(token_path_out, token_path_out_size, "%s/%s/%s", USERS_DIR, user_name, TOKEN_FILE);
}

int32_t get_user_data_path(const char* user_name, char *data_path_out, size_t data_path_out_size) {
    return snprintf(data_path_out, data_path_out_size, "%s/%s/%s", USERS_DIR, user_name, DATA_FILE);
}

int32_t delete_user(const char *user_name) {
    // Find username
    char user_dir_path[USER_PATH_SIZE + 1];
    get_user_dir_path(user_name, user_dir_path, sizeof(user_dir_path));
    if (!dir_exists(user_dir_path)) {
        logln_error("User dir \"%s\" does not exist during deletion", user_dir_path);
        return 1;
    }
    // Get other paths
    char token_path[TOKEN_PATH_SIZE + 1];
    get_user_token_path(user_name, token_path, sizeof(token_path));
    char data_path[DATA_PATH_SIZE + 1];
    get_user_data_path(user_name, data_path, sizeof(data_path));
    // Delete user files and directory
    delete_file(token_path);
    delete_file(data_path);
    delete_file(user_dir_path);
    if (dir_exists(user_dir_path)) {
        logln_error("Failed to delete user dir \"%s\"!!! Please clear out these files and try again:", user_dir_path);
        list_dir(user_dir_path);
        return 1;
    }
    return 0;
}

// stores user data using a token to authenticate
// returns size of stored data on success and -1 on failure
int32_t upload_user_data(const uint8_t *user_token, uint8_t *buf, uint16_t buf_size) {
    // Get user
    char user_name[MAX_USERNAME_LEN + 1];
    if (get_user(user_token, user_name, sizeof(user_name)) == -1) {
        logln_error("Failed to authenticate token 0x%lX%lX while uploading user data!\n", *(uint32_t*)&user_token[4], *(uint32_t*)&user_token[0]);
        return -1;
    }
    // Get user data path
    char data_path[DATA_PATH_SIZE + 1];
    get_user_data_path(user_name, data_path, sizeof(data_path));
    // Create if it doesn't exist
    if (!file_exists(data_path)) {
        touch(data_path);
        // check again just to be sure
        if (!file_exists(data_path)) {
            logln_error("Can't create data file for user \"%s\" during upload, something is seriously wrong!", user_name);
            return -1;
        }
    }
    // Check that the user has enough storage remaining
    FILINFO file_info;
    stat(data_path, &file_info);
    if (file_info.fsize >= USER_MAX_TOTAL_DATA_SIZE) {
        logln_error("User \"%s\" has no storage space left in their size 0x%lx byte allotment, upload failed!", USER_MAX_TOTAL_DATA_SIZE);
        return -1;
    }
    size_t new_data_size = buf_size + file_info.fsize;
    if (new_data_size > USER_MAX_TOTAL_DATA_SIZE) {
        buf_size = file_info.fsize - USER_MAX_TOTAL_DATA_SIZE;
        logln_warn("User \"%s\" only has 0x%hx bytes left in their allotment, truncating uploaded data!", buf_size);
    }
    // Append to file
    write_file(data_path, buf, buf_size, true);
    return 0;
}

void user_auth_test() {
    // Setup filesystem
    make_filesystem();

    // Create user
    const char user_name[] = "asu";
    const char token[] = "\x69\x68\x67\x66\x65\x64\x63\x62";
    add_user(user_name, token);

    // Show users dir
    logln_info("%s", USERS_DIR);
    list_dir(USERS_DIR);

    // Upload Data
    const char user_data[] = "hello\n";
    upload_user_data(token, user_data, sizeof(user_data));

    // Show user dir
    char user_dir_path[USER_PATH_SIZE + 1];
    get_user_dir_path(user_name, user_dir_path, sizeof(user_dir_path));
    logln_info("%s", user_dir_path);
    list_dir(user_dir_path);

    // Print data file
    const char user_data_path[DATA_PATH_SIZE + 1];
    get_user_data_path(user_name, user_data_path, sizeof(user_data_path));
    logln_info("%s", user_data_path);
    cat(user_data_path);

    // Delete user
    delete_user(user_name);
    logln_info("%s", USERS_DIR);
    list_dir(USERS_DIR);
}
