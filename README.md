# onvif_http_C
天地伟业 tiandy onvif 接口 http snapshot token

#how to use

1、change onvif_http.c file userAndPass and m_url
const char* userAndPass = "admin:admin"; 
const char* m_url = "http://192.168.1.101"; 
to your camera username password and ip

2、Linux should install curl，and curl with ssl on


#make

1、unzip

2、make

3、./onvif_http.out 


#Participation and contribution

Thanks to ezxml author, a very easy-to-use XML parsing library 

https://github.com/lxfontes/ezxml



#characteristic

1.lightweight onvif protocol

2.no gsoap

3.get camera token and snapshot

