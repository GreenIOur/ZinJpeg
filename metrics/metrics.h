#ifndef __metrics_h__
#define __metrics_h__
/*
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "opencv2/imgproc/imgproc.hpp"
*/
#include "compressor.h"


class metrics{
	
	private :
	
	unsigned char *in_buffer;
	unsigned char *out_buffer;
	unsigned long compressed_size;
	unsigned short width;
	unsigned short height;
	std::string file_name;
	compressor *comp;
	int Nlosses;

	public :
    
	metrics(compressor *c);
	~metrics();
	void process(std::string file_name);
	void process(std::string file_name, float PLR, int BL);
	int getCompression_ratio (void);
	float getMSE (void);
	float getPSNR (void);
	double getMSSIM(void);
	char getBuffstate(void);
	unsigned char* getOut(void);

};

#endif



