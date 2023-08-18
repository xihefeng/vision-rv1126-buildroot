#include <stdio.h>  
#include <stdlib.h>   
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>  
#include <cstring>  
#include <curl/curl.h>
#include <sys/stat.h>
#include <getopt.h>
#include "rkmedia_api.h"
#include "sample_common_firefly_rkmedia.h"
#include <sys/mman.h>


unsigned long get_time(void)
{
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return (ts.tv_sec * 1000 + ts.tv_usec / 1000);
}
  
static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}


#if 1
char * httpurl = NULL;
static void *httpSend(void *data) {
	{
		int fd = shm_open("posixsm", O_RDWR, 0666);
		ftruncate(fd, sizeof(struct share_data));
		struct share_data *posixsm_share = (struct share_data *)mmap(NULL, sizeof(struct share_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		CURL *curl;
		CURLcode res;

		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl = curl_easy_init();
		if(curl) {
			/* First set the URL that is about to receive our POST. This URL can
			   just as well be a https:// URL if that is what should receive the
			   data. */
			while(!quit){
				if (posixsm_share->http_data_update == 1) {
					curl_easy_setopt(curl, CURLOPT_URL, httpurl);
					/* Now specify the POST data */
					//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "num=12345");
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, posixsm_share->http_data);

					/* Perform the request, res will get the return code */
					res = curl_easy_perform(curl);
					/* Check for errors */
					if(res != CURLE_OK)
						fprintf(stderr, "curl_easy_perform() failed: %s\n",
								curl_easy_strerror(res));
					posixsm_share->http_data_update = 0;
				} else {
					usleep(50);
				}
				//std::cout<<httpstr<<std::endl;
			}
			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		munmap(posixsm_share, sizeof(struct share_data)); //取消内存映射
		curl_global_cleanup();
		return 0;
	}
}
#endif

static RK_CHAR optstr[] = "c";
static const struct option long_options[] = {
    {"aiq", optional_argument, NULL, 'a'},
    {"camid", required_argument, NULL, 'I'},
    {"multictx", required_argument, NULL, 'M'},
    {"help", optional_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

int main(int argc, char **argv)  
{  
	int c;
	struct Session *session_cfg;
	session_cfg = (Session *)malloc(sizeof(Session));
	char *cfg_file_path = NULL;
	while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
		const char *tmp_optarg = optarg;
		switch (c) {
			case 'c':
				if (!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
					tmp_optarg = argv[optind++];
				}
				if (tmp_optarg) {
					cfg_file_path = (char *)tmp_optarg;
				} else {
					cfg_file_path = (char *)"/usr/share/xhlpr_app/xhlpr_app.cfg";
				}
				break;
			default:
				return 0;
		}
	}
	load_cfg(cfg_file_path, session_cfg, 'h');
	httpurl = (char *)malloc(sizeof(session_cfg->http_path));
	memcpy(httpurl, session_cfg->http_path, sizeof(session_cfg->http_path));
	free(session_cfg);

	pthread_t http_thread;
	pthread_create(&http_thread, NULL, httpSend, NULL);//发送数据到 HTTP

	while (!quit) {
		usleep(100000);
	}
	pthread_join(http_thread, NULL);
	return 0;
}  
