#include "metrics.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "compressor.h"
#include "jpeg_enc_interface.h"
#include "../inc/pdfgen.h"
#include "../inc/lossmodels.h"
//#include "../inc/jpgd.h"
#include "../inc/jpeg_decoder.h"

using namespace std;
using namespace cv;

/*************************************************************************/
/***************** Metrics constructor                 *******************/
/*************************************************************************/
/*
Set a new metrics object. Need to pass a compressor in order to construct
 the object. Goto Compressor class to see more details about compressors.
*/
/*************************************************************************/
/*************************************************************************/

metrics::metrics(compressor *c)
{
	this->comp = c;
	//setting width and height thanks to the original image
	//this->width = width;
	//this->height = height;
	//this->in_buffer = char *buff
	this->in_buffer = NULL;
	this->out_buffer = NULL;
}
metrics::~metrics(){
    //release all buffer created during metric processing
if (this->out_buffer != NULL) { delete [] this->out_buffer;}
if (this->in_buffer != NULL) { delete [] this->in_buffer;}
}

/*************************************************************************/
/***************** Metrics Processing method           *******************/
/*************************************************************************/
/*
This method is the main method of the Metrics class.
 /!\/!\/!\/!\/!\ USE THIS METHOD BEFORE CALLING "get" METHODS/!\/!\/!\/!\
 
 Just pass the file-name of your picture in your computer.
 The method, will compress, decompress the image and will update the image
 buffer to compute calculation (metrics outputs).
 */
/*************************************************************************/
/*************************************************************************/
void metrics::process(std::string file_name){
	
	//set the file_name for future calculation
	this->file_name = file_name;
	//loading the original image from the file name in GRAYSCALE
	Mat image = imread(file_name, CV_LOAD_IMAGE_GRAYSCALE);
	//set correct size for the image from file name
	
	this->width = image.cols;
	this->height = image.rows;
cout<< "deb : "<< width << height << endl; 
	//fill the input buffer with the image we have red
	//set a new buffer thanks to the size of the image
	if (this->in_buffer != NULL) { this->in_buffer = new unsigned char[width * height]; }
	else {delete [] this->in_buffer; this->in_buffer = new unsigned char[width * height];}
	this->in_buffer = (unsigned char*)memcpy(in_buffer, (unsigned char*)image.data, width*height );
	//compress the image
	//set a new out buffer thanks to image width and height * 4 (min)
	if (this->out_buffer != NULL) { this->out_buffer = new unsigned char[width * height * 10]; }
	else {delete [] this->out_buffer; this->out_buffer = new unsigned char[width * height * 10];}

	//compress the image with correct width and height, result is in out_buffer, return compressed_size
	//first send size of the image to compressor
	comp->setSize(width,height);
	unsigned char tmp_buff[width*height];
	cerr << " compress start" << endl;
	this->compressed_size = comp->compress(in_buffer,tmp_buff);
	cerr << " compress end" << endl;
	//comp->decompress(tmp_buff, out_buffer);
	Jpeg::Decoder decoder(tmp_buff, compressed_size);
	out_buffer = decoder.GetImage();

}

/*************************************************************************/
/***************** Metrics Processing method           *******************/
/*************************************************************************/
/*
 This method is the main method of the Metrics class.
 /!\/!\/!\/!\/!\ USE THIS METHOD BEFORE CALLING "get" METHODS/!\/!\/!\/!\
 
Need file-name , PLR and BL are the two value for noise generation.
 The seed is the seed of the noise generator ( leave it blank if you dont
 want to set a special seed).
 
 The method, will compress, add noise, decompress the image and will 
 update the image buffer to compute calculation (metrics outputs).
 
 If the image is too noisy and the decompressor is not able to reconstruct
 the image, you will have a failure message.
 */
/*************************************************************************/
/*************************************************************************/
void metrics::process(std::string file_name, float PLR, int BL){

	//set the file_name for future calculation
	this->file_name = file_name;
	//loading the original image from the file name in GRAYSCALE
	Mat image = imread(file_name, CV_LOAD_IMAGE_GRAYSCALE);
	//set correct size for the image from file name
	
	this->width = image.cols;
	this->height = image.rows;
cout<< "deb : "<< width << height << endl; 
	//fill the input buffer with the image we have red
	//set a new buffer thanks to the size of the image
	if (this->in_buffer != NULL) { this->in_buffer = new unsigned char[width * height]; }
	else {delete [] this->in_buffer; this->in_buffer = new unsigned char[width * height];}
	this->in_buffer = (unsigned char*)memcpy(in_buffer, (unsigned char*)image.data, width*height );
	//compress the image
	//set a new out buffer thanks to image width and height * 4 (min)
	if (this->out_buffer != NULL) { this->out_buffer = new unsigned char[width * height * 10]; }
	else {delete [] this->out_buffer; this->out_buffer = new unsigned char[width * height * 10];}

	//compress the image with correct width and height, result is in out_buffer, return compressed_size
	//first send size of the image to compressor
	comp->setSize(width,height);
	unsigned char tmp_buff[width*height];
	cerr << " compress start" << endl;
	this->compressed_size = comp->compress(in_buffer,tmp_buff);
	//Start noise generator
	//declare a buffer to store noise
	char *noise_buffer = (char*)malloc(sizeof(char)*compressed_size);
	char *ptr = noise_buffer;
	int i = 0, j = 0;
	//create noise from gilbert elliot method
    //TODO : include new gilbert elliott class noise generator
	TwoStateGilbertElliot(noise_buffer,compressed_size,PLR,BL,&Nlosses);
	//Apply the noise on the buffer
	//int SOF = 0;
	//while(tmp_buff[SOF] != 0xFF && tmp_buff[SOF+1]!= 0xDA){SOF++;}
	/*for(long i = compressed_size - 60 ; i < 1; i++){
		if(buffer[i] == 0){
			//flip the bit
			if(tmp_buff[i] == 0){
			tmp_buff[i] = 1;			
			} else{tmp_buff[i] = 0;}
		}		
	}*/
	Jpeg::Decoder decoder(tmp_buff, compressed_size);
	out_buffer = decoder.GetImage();
	//output for debug
	FILE *fp;
	fp=fopen("timage.ppm", "wb");
	fprintf(fp, "P%d\n%d %d\n255\n", decoder.IsColor() ? 6 : 5, decoder.GetWidth(), decoder.GetHeight());
    fwrite(decoder.GetImage(), 1, decoder.GetImageSize(), fp);
  	fclose(fp);
	//end debug
	cerr << " compress end" << endl;
	cv::Mat img(120,160,CV_8UC1,decoder.GetImage());
	cv::imshow( "Decompress metric img", img );
    	cv::waitKey(0);
	//comp->decompress(tmp_buff, out_buffer);
}


