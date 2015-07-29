
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "zinjpeg_decode_interface.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

#define FILE_NAME "/Users/pandor/Desktop/out.ZinJpeg"
#define QUALITY 80

/******************************************************************************
 Main
 ******************************************************************************/

int main(int argc, char**argv) {
    
    //creating buffer to store the zinjpeg image
    unsigned char buffer_image[640*480*4]; // Assume that the input jpgpkg
    unsigned char* buff_img = buffer_image;
    
    //open image from file
    FILE *fp;
    fp = fopen(FILE_NAME, "rb" );
    int c;
    c = fgetc(fp);
    int input_file_length = 0;
    cerr << "start read file ";
    while (c != EOF) {
        buffer_image[input_file_length] = (unsigned char)c;
        c = fgetc(fp);
        cerr << c;
        input_file_length++;
    }
    cerr << endl << endl << endl << input_file_length << endl;
    fclose(fp);
    
    unsigned char *out_buffer;
    //declare a decompressor
    zinjpeg_decompressor::zinjpeg_decompressor zin_comp(80,2);
    out_buffer = zin_comp.decompress_zinjpeg(buff_img, input_file_length);
    
    
    
    ofstream myfile_zin;
    myfile_zin.open ("/Users/pandor/Desktop/outZinJpeg.jpg");
    
    
    unsigned char* ptr = out_buffer;
    
    //transform outbuffer in a vector
    std::vector<unsigned char> vec;
    for (int i = 0; i < zin_comp.getSize(); i++) {
        vec.push_back(ptr[i]);
        cerr << ptr[i];
        myfile_zin << ptr[i];
    }
    myfile_zin.close();
    //print the image with opencv
    Mat img = imdecode(vec, CV_LOAD_IMAGE_GRAYSCALE);
    imshow("image window", img);
    waitKey(0);

}

