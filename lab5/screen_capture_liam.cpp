/*

Liam McCarthy, Wyatt Colburn
CPE442 Real Time Embedded Systems
Lab 5 - Vectorized/Parallelized, Threaded Sobel

Uses ARM Neon vecotr intrinsics to speed up a simple sobel
filter on a raspberry pi

Header file - contains all functions and logic

*/


#include "sobel_threaded.hpp"

struct sobel_thread_args
{
    pthread_t* thread; //pointer to specific thread
    Mat* inMat; //pointer to input frame
    Mat* grayMat; //pointer to shared grayscale mat
    Mat* outMat;  //shared output mat

    int thread_id; //specify which thread

    int start_row; //specify where to start and end ops
    int end_row; //calculated from thread id
};

//sobel operation function
void* sobel_thread(void* sobelArgs){
    struct sobel_thread_args *threadArgs = (struct sobel_thread_args *)sobelArgs; //cast back

    for(int img_row = threadArgs->start_row; img_row < threadArgs->end_row; img_row++){ //iterate rows
        for (int img_col = 0; img_col < threadArgs->inMat->cols; img_col++){ //iterate cols
            uint8_t *colors = threadArgs->inMat->ptr(img_row, img_col);//grab color data at that pixel

            //grayscale calc
            uint8_t grayVal = ((0.2126 * colors[2]) + (0.7152 * colors[1]) + (0.0722 * colors[0]));

            //put grayscale value into the grayscale mat at the right place
            threadArgs->grayMat->at<uchar>(img_row, img_col) = uchar(grayVal);
        }
    }
    if (threadArgs->thread_id == 0){ //top thread, clamp first row so it stays in bounds
        threadArgs->start_row = 1;
    }
    if (threadArgs->thread_id == 3){ //bottom thread, clamp last row
        threadArgs->end_row = threadArgs->outMat->rows - 1;
    }    
    int16x8_t TL, ML, BL, TM, BM, TR, MR, BR;
    //grayscale mat should be shared, can grab pixel values no problem
    for(int img_row = threadArgs->start_row; img_row < threadArgs->end_row; img_row++){ //iterate row
        uchar *gray_mid_row = threadArgs->grayMat->ptr<uchar>(img_row); //ptr to first pixel in current row
        uchar *gray_bottom_row = threadArgs->grayMat->ptr<uchar>(img_row + 1); //ptr to first pixel in below row
        uchar *gray_top_row = threadArgs->grayMat->ptr<uchar>(img_row - 1);
        uchar *output = threadArgs->outMat->ptr<uchar>(img_row); //output row pointer
        for (int img_col = 1; img_col < (threadArgs->grayMat->cols - 7); img_col += 8){ //iterate cols

            TL = vmovl_s8(vld1_s8(gray_top_row)); //load 8 pixels into size 8 vectors, then make all vectors size 16 without sign extend
            TM = vmovl_s8(vld1_s8(gray_top_row + 1)); //top mid
            TR = vmovl_s8(vld1_s8(gray_top_row + 2)); //top right
            ML = vmovl_s8(vld1_s8(gray_mid_row)); //middle left
            MR = vmovl_s8(vld1_s8(gray_mid_row + 2)); //middle right
            BL = vmovl_s8(vld1_s8(gray_bottom_row)); //bottom left
            BM = vmovl_s8(vld1_s8(gray_bottom_row + 1)); //bottom mid
            BR = vmovl_s8(vld1_s8(gray_bottom_row + 2)); //bottom right

            int16x8_t sobelWvect = vaddq_s16( \
            vabsq_s16( //absolute of X sobel operation
            vsubq_s16(vaddq_s16(vaddq_s16(MR, MR), vaddq_s16(TR, BR)), vaddq_s16(vaddq_s16(ML, ML), vaddq_s16(TL, BL)))), \
            vabsq_s16( //absolute of sobel Y operation
            vsubq_s16(vaddq_s16(vaddq_s16(TM, TM), vaddq_s16(TR, TL)), vaddq_s16(vaddq_s16(BM, BM), vaddq_s16(BR, BL)))));

            vst1_s8((output + 1), vmovn_s16(sobelWvect)); //stores the 8 values in a row
            gray_mid_row += 8;
            gray_top_row += 8;
            gray_bottom_row += 8;
            output += 8;
        }
        if ((threadArgs->grayMat->cols & 0x02) != 0){ //if last 2 digits are not 0 we have leftover cols


        }
    }
    return NULL;
}

//manages a 1 parent 4 child thread version of the previous sobel filter function
//creates struct threads, divides screen up by 4 rows, 
int sobel_filter_threaded(string videoName){
    VideoCapture cap(videoName);

    if (!cap.isOpened()){ //no video exists
        cout << "Failed to Open Video. Did you type the name right??" << endl; //Miguel
        exit(-1);
    }
    //make shared mats and stuff
    Mat inMat;
    pthread_t sobelThreads[NUM_THREADS];
    struct sobel_thread_args thread_args[NUM_THREADS];
    cap >> inMat; //put first frame in
    Mat grayMat(inMat.size(), CV_8UC1);
    Mat outMat(inMat.size(), CV_8UC1);

    string title = "Threaded Sobel: " + videoName;
    namedWindow(title, WINDOW_NORMAL);

    //init child thread structs
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread = &sobelThreads[i];
        thread_args[i].inMat = &inMat;
        thread_args[i].outMat = &outMat;
        thread_args[i].grayMat = &grayMat;
        thread_args[i].thread_id = i;
        thread_args[i].start_row = (i * inMat.rows) / NUM_THREADS;
        thread_args[i].end_row = ((i + 1) * inMat.rows) / NUM_THREADS;
    }
    
    while(1){ //not done with code to actually setup and take down the threads
        if(inMat.empty()){
            break;
        }
        for(int i = 0; i < NUM_THREADS; i++){ //start the threads
            pthread_create(&sobelThreads[i], nullptr, sobel_thread, &thread_args[i]);
        }
        for(int i = 0; i < NUM_THREADS; i++){ //wait for them to finish
            pthread_join(sobelThreads[i], NULL);
        }
        imshow(title, outMat);
        waitKey(2);


        cap >> inMat; //put new frame in
    }

    
}