char metrics::getBuffstate(void){
if(this->out_buffer != NULL) {return 1;}
return 0;
}
long original_size( std::string file_name ){

	FILE * pFile;
	  long Original_img_size;

	  pFile = fopen ((file_name).c_str(),"rb");
	  if (pFile==NULL) perror ("Error opening file");
	  else
	  {
	    fseek (pFile, 0, SEEK_END); 
	    Original_img_size = ftell (pFile);
	    fclose (pFile);
	}
	return Original_img_size;
}

/*************************************************************************/
/***************** Compression ratio computation       *******************/
/*************************************************************************/
/*
 Call this method to obtain the Compression ratio between original image
 and compressed one.
 */
/*************************************************************************/
/*************************************************************************/

int metrics::getCompression_ratio(void){
	long ratio;
	ratio = ((compressed_size * 10000) / original_size(this->file_name) );
	return (int)ratio;
}

/*************************************************************************/
/***************** MSE computation                     *******************/
/*************************************************************************/
/*
This is the internal function for PSNR computation. You are not able to
 use it in main function.
 */
/*************************************************************************/
/*************************************************************************/

float metrics::getMSE (void)
{
	float mse = 0.0;
	for(int i = 0; i< (this->width*this->height); i++){
	mse += (((this->in_buffer)[i] - (this->out_buffer)[i])*((this->in_buffer)[i] - (this->out_buffer)[i]));
	}
	mse = mse / (this->width * this->height);
	return mse;
}

/*************************************************************************/
/***************** Peak Signal to Noise Ratio computation ****************/
/*************************************************************************/
/*
Give to you the PSNR between original file and compressed/decompressed one.
 */
/*************************************************************************/
/*************************************************************************/

float metrics::getPSNR (void){
	double psnr = 0.0;
	psnr = 10 * log10((255.0*255.0) / metrics::getMSE() );
	return psnr;
}

/*************************************************************************/
/***************** MSSIM computation                   *******************/
/*************************************************************************/
/*
Give to you the MSSIM between the original image and the 
 compressed/decompressed image.
 MSSIM is the most accurate comparaison for human vision accuracy in
 metrics class.
 The MSSIM is from openCV code examples. GNU
 */
/*************************************************************************/
/*************************************************************************/

double metrics::getMSSIM(void)
{
	cv::Mat i1;
	cv::Mat i2(height,width,CV_8UC1,out_buffer);
	cv::imshow( "image i2", i2 );
    	cv::waitKey(0); 
	const char* file_out_name = "compressed_image";
	//const char* file_in_name = file_name;
	/*FILE *out_file = fopen(file_out_name, "wb");
	if (!out_file) {
			printf("Cannot open the file!");
			exit(0);
		}
	fwrite(out_buffer, compressed_size, 1, out_file);
	fclose(out_file);*/
	i1 = imread(file_name, CV_LOAD_IMAGE_GRAYSCALE);
	cv::imshow( "image i1", i1 );
    	cv::waitKey(0); 
	cout << "open i1" << endl;
	//i2 = imread(file_out_name,CV_LOAD_IMAGE_GRAYSCALE);
	//cout << "open i2" << endl;
    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/
    int d     = CV_32F;

    Mat I1, I2;
    i1.convertTo(I1, d);           // cannot calculate on one byte large values
    i2.convertTo(I2, d);

    Mat I2_2   = I2.mul(I2);        // I2^2
    Mat I1_2   = I1.mul(I1);        // I1^2
    Mat I1_I2  = I1.mul(I2);        // I1 * I2

    /*************************** END INITS **********************************/

    Mat mu1, mu2;   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);

    Mat mu1_2   =   mu1.mul(mu1);
    Mat mu2_2   =   mu2.mul(mu2);
    Mat mu1_mu2 =   mu1.mul(mu2);

    Mat sigma1_2, sigma2_2, sigma12;

    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3;

    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    Mat ssim_map;
    divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;

    cv::Scalar mssim = mean( ssim_map ); // mssim = average of ssim map  
  return sum(mssim)[0];
}

unsigned char* metrics::getOut(void){
	return out_buffer;
}
