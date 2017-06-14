/*
 * fs_custom_files.h
 *
 *  Created on: Apr 26, 2017
 *      Author: SmartWebProgrammer
 */

#ifndef FS_CUSTOM_FILES_H_
#define FS_CUSTOM_FILES_H_


#include <stdint.h>
#include <fatfs.h>
#include <Network/PostResponse.h>

typedef enum {
	CUSTOM_FILE_OFF = 0,
	CUSTOM_FILE_SD,
	CUSTOM_FILE_JSON,
	CUSTOM_FILE_SMALL_PAGES,
	CUSTOM_FILE_CONFIGS,

	CUSTOM_FILE_MAX_NUM,
} eCustomFileType;

typedef struct fs_pextension {
	enum {
		ETAG_HEADER_BUFFER_SIZE = 25,
	};
	  FIL * fileObject;
	  PostResponse *jsonResponse;
	  char ETagHeaderBuffer[ETAG_HEADER_BUFFER_SIZE];
	  uint32_t ETag;
	  eCustomFileType type;
	  fs_pextension (
			  FIL* const fObj,
			  PostResponse* const response,
			  uint32_t const tag,
			  eCustomFileType const t);
} fs_pextension_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
int fs_read_custom(struct fs_file *file, char *buffer, int count);

char * getETagHeader(void * const pextension);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* FS_CUSTOM_FILES_H_ */
