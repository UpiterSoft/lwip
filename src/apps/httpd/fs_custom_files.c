#include <stdint.h>
#include <fatfs.h>
#include <lwip/apps/fs.h>


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

int fs_open_custom(struct fs_file *file, const char *name, uint32_t session_id)
{
	PRINT_LOG(CFG_DEBUG_LOG_NET, LOG_LEVEL_LOW_PRIORITY, "open %p '%s'\r\n", file, name);
	const size_t JSON_RESPONSE_URI_LEN = strlen(jsonResponseUri);

	if (strncmp(name, jsonResponseUri, JSON_RESPONSE_URI_LEN) == 0)
	{
		file->is_custom_file = CUSTOM_FILE_JSON;

		tPostRequest * request = NULL;

		for (auto it = requestList.begin(); it != requestList.end(); ++it)
		{
			if ( (*it)->response == NULL )
				continue;

			char responseUriAddress[JSON_RESPONSE_URI_LEN + 9];
			sprintf(responseUriAddress, "%s", (*it)->response->GetResponseUri());

			if (strcmp(responseUriAddress, name) == 0)
			{
				request = *it;
				requestList.erase(it);
				break;
			}
		}

		if (request == NULL)
		{
			return 0;
		}

		file->http_header_included = 0;	// no "HTTP/1.0 200 OK"
		file->index = 0;				// position in file to start read with
		file->len = INT32_MAX;				// file size - we don't know it before we start to make response
		file->pextension = NULL;		// donno
		file->data = NULL;				// data is not exists, now
		file->jsonResponse = request->response;
		file->ETag = 0;

		delete request;

		return 1;
	}

	if (strncmp(name, smallPageUri, strlen(smallPageUri)) == 0)
	{
		file->is_custom_file = CUSTOM_FILE_JSON;

		file->http_header_included = 0;	// no "HTTP/1.0 200 OK"
		file->index = 0;				// position in file to start read with
		file->len = INT32_MAX;				// file size - we don't know it before we start to make response
		file->pextension = NULL;		// donno
		file->data = NULL;				// data is not exists, now
		file->jsonResponse = SmallPages::Create(name);
		file->ETag = 0;

		if (file->jsonResponse == NULL)
			return 0;

		return 1;
	}

    file->is_custom_file = CUSTOM_FILE_SD;

    if (strlen(name)<=4){
    	PRINT_LOG(CFG_DEBUG_LOG_NET, LOG_LEVEL_LOW_PRIORITY, "Small name '%s' - %i\r\n", name, strlen(name));
    }

    const char * cfilePath;

    if ( 0 == strncmp(name, "/configs/", 9) ){
    	if (LoginSession::isExpert(session_id)){
    		cfilePath = name;
    	} else {
    		file->fileObject = NULL;
    		return 0;
    	}
    } else {
		const char * normalizedName = convertFilePath(name);
		size_t filePathSize = strlen(WebFolder) + strlen(normalizedName);
		char * filePath = stringBufferAlloc(filePathSize + 1);
		filePath[filePathSize] = '\0';

		sprintf(filePath, "%s%s", WebFolder, normalizedName);
		cfilePath = filePath;
    }

	FIL * fil = new FIL;       /* File object */
	FRESULT fr;    				/* FatFs return code */

	MY_PRINT(("opening file path %s\n\r", filePath));
	/* Open file */
	fr = f_open(fil, (const TCHAR*)cfilePath, FA_READ);

	if (fr)
	{
		MY_PRINT(("open file %s %d error\n\r", name, fr));
		delete fil;
		file->fileObject = NULL;
		return 0;
	}


	file->http_header_included = 0;	// no "HTTP/1.0 200 OK" in file data
	file->index = 0;				// position in file to start read with
	file->len = f_size(fil);		// file size
	file->pextension = NULL;		// donno
	file->data = NULL;				// data is on sd-card
	file->fileObject = fil;			// File object
	file->ETag = getFileETag(cfilePath);

	MY_PRINT(("opened file %s\nsize %d\nptr %d\r\n",name, file->len, file));

	return 1;
}

void fs_close_custom(struct fs_file *file)
{
	PRINT_LOG(CFG_DEBUG_LOG_NET, LOG_LEVEL_LOW_PRIORITY, "close %p\r\n", file);
	switch (file->is_custom_file) {
	case CUSTOM_FILE_SD:
	{
		if (file->fileObject != NULL)
		{
			MY_PRINT(("closing file %d\r\n", file));
			f_close(file->fileObject);
			delete file->fileObject;
		}
		return;
	}
	case CUSTOM_FILE_JSON:
	{
		delete file->jsonResponse;
		return;
	}
	}

}

int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
	switch (file->is_custom_file) {
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
			return EOF;
		}
		UINT numberOfBytesRead = 0;
		f_read(file->fileObject, buffer, read, &numberOfBytesRead);
		file->index += numberOfBytesRead;
		return numberOfBytesRead;
	}
	case CUSTOM_FILE_JSON:
	{
		DebugTime dtm("readjson", 80);

		int numberOfBytesRead = 0;
		(file->jsonResponse)->GetResponse(buffer, count, &numberOfBytesRead);
		file->index += numberOfBytesRead;
		if (numberOfBytesRead <= 0)
			file->len = file->index;
		return numberOfBytesRead;
	}
	}

	MY_PRINT(("file %d reached eof\r\n", file));
	return EOF;
}
