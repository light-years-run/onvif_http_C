#include "onvif_http.h"
const char* userAndPass = "admin:admin";
const char* m_url = "http://192.168.31.101";
CURL* pCurl;
char* reqBody = NULL;
char* resBody = NULL;
char pullUrl[100] = { '\0' };
char token[50] = { '\0' };
char snapShotUri[100] = { '\0' };
struct DataStruct {
	char* buffer;
	size_t size;
};
void onvif_http_init()
{
	pCurl = curl_easy_init();
}
size_t write_function(void* contents, size_t size, size_t number, void* stream)
{
	int len = size * number;
	struct DataStruct* pdata = (struct DataStruct*)stream;

	char* pt = realloc(pdata->buffer, pdata->size + len + 1);
	if (!pt)
	{
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	pdata->buffer = pt;
	memcpy(&(pdata->buffer[pdata->size]), contents, len);
	pdata->size += len;
	pdata->buffer[pdata->size] = '\0';

	//memset(&pdata->buffer[pdata->size], "\0", len + 1);	
	//strcpy(pdata->buffer, (char*)contents);

	return len;
}
size_t write_jpg(void* ptr, size_t size, size_t nmemb, void* stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
	return written;
}
char* url_request(const char* url, const char* body, const char* method)
{
	CURLcode ret;
	struct DataStruct data = { '\0' };
	data.buffer = (char*)malloc(10);
	data.size = 0;
	if (!data.buffer)
		return NULL;
	memset(data.buffer, '\0', 10);

	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(pCurl, CURLOPT_URL, url);
	curl_easy_setopt(pCurl, CURLOPT_USERPWD, userAndPass);
	curl_easy_setopt(pCurl, CURLOPT_HTTPAUTH, (CURLAUTH_DIGEST | CURLAUTH_BASIC));

	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_function);

	if (strcmp(body, "") != 0)
		curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, body);

	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &data);
	//连接是长连接，设置读取超时5分钟，5分钟后重连连接
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(pCurl, CURLOPT_MAXREDIRS, 1);
	//连接超时
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5);

	ret = curl_easy_perform(pCurl);


	if (ret != CURLE_OK)
	{
		printf("ret = %d\n", (int)ret);
	}

	return data.buffer;
}
//读文件获取请求报文,用后注意free
char* get_reqbody(const char* fileName)
{
	FILE* fp;
	int readLen, fileLen = 0;
	char* buf;
	if (NULL == (fp = fopen(fileName, "rb")))
	{
		perror("fopen");
		return NULL;
	}
	if (!(buf = malloc(1024)))
		return NULL;
	memset(buf, '\0', 1024);

	while (!feof(fp))
	{
		readLen = fread((buf + fileLen), 1, 1024, fp);
		fileLen += readLen;
		if (readLen == 1024)
		{
			buf = realloc(buf, fileLen + 1024);
			memset(buf + fileLen, '\0', 1024);
		}
	}
	fclose(fp);

	return buf;
}

void my_free(char* pt)
{
	if (pt)
		free(pt);
}

//for snapshot
char* get_profiletoken()
{
	char reqUrl[100] = { '\0' };
	char* value = NULL;
	ezxml_t child = NULL;
	strcat(reqUrl, m_url);
	strcat(reqUrl, "/onvif/device_service");
	reqBody = get_reqbody("./GetProfileTocken.xml");
	resBody = url_request(reqUrl, reqBody, "POST");
	if (!resBody)
		return;

	ezxml_t root = ezxml_parse_str(resBody, strlen(resBody));

	child = ezxml_get(root, "SOAP-ENV:Body", 0,
		"trt20:GetProfilesResponse", 0, "trt20:Profiles", -1);
	if (child)
	{
		value = ezxml_attr(child, "token");
		strcpy(token, value);
	}

	my_free(reqBody);
	my_free(resBody);
	ezxml_free(root);
	return token;
}
char* get_snapshot_uri(char* token)
{
	char reqUrl[100] = { '\0' };
	char* value = NULL;
	ezxml_t child = NULL;
	strcat(reqUrl, m_url);
	strcat(reqUrl, "/onvif/device_service");
	reqBody = get_reqbody("./GetSnapShotUri.xml");
	if (!reqBody)
		return;
	//默认token名字为protoken_ch0001,如果不是,则替换
	if (strstr(reqBody, token) == NULL)
	{
		ezxml_t root = ezxml_parse_str(reqBody, strlen(reqBody));

		child = ezxml_get(root, "soap:Body", 0,
			"trt:GetSnapshotUri", 0, "trt:ProfileToken", -1);
		if (child)
		{
			ezxml_set_txt(child, token);
			my_free(reqBody);
			reqBody = ezxml_toxml(root);
		}
		ezxml_free(root);
	}

	resBody = url_request(reqUrl, reqBody, "POST");

	ezxml_t root = ezxml_parse_str(resBody, strlen(resBody));

	child = ezxml_get(root, "SOAP-ENV:Body", 0, "trt:GetSnapshotUriResponse", 0,
		"trt:MediaUri", 0, "tt:Uri", -1);
	if (child)
	{
		value = child->txt;
		strcpy(snapShotUri, value);
	}

	my_free(reqBody);
	my_free(resBody);
	ezxml_free(root);
	return snapShotUri;
}
void get_snapshot_img(char* uri)
{
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(pCurl, CURLOPT_URL, uri);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_jpg);
	FILE* file = fopen("snapshot.jpg", "wb");
	if (file)
	{
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, file);
		curl_easy_perform(pCurl);
		fclose(file);
	}
}
void onvif_http_clean()
{
	curl_easy_cleanup(pCurl);
}