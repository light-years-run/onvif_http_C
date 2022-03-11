#include "onvif_http.h"

int main()
{
	onvif_http_init();
	//»ñÈ¡¿ìÕÕtoken
	char* ptoken = get_profiletoken();
	if (!ptoken)
	{
		printf("can not get token\n");
		return -1;
	}
	int j = 0;
	while (1)
	{
		char* url = get_snapshot_uri(ptoken);
		if (!url)
		{
			printf("can not get snapShotUri\n");
			return -1;
		}
		get_snapshot_img(url);
		j++;
		sleep(1);
		if (j > 10)
			break;
	}

	onvif_http_clean();
	return 0;
}