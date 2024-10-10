//Include Libraries
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
 
// Namespaces
using namespace cv;


int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: image_viewer.out <Image_Path>\n");
        return -1;
    }
    Mat img_color = imread(argv[1], IMREAD_COLOR); //gets the rbg pixel data
    Mat img_grayscale = imread(argv[1], IMREAD_GRAYSCALE);
    Mat img_unchanged = imread(argv[1], IMREAD_UNCHANGED);
    if ( !img_grayscale.data)
    {
        printf("No image data \n");
        return -1;
    }
    //make windows
    namedWindow("RGB Color", WINDOW_AUTOSIZE);
    namedWindow("Grayscale", WINDOW_AUTOSIZE);
    namedWindow("True Color", WINDOW_AUTOSIZE);
 
    //put image in windows
    imshow( "RGB Color", img_color ); 
    imshow( "Grayscale", img_grayscale );
    imshow( "True Color", img_unchanged ); 
 
    //keystroke destroys windows
    waitKey(0);                        
    destroyAllWindows();
    return 0;
}
