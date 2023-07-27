#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include "tcp_comm.h"
#include <sys/time.h>

#define MAX_DECT_COUNT 		    30
#define MAX_LABEL_LEN		    30

struct label_and_location {
        char c_label[MAX_DECT_COUNT][MAX_LABEL_LEN];
        float x1[MAX_DECT_COUNT];
        float y1[MAX_DECT_COUNT];
        float x2[MAX_DECT_COUNT];
        float y2[MAX_DECT_COUNT];
	float prop[MAX_DECT_COUNT];
        int topclass[MAX_DECT_COUNT];
        int val;
	int camera_input_width;
	int camera_input_height;
};

class rknn_sock {
private:
	int output_state;
	//class tcp_comm tcp_client;
	//std::vector <rknn_tensor_attr >attrs;
	//std::vector <rknn_output >float_outs;
	//rknn_input_output_num in_out_num;

public:
	class tcp_comm tcp_client;
	rknn_sock();
	~rknn_sock();
};

//extern class tcp_comm tcp_client;
#endif
