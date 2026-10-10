#ifndef VAL_H
#define VAL_H

int validate_port(int port);

int validate_uid(const char *uid);

int validate_password(const char *password);

int validate_filename(const char *filename);
int validate_label(const char *label);
long get_file_size(const char *filename);

#endif