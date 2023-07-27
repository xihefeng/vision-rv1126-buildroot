#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "client.h"

#define MAXLINE 4096
#define SERVER_IP	"172.16.110.6"
#define DEBUG 0

using namespace cv;
using namespace std;

//#define VIDEO_NODE "video-mp4-640x360.avi"
#define VIDEO_NODE 0

int ready = 0;

Scalar colorArray[10] = {
	Scalar(139, 0, 0, 255),
	Scalar(139, 0, 139, 255),
	Scalar(0, 0, 139, 255),
	Scalar(0, 100, 0, 255),
	Scalar(139, 139, 0, 255),
	Scalar(209, 206, 0, 255),
	Scalar(0, 127, 255, 255),
	Scalar(139, 61, 72, 255),
	Scalar(0, 255, 0, 255),
	Scalar(255, 0, 0, 255),
};

rknn_sock::rknn_sock()
{
}

rknn_sock::~rknn_sock()
{
}

label_and_location nndata_draw;
void *GetCameraBuffer(void *data) {

    VideoCapture cap;

    //while(!ready);//等待摄像头准备好
    while(1) {
	printf("open camera\n");
        cap.open(VIDEO_NODE);
        if (!cap.isOpened()) {
            printf("cap camera fail\n");
	    //return 0;
        } else {
            break;
        }
        usleep(1000000);
    }
    namedWindow("Video", 1);
    while (1)
    {
	cv::Mat frame;
	cap >> frame;
	if (frame.empty())
	    return 0;
	if (nndata_draw.val > 0) {
	    for(int i = 0 ; i < nndata_draw.val ; i++ ) {
		cv::rectangle(frame, Point(nndata_draw.x1[i], nndata_draw.y1[i] ), Point(nndata_draw.x2[i], nndata_draw.y2[i]),
				colorArray[nndata_draw.topclass[i] %
				10], 2);
		cv::putText(frame, nndata_draw.c_label[i], Point(nndata_draw.x1[i] , nndata_draw.y1[i] - 4), 1, 1,
				colorArray[nndata_draw.topclass[i] %
				10]);
	    }
	}
	imshow("Video", frame);
	waitKey(30);	//延时30
    }
    cap.release();
    return 0;
}

void *TcpConnect(void *data) {
    int ret;
    rknn_sock tcp_api;
    label_and_location tcp_recv_data;
    label_and_location * nndata;
    nndata = &tcp_recv_data;

    ret = tcp_api.tcp_client.tcpc_connect(SERVER_IP, 1126); // 链接 1126
    if (ret < 0) {
	printf("connect server fail! ret=%d\n", ret);
	return 0;
    }

    ret = tcp_api.tcp_client.recv_data(&ready, sizeof(int)); // 接收准备完毕标志
    if (ret < 0) {
	printf("camera is not ready");
	return 0;
    }
    printf("ready = %d\n", ready);
    while(1) {
        memset(nndata, 0x00, sizeof(label_and_location));
        ret = tcp_api.tcp_client.recv_data(nndata, sizeof(label_and_location));//接收label和坐标数据
        if (ret != sizeof(label_and_location)) {
            printf("ret != sizeof(recv_buf)\n");
            return 0;
        }

        for (int i = 0; i < nndata->val ; i++) {
	    strcpy(nndata_draw.c_label[i], tcp_recv_data.c_label[i]);
    	    nndata_draw.x1[i] = ((int) (nndata->x1[i] * nndata->camera_input_width));
	    nndata_draw.y1[i] = ((int) (nndata->y1[i] * nndata->camera_input_height));
	    nndata_draw.x2[i] = ((int) (nndata->x2[i] * nndata->camera_input_width));
	    nndata_draw.y2[i] = ((int) (nndata->y2[i] * nndata->camera_input_height));
	    nndata_draw.prop[i] = nndata->prop[i];
	    nndata_draw.camera_input_width = nndata->camera_input_width;
	    nndata_draw.camera_input_height = nndata->camera_input_height;
	    nndata_draw.topclass[i] = nndata->topclass[i];
            if(DEBUG) {
                printf("recv %s(%d, %d, %d, %d)%.2f \t (%d, %d)%d\n",
	    	    tcp_recv_data.c_label[i],
    		    ((int) (nndata->x1[i] * nndata->camera_input_width)),
		    ((int) (nndata->y1[i] * nndata->camera_input_height)),
		    ((int) (nndata->x2[i] * nndata->camera_input_width)),
		    ((int) (nndata->y2[i] * nndata->camera_input_height)),
		    nndata->prop[i],
		    nndata->camera_input_width,
		    nndata->camera_input_height,
		    nndata->topclass[i]
                );
	    }
        }
        nndata_draw.val = nndata->val;
    }
}



int main(int argc, char**argv)
{
    pthread_t read_thread;
    pthread_t tcp_thread;

    system("ifconfig usb0 172.16.110.100");//配置电脑 usb0 网卡 ip

    pthread_create(&read_thread, NULL, GetCameraBuffer, NULL);
    pthread_create(&tcp_thread, NULL, TcpConnect, NULL);

    while(1) {
        usleep(1000);
    }
    pthread_join(read_thread, NULL);
    pthread_join(tcp_thread, NULL);
    return 0;
}

