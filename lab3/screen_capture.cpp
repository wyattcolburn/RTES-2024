//Include Libraries
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <chrono>
 
// Namespaces
using namespace cv;
using namespace std;



Mat sobel_filter(const Mat& greyscale_color) {
    Mat sobel_filter(greyscale_color.rows, greyscale_color.cols, CV_8UC1);
    int sobel_x_matrix[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int sobel_y_matrix[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

    for (int i = 1; i < greyscale_color.rows - 1; ++i) {
        for (int j = 1; j < greyscale_color.cols - 1; ++j) {
            int sumX = 0;
            int sumY = 0;

            for (int m = -1; m <= 1; ++m) {
                for (int n = -1; n <= 1; ++n) {
                    sumX += greyscale_color.at<uchar>(i + m, j + n) * sobel_x_matrix[m + 1][n + 1];
                    sumY += greyscale_color.at<uchar>(i + m, j + n) * sobel_y_matrix[m + 1][n + 1];
                }
            }

            sobel_filter.at<uchar>(i, j) = saturate_cast<uchar>(abs(sumX) + abs(sumY));
        }
    }
    return sobel_filter;
}


Mat to442_grayscale(Mat img_color) {
    
    Mat greyscale(img_color.rows, img_color.cols, CV_8UC1);
    for (int i = 0; i < img_color.rows; ++i) 
    {
        for (int j = 0; j < img_color.cols; ++j) 
        {
            Vec3b pixel = img_color.at<Vec3b>(i, j);
            greyscale.at<uchar>(i,j) = uchar(pixel[0] * .2126 + pixel[1] * .7152 + pixel[2] *.0722);
        }
    }
    return greyscale;    
}

int display(std::string input_file) {
    VideoCapture cap(input_file);
    
    if(!cap.isOpened())
    {
        return -1;
    } 
    auto start = chrono::steady_clock::now(); //get start time
    int fpsCounter = 0;
    while(1)
    {
      Mat frame;

       // Capture frame-by-frame-then apply gray scale to frame, then bring back to window
      cap >> frame;
      fpsCounter +=1; 
      // If the frame is empty, break immediately
      if (frame.empty())
        break;
      
      Mat grayscale_frame = frame;
      Mat sobel_frame = frame; 
      
      // Display the resulting frame
      grayscale_frame = to442_grayscale(grayscale_frame);
      sobel_frame = sobel_filter(grayscale_frame);
      
      imshow( "Sobel Filter", sobel_frame);
      
      // Press  ESC on keyboard to exit
      char c=(char)waitKey(25);
      if(c==27)
        break;
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    double fps = static_cast<double>(fpsCounter) / (duration / 1000.0);

    cout << "Averaged per second: " << fps << endl;
    cap.release();
    
    destroyAllWindows();
    // clock_t fpsEnd = clock();
    // float totaltime = (float)(fpsEnd = fpsStart);
    // cout << "totalTime" << totaltime;
    // float avg_fps = (float)(fpsEnd - fpsStart) / fpsCounter;
    // cout << "average fps" << avg_fps << endl;
    
    return 0;
}

int main(int argc, char** argv )
{
    cout << "start" << endl;
    if ( argc != 2 )
    {
        printf("usage: image_viewer.out <Image_Path>\n");
        return -1;
    }
    display(argv[1]);
    return 0;
}

