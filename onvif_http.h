#ifndef _ONVIF_HTTP_H
#define _ONVIF_HTTP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "curl/curl.h"
#include "ezxml.h"
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

	void onvif_http_init();
	//request url, return resBody
	char* url_request(const char* url, const char* body, const char* method);
	//read file get reqBody
	char* get_reqbody(const char* fileName);
	//free pt
	void my_free(char* pt);
	//for snapshot,get token
	char* get_profiletoken();
	//get snapshot uri to snapShotUri
	char* get_snapshot_uri(char* token);
	//get img and write to file
	void get_snapshot_img(char* uri);
	//free url
	void onvif_http_clean();
#ifdef __cplusplus
}
#endif

#endif // _ONVIF_HTTP_H