/*
 * fs_custom_data.c
 *
 *  Created on: May 11, 2017
 *      Author: SmartWebProgrammer
 */

#define file_NULL (struct fsdata_file *) NULL

static const unsigned char data__index_html[] = {0x00};

const struct fsdata_file file__index_html[] = { {
file_NULL,
data__index_html,
NULL,
0,
0,
}};

#define FS_ROOT file__index_html


