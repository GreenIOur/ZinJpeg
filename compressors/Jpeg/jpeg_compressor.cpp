#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include "jpeg_compressor.h"
#include "jpeg_enc_interface.h"
#include <vector>


using namespace std;

//method to set the size of the input image
void jpeg_compressor::setSize( int Width, int Height){
	this->width = Width;
	this->height = Height;
	jpeg_set_size(Width,Height);
}

//method to set the quality of the compressor 0 to 100 ( 100 is the better quality possible )
void jpeg_compressor::setQuality(int quality){

	jpeg_set_quality(quality);
}

//set the markers interval length
void jpeg_compressor::setMarkers(int markers_length){

	jpeg_set_distance(markers_length);
}

//Compute from an input image to an output image ( buffers ) with jpeg encoding
int jpeg_compressor::compress(unsigned char *buffer_in,unsigned char *buffer_out){
	this->compressed_size = jpeg_encode(buffer_in,buffer_out);
	/*for(int o = 0; o < width; o++){
	std::cerr << buffer_in[o];	
	}
	std::cerr<<std::endl;*/
	return this->compressed_size;
}

//Compute from an JPEG buffer a Mat
void jpeg_compressor::decompress(unsigned char *buffer_in,unsigned char *buffer_out){
	std::vector<unsigned char> v;
	//filling the vector with the jpeg data
	//std::cerr << "filling" << std::endl;
	for(int i = 0; i < compressed_size; i++){
	//v[i] = buffer_in[i];
	v.push_back(buffer_in[i]);
	}
	//std::cerr << "end filling" << std::endl;
	cv::Mat img = cv::imdecode(v, CV_LOAD_IMAGE_GRAYSCALE);
	//cv::imshow("img",img);
	//cv::waitKey(0);
	/*for(int o = 0; o < width; o++){
	std::cerr << (img.data)[o];	
	}
std::cerr<<std::endl;*/
	cv::imshow( "Decompress img", img );
    	cv::waitKey(0); 
	memcpy(buffer_out,img.data,(width*height));
}
