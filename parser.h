#pragma once


char* get_client_request(char* buffer);

char* extract_filename(const char *input);

char* extract_extension(const char *input);

char* extract_params(const char *input);
