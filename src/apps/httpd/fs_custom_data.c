/*
 * fs_custom_data.c
 *
 *  Created on: May 11, 2017
 *      Author: SmartWebProgrammer
 */

#define file_NULL (struct fsdata_file *) NULL

const struct fsdata_file file__img_sics_gif[] = { {
file_NULL,
NULL,
0,
0,
FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT,
}};

const struct fsdata_file file__404_html[] = { {
file_NULL,
NULL,
0,
0,
FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT,
}};

const struct fsdata_file file__index_html[] = { {
file_NULL,
NULL,
0,
0,
FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT,
}};

#define FS_ROOT file__index_html


