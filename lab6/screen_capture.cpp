//Include Libraries
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <pthread.h> 
#include <time.h>
// Namespaces
using namespace cv;
// Function Prototypes
volatile uint8_t done = 1;

pthread_barrier_t b1, b2;


struct MyStruct { 
  //init will all be the same frame, frame passed to struct 
  Mat orginal_frame;  
  Mat greyscale_frame; //keep so we can implement vectors
                       //
  Mat sobel_frame; 
  //used to set the size of the frames (1/4) of the cap 
  int rows;
  int cols;
 
  bool esc_flag;

  pthread_barrier_t b1;
  pthread_barrier_t b2;

  //inits the size of the frames
  void structFunction(int rows, int cols) {
    orginal_frame = Mat(rows, cols, CV_8UC3);
    greyscale_frame = Mat(rows, cols, CV_8UC1);
    sobel_frame = Mat(rows, cols, CV_8UC1);

  }
};

void *thread_operation(void * arg) {
  //grayscale operation portion of function
  //changes thread1.orginal_frame which sobel will eventaully use
  
  //thread function always runs, implement barriers 
  while (1) {
    pthread_barrier_wait(&b1);
    if (done == 1) {
      MyStruct* thread1 = static_cast<MyStruct*>(arg);
      for (int i = 0; i < thread1 -> orginal_frame.rows; ++i) 
      {
        for (int j = 0; j < thread1 -> orginal_frame.cols; ++j) 
        {
          Vec3b pixel = thread1 -> orginal_frame.at<Vec3b>(i, j);
          //weighted average to make greyscale pixel
          thread1 -> greyscale_frame.at<uchar>(i,j) = uchar(pixel[0] * .2126 + pixel[1] * .7152 + pixel[2] *.0722); 
        }
      }
      //sobel filter porition, 

      int sobel_x_matrix[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
      int sobel_y_matrix[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

      for (int i = 1; i < thread1 -> greyscale_frame.rows- 1; ++i) {
        for  (int j = 1; j < thread1 -> greyscale_frame.cols - 1; ++j) {
          int sumX = 0;
          int sumY = 0;

          for (int m = -1; m <= 1; ++m) {
            for (int n = -1; n <= 1; ++n) {
              sumX += thread1 -> greyscale_frame.at<uchar>(i + m, j + n) * sobel_x_matrix[m + 1][n + 1];
              sumY += thread1 -> greyscale_frame.at<uchar>(i + m, j + n) * sobel_y_matrix[m + 1][n + 1];
            }
          }
          //write pixels of sobel_frame based on calculations above
          thread1 -> sobel_frame.at<uchar>(i, j) = saturate_cast<uchar>(abs(sumX) + abs(sumY));
        }
      }
      pthread_barrier_wait(&b2);
    }
    else { 
      return NULL;
    }
  }
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("usage: image_viewer.out <Image_Path>\n");
    return -1;
  }
  VideoCapture cap(argv[1]);

  if (!cap.isOpened())
  {
    return 0;
  }
  int num_threads = 4;

  //create array of structs for threads, continously updated
  MyStruct thread_data[num_threads];
  
  //create the barriers
  pthread_t threads[num_threads];
 
  //init for the barriers
  pthread_barrier_init(&b1, NULL, num_threads + 1);
  pthread_barrier_init(&b2, NULL, num_threads + 1);

  //start the threads, create only once
  for (int i = 0; i < num_threads; ++i) {
    pthread_create(&threads[i], nullptr, &thread_operation, &thread_data[i]);
  }


  while (1)
  {
    Mat frame;
    // Capture frame-by-frame and update orginal_frame in the struct
    cap >> frame;
    if (frame.empty()) {
      done = 0;
    }
    // counter = counter + 1;

    //just hardcode lol
    thread_data[0] = MyStruct();
    thread_data[0].rows = frame.rows;
    thread_data[0].cols = frame.cols / 4 + 1;
    thread_data[0].structFunction(thread_data[0].rows, thread_data[0].cols);
    //hard code stop and start values
    thread_data[0].orginal_frame = frame.colRange((0 * frame.cols) / num_threads, (((0+1)* frame.cols) / num_threads)+ 1); 
    thread_data[0].esc_flag = true;
        
    thread_data[1] = MyStruct();
    thread_data[1].rows = frame.rows;
    thread_data[1].cols = frame.cols / 4 + 2;
    thread_data[1].structFunction(thread_data[1].rows, thread_data[1].cols);
    //hard code stop and start values
    thread_data[1].orginal_frame = frame.colRange((1 * frame.cols) / num_threads - 1, (((1+1)* frame.cols) / num_threads)+ 1); 
    thread_data[1].esc_flag = true;
    
    thread_data[2] = MyStruct();
    thread_data[2].rows = frame.rows;
    thread_data[2].cols = frame.cols / 4 + 2;
    thread_data[2].structFunction(thread_data[2].rows, thread_data[2].cols);
    //hard code stop and start values
    thread_data[2].orginal_frame = frame.colRange((2 * frame.cols) / num_threads - 1, (((2+1)* frame.cols) / num_threads)+ 1); 
    thread_data[2].esc_flag = true;
    

    thread_data[3] = MyStruct();
    thread_data[3].rows = frame.rows;
    thread_data[3].cols = frame.cols / 4 + 1;
    thread_data[3].structFunction(thread_data[3].rows, thread_data[3].cols);
    //hard code stop and start values
    thread_data[3].orginal_frame = frame.colRange((3 * frame.cols) / num_threads - 1, (((3+1)* frame.cols) / num_threads)); 
    thread_data[3].esc_flag = true;
    
    pthread_barrier_wait(&b1);
    pthread_barrier_wait(&b2);
    //need to combined the threads info now
    
    
    //faster if i define size, sobel size.rows = og.rows -2, same for cols
    Mat combined_frame = Mat::zeros(frame.rows - 2, frame.cols - 2, CV_8UC1);
    
    //crop boundries
    Range row1(1, thread_data[0].sobel_frame.rows -1);
    Range col1(1, thread_data[0].sobel_frame.cols - 1);
    Range row2(1, thread_data[1].sobel_frame.rows -1);
    Range col2(1, thread_data[1].sobel_frame.cols -1);
      
    Range row3(1, thread_data[2].sobel_frame.rows -1);
    Range col3(1, thread_data[2].sobel_frame.cols - 1);
    Range row4(1, thread_data[3].sobel_frame.rows -1);
    Range col4(1, thread_data[3].sobel_frame.cols -1);
      
    hconcat(thread_data[0].sobel_frame(row1, col1), thread_data[1].sobel_frame(row2, col2), combined_frame); 
    hconcat(combined_frame, thread_data[2].sobel_frame(row3, col3), combined_frame);
    hconcat(combined_frame, thread_data[3].sobel_frame(row4, col4), combined_frame);
    // Display the combined frame
    imshow("Combined Sobel Filter", combined_frame);
    
    // Wait for a short time (e.g., 25 ms) before the next frame
    char c = (char)waitKey(25);
    if (c == 27) {
      thread_data[0].esc_flag = false;  // Break the loop if 'ESC' key is pressed
    }

    if (thread_data[0].esc_flag == false)
    {
      break;
    }
  }

  cap.release();
  destroyAllWindows();
  pthread_barrier_destroy(&b1);
  pthread_barrier_destroy(&b2);
  
  return 0;
}
