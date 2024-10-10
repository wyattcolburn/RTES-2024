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
  Mat greyscale_frame;
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
      
      //need 8 arrays which hold 8 signed ints
      int16x8_t topLeft, topMiddle, topRight, middleLeft, middleRight, bottomLeft, bottomMiddle, bottomRight;
      //pointer everyrow
      //
      //
      int row_length = thread1 -> greyscale_frame.cols;

      for (int row_counter = 0; row_counter < thread1 -> greyscale_frame.rows; row_counter++) {
        uchar *pointer = thread1 -> greyscale_frame->ptr<uchar>(row_counter);
        uchar * output_pointer = thread1 -> sobel_frame ->ptr<uchar>(row_counter);

        for (int col_counter = 0; col_counter < thread1 -> greyscale_frame.cols; col_counter += 8) {
        
          

          topLeft = vmovl_s8(vld1_s8(pointer - 1 - row_length));
          topMiddle = vmolv_s8(vld1_s8(pointer - row_length));
          topRight = vmolv_s8(vld1_s8(pointer - row_length + 1));
          middleLeft = vmolv_s8(vld1_s8(pointer - 1));
          middleRight = vmolv_s8(vld1_s8(pointer + 1));
          bottomLeft = vmolv_s8(vld1_s8(pointer - 1 + row_length));
          bottomMiddle = vmolv_s8(vld1_s8(pointer + row_length));
          bottomRight = vmolv_s8(vld1_s8(pointer + 1 +row_length));


          int16x8_t sobelWvect = vaddq_s16( \
          vabsq_s16( //absolute of X sobel operation
          vsubq_s16(vaddq_s16(vaddq_s16(MR, MR), vaddq_s16(TR, BR)), vaddq_s16(vaddq_s16(ML, ML), vaddq_s16(TL, BL)))), \
          vabsq_s16( //absolute of sobel Y operation
          vsubq_s16(vaddq_s16(vaddq_s16(TM, TM), vaddq_s16(TR, TL)), vaddq_s16(vaddq_s16(BM, BM), vaddq_s16(BR, BL)))));

          vst1_s8((output_pointer +1), vmovn_s16(sobelWvect)); 
          pointer += 8;
          output_pointer += 8; 
                    
        }
      }
      //
      // for (int i = 1; i < thread1 -> greyscale_frame.rows- 1; ++i) {
      //   for  (int j = 1; j < thread1 -> greyscale_frame.cols - 1; ++j) {
      //     int sumX = 0;
      //     int sumY = 0;
      //
      //     for (int m = -1; m <= 1; ++m) {
      //       for (int n = -1; n <= 1; ++n) {
      //         sumX += thread1 -> greyscale_frame.at<uchar>(i + m, j + n) * sobel_x_matrix[m + 1][n + 1];
      //         sumY += thread1 -> greyscale_frame.at<uchar>(i + m, j + n) * sobel_y_matrix[m + 1][n + 1];
      //       }
      //     }
      //     //write pixels of sobel_frame based on calculations above
      //     thread1 -> sobel_frame.at<uchar>(i, j) = saturate_cast<uchar>(abs(sumX) + abs(sumY));
      //   }
      // }
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
    for (uint8_t i = 0; i < num_threads; ++i) {
      thread_data[i].rows = frame.rows /4;
      thread_data[i].cols = frame.cols;
      thread_data[i].structFunction(thread_data[i].rows, thread_data[i].cols);
      thread_data[i].orginal_frame = frame.rowRange((i * frame.rows) / num_threads, (((i + 1)* frame.rows /num_threads)));
      thread_data[i].esc_flag = true;
    }
    
    pthread_barrier_wait(&b1);
    pthread_barrier_wait(&b2);
    //need to combined the threads info now
    
    
    //faster if i define size, sobel size.rows = og.rows -2, same for cols
    Mat combined_frame = Mat::zeros(frame.rows - 8, frame.cols - 8, CV_8UC1);
    
    //crop boundries, hard 
    Range crop_row(1, thread_data[0].sobel_frame.rows - 1);
    Range crop_col(1, thread_data[0].sobel_frame.cols - 1);
      
    vconcat(thread_data[0].sobel_frame(crop_row, crop_col), thread_data[1].sobel_frame(crop_row, crop_col), combined_frame); 
    vconcat(combined_frame, thread_data[2].sobel_frame(crop_row, crop_col), combined_frame);
    vconcat(combined_frame, thread_data[3].sobel_frame(crop_row, crop_col), combined_frame);
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
