#ifndef __DSQLITE_
#define __DSQLITE_

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <string.h>

/* C++ */
#include <string>
#include <iostream>
using namespace std;

struct face_data {
    unsigned char*          feature;    // 人脸特征信息
    signed int          featureSize;    // 人脸特征信息长度    
};

struct sqldata {
	int id;
	string name;
	string imgpath;
	struct face_data data;
	//LPASF_FaceFeature feature;
};

class dsqlite {
public:
        dsqlite(string, int (*dcompare_callback)(int , const unsigned char *, const unsigned char *, const void *, int, void *),void * data); 
        ~dsqlite();
	int add(sqldata*);
	int del(int);
	int compare(void);
	//int find(int);
private:
	int (*compare_callback)(int , const unsigned char *, const unsigned char *, const void *,int,void *);
	sqlite3 *db;
	void * fun_data;
	string trans(sqldata *);
};


#endif

