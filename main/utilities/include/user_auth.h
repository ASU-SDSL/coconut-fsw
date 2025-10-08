#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "filesystem.h"

#define TOKEN_LENGTH 8U
#define USER_MAX_TOTAL_DATA_SIZE 25 * 1024 /* 250Kb divided between 10 users (half of 0.5Mb MRAM)*/

#define MAX_USERNAME_LEN 10 // 10 character usernames 
#define USERS_DIR "/users"
#define USER_PATH_SIZE (strlen(USERS_DIR) + MAX_USERNAME_LEN + 1) /* +1 for extra "/" */

#define DATA_FILE "data"
#define DATA_PATH_SIZE (USER_PATH_SIZE + strlen(DATA_FILE) + 1) /* +1 for extra "/" */

#define TOKEN_FILE "token"
#define TOKEN_PATH_SIZE (USER_PATH_SIZE + strlen(TOKEN_FILE) + 1) /* +1 for extra "/" */

bool is_admin(const uint8_t* user_token);

int32_t add_user(const char *user_name, const uint8_t* user_token);
int32_t get_user(const uint8_t *user_token, char *user_name_out, size_t user_name_bufsize);
int32_t get_user_dir_path(const char* user_name, char *user_dir_out, size_t user_dir_out_size);
int32_t get_user_token_path(const char* user_name, char *token_path_out, size_t token_path_out_size);
int32_t get_user_data_path(const char* user_name, char *data_path_out, size_t data_path_out_size);
int32_t delete_user(const char *user_name);
int32_t upload_user_data(const uint8_t *user_token, uint8_t *buf, uint16_t buf_size);

void user_auth_test();