
#ifndef _ROMI_FS_H_
#define _ROMI_FS_H_

#ifdef __cplusplus
extern "C" {
#endif

int is_absolute(const char *path);
int make_absolute_path(const char *path, char *buffer, int len);

#ifdef __cplusplus
}
#endif

#endif  /* _ROMI_FS_H_ */
