#include <stdint.h>
#include <string.h>
#include <fatfs.h>
#include <debug_log.h>
#include <os/StringBuffer.h>
#include <Network/DebugTime.h>
#include <Network/filenameReducer.h>
#include <Network/post.h>
#include <Network/SmallPages.h>
#include <lwip/apps/fs.h>
#include <lwip/apps/fs_custom_files.h>

#define MY_DEBUG 0

#if MY_DEBUG
#define MY_PRINT(X) printf X
#else
#define MY_PRINT(X)
#endif

extern "C"
u8_t
fs_canread_custom(struct fs_file *file);

const char * const WebFolder = "WEB";

static uint32_t getFileETag(const char *name)
{
	FILINFO fi;
	if (FR_OK == f_stat(name, &fi)){
		return
				fi.fdate ^
				fi.ftime ^
				(fi.fsize<<16);
	}
	return 0;
}

static tPostRequest * getPostRequest(const char *name) {
	for (auto it = requestList.begin(); it != requestList.end(); ++it)
	{
		if ( (*it)->response == NULL )
			continue;

		if (strcmp((*it)->response->GetResponseUri(), name) == 0)
		{
			tPostRequest *request = *it;
			requestList.erase(it);
			return request;
		}
	}

	return nullptr;
}

static void initFileStruct(struct fs_file *const file, PostResponse* const response) {
	fs_pextension_t* const extra = new fs_pextension_t(
			nullptr, response, 0, CUSTOM_FILE_JSON);

	file->flags &= ~FS_FILE_FLAGS_HEADER_INCLUDED;	// no "HTTP/1.0 200 OK"
	file->index = 0;				// position in file to start read with
	file->len = INT32_MAX;			// file size - we don't know it before we start to make response
	file->pextension = extra;		// put extra data to fs_file structure
	file->data = NULL;				// data is not exists, for now
}

extern "C"
int fs_open_custom(struct fs_file *file, const char *name)
{
	PRINT_LOG(CFG_DEBUG_LOG_NET, LOG_LEVEL_LOW_PRIORITY, "open %p '%s'\r\n", file, name);

	file->pextension = nullptr;

	if (strncmp(name, jsonResponseUri, strlen(jsonResponseUri)) == 0)
	{
		tPostRequest * const request = getPostRequest(name);

		if (request == nullptr) {
			return 0;
		}

		initFileStruct(file, request->response);

		delete request;

		return 1;
	}

	if (strncmp(name, smallPageUri, strlen(smallPageUri)) == 0)
	{
		PostResponse* const response = SmallPages::Create(name);

		if (response == nullptr) {
			return 0;
		}

		initFileStruct(file, response);

		return 1;
	}


    if (strlen(name)<=4){
    	PRINT_LOG(CFG_DEBUG_LOG_NET, LOG_LEVEL_LOW_PRIORITY, "Small name '%s' - %i\r\n", name, strlen(name));
    }

    const char * cfilePath;

    if ( 0 == strncmp(name, "/configs/", 9) ){
  //  	if (LoginSession::isExpert(session_id)){
    		cfilePath = name;
    //	} else {
    //		file->fileObject = NULL;
    //		return 0;
    //	}
    } else {
		const char * normalizedName = convertFilePath(name);
		size_t filePathSize = strlen(WebFolder) + strlen(normalizedName);
		char * filePath = stringBufferAlloc(filePathSize + 1);

		sprintf(filePath, "%s%s", WebFolder, normalizedName);
		cfilePath = filePath;
    }

	FIL * fil = new FIL;       /* File object */
	FRESULT fr;    				/* FatFs return code */

	MY_PRINT(("opening file path %s\n\r", filePath));
	/* Open file */
	fr = f_open(fil, (const TCHAR*)cfilePath, FA_READ);

	if (fr != FR_OK)
	{
		MY_PRINT(("open file %s %d error\n\r", name, fr));
		delete fil;
		return 0;
	}


	fs_pextension_t* const extra = new fs_pextension_t(
			fil, nullptr, getFileETag(cfilePath), CUSTOM_FILE_SD);

	file->flags &= ~FS_FILE_FLAGS_HEADER_INCLUDED;	// no "HTTP/1.0 200 OK" in file data
	file->index = 0;				// position in file to start read with
	file->len = f_size(fil);		// file size
	file->pextension = extra;		// put extra data to fs_file structure
	file->data = NULL;				// data is on sd-card

	MY_PRINT(("opened file %s\nsize %d\nptr %d\r\n",name, file->len, file));

	return 1;
}

extern "C"
void fs_close_custom(struct fs_file *file)
{
	PRINT_LOG(CFG_DEBUG_LOG_NET, LOG_LEVEL_LOW_PRIORITY, "close %p\r\n", file);
	fs_pextension_t* const extra = (fs_pextension_t*)file->pextension;

	switch (extra->type) {
	case CUSTOM_FILE_SD:
	{
		if (extra->fileObject != nullptr)
		{
			MY_PRINT(("closing file %d\r\n", file));
			f_close(extra->fileObject);
			delete extra->fileObject;
			extra->fileObject = nullptr;
		}
		break;
	}
	case CUSTOM_FILE_JSON:
	{
		delete extra->jsonResponse;
		extra->jsonResponse = nullptr;
		break;
	}
	}

	delete extra;
}

extern "C"
int fs_read_async_custom(struct fs_file *file, char *buffer, int count, fs_wait_cb callback_fn, void *callback_arg) {
	fs_pextension_t* const extra = (fs_pextension_t*)file->pextension;

	switch (extra->type) {
	case CUSTOM_FILE_SD:
	{
		DebugTime dtm("readsd", 50);
		int read;

		if (file->index < file->len)
		{
			read = file->len - file->index;
			if (read > count) {
				read = count;
			}

			MY_PRINT(("file %d reading %d bytes from pos %d\r\n", file, read, file->index));
		}
		else
		{
			return FS_READ_EOF;
		}
		UINT numberOfBytesRead = 0;
		f_read(extra->fileObject, buffer, read, &numberOfBytesRead);
		file->index += numberOfBytesRead;
		return numberOfBytesRead;
	}
	case CUSTOM_FILE_JSON:
	{
		DebugTime dtm("readjson", 80);

		if (!fs_canread_custom(file)) {
			return FS_READ_DELAYED;
		}

		int numberOfBytesRead = 0;
		(extra->jsonResponse)->GetResponse(buffer, count, &numberOfBytesRead);
		file->index += numberOfBytesRead;
		if (numberOfBytesRead <= 0)
			file->len = file->index;
		return numberOfBytesRead;
	}
	}

	MY_PRINT(("file %d reached eof\r\n", file));
	return FS_READ_EOF;
}

extern "C"
u8_t
fs_canread_custom(struct fs_file *file)
{
  /* If reading would block, return 0 and implement fs_wait_read_custom() to call the
     supplied callback if reading works. */
	const PostResponse * const jsonResponse =
			((fs_pextension*) (file->pextension))->jsonResponse;
	if (jsonResponse != nullptr) {
		return jsonResponse->isResponseReady() ? 1 : 0;
	}
  return 1;
}

extern "C"
u8_t
fs_wait_read_custom(struct fs_file *file, fs_wait_cb callback_fn, void *callback_arg)
{
  /* not implemented in this example */
  LWIP_UNUSED_ARG(file);
  LWIP_UNUSED_ARG(callback_fn);
  LWIP_UNUSED_ARG(callback_arg);
  /* Return
     - 1 if ready to read (at least one byte)
     - 0 if reading should be delayed (call 'tcpip_callback(callback_fn, callback_arg)' when ready) */
  return 0;
}

