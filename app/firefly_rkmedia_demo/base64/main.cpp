#include <iostream>
#include "base64.h"

using namespace std;
int main()
{
	int nFileLen = 0;
	FILE *fp = fopen("test.jpg", "rb");
	if (fp == NULL)
	{
		cout << "can't open file" << endl;
		return 0;
	}

	fseek(fp,0,SEEK_END); //定位到文件末 
	if ((nFileLen = ftell(fp))<1)//文件长度
	{
		fclose(fp);
		return 0;
	}
	fseek(fp,0,SEEK_SET); //定位到文件末 

	char * data = (char *)malloc(sizeof(char)*(nFileLen+1));
	if (NULL == data)
	{
		fclose(fp);
		return 0;
	}
	fread(data, nFileLen, 1, fp);

	string normal,encoded;
	int i;
	Base64 *base = new Base64();
	encoded = base->Encode((const unsigned char *)data,nFileLen);
	cout << "base64 encode : " << encoded << endl;
	nFileLen = encoded.length();
	const char * str2 = encoded.c_str();
	normal = base->Decode(str2,nFileLen);
	//cout << "base64 decode : " << normal <<endl;
	FILE *fpp = fopen("save.jpg", "wb");
	fwrite(normal.c_str(),1,nFileLen,fpp);

	
	
	fclose(fpp);
	delete base;
	free(data);
	fclose(fp);
	return 0;
}
