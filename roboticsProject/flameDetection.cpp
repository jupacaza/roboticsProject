#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <queue>
#define imageHeight 480
#define imageWidth 640
using std::cout;
using std::cin;
using std::endl;
using namespace cv;
vector <Mat> hsv_planes;
int coordinateX = 0, coordinateY = 0;
int redClick = 0, blueClick = 0, greenClick = 0;
int hueClick = 0, saturationClick = 0, valueClick = 0;
int maxRed = 0, maxBlue = 0, maxGreen = 0;
int minRed = 0, minBlue = 0, minGreen = 0;
int nClicks = 0;
RNG rng(12345);
Mat currentImage = Mat(imageHeight, imageWidth, CV_8UC3);
Mat gammaImage = Mat(imageHeight, imageWidth, CV_8UC3);
Mat complementedImage = Mat(imageHeight, imageWidth, CV_8UC3);
Mat hsvImage = Mat(imageHeight, imageWidth, CV_8UC3);
Mat binImageYellow = Mat(imageHeight, imageWidth, CV_8UC1);
Mat binImageGreen = Mat(imageHeight, imageWidth, CV_8UC1);
Mat binImageGreenDetail = Mat(imageHeight, imageWidth, CV_8UC1);
Mat binImage = Mat(imageHeight, imageWidth, CV_8UC1);
Mat filledImage = Mat(imageHeight, imageWidth, CV_8UC1);
Mat grayImage = Mat(imageHeight, imageWidth, CV_8UC3);

void mouseCoordinates(int event, int x, int y, int flags, void* param);
Mat correctGamma(Mat &img, double gamma);
void complementImage(const Mat &sourceImage, Mat &destinationImage);
void thresholdImage(const Mat &sourceImage, Mat &destinationImage);
void deteccion_fuego(Mat frame);
void print();

int main()
{
	VideoCapture cameraFeed;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat element = getStructuringElement(MORPH_RECT, Size(11, 23), Point(-1,-1));
	Mat cannyOutput;
	//Mat kernel = Mat::ones(Size(5, 5), CV_8UC1);
	char key = ' ';
	double gamma = 0.1;
	int counter = 0;
	int thresh = 100;
	char freeze = 0;
	cameraFeed.open(0);
	//namedWindow("Camera Feed");
	//namedWindow("Complemented image");
	namedWindow("HSV");
	//setMouseCallback("Complemented image", mouseCoordinates);
	setMouseCallback("HSV", mouseCoordinates);
	//imshow("Camera Feed", currentImage);
	
	while(key != 27)
	{
	//	system("cls");
	//	print();
		if(key == 'f')
		{
			freeze = !freeze;
			key = '0';
		}
		if(!freeze)
		{
			cameraFeed >> currentImage;
			gammaImage = correctGamma(currentImage, 0.2);
			complementImage(gammaImage, complementedImage);
			cvtColor(complementedImage, hsvImage, CV_BGR2HSV);
			imshow("HSV", hsvImage);
			//split(hsvImage, hsv_planes);
			inRange(hsvImage, Scalar(85, 230, 185), Scalar(106, 255, 225), binImageYellow);
			inRange(hsvImage, Scalar(90, 250, 5), Scalar(120, 255, 144), binImageGreen);
			bitwise_or(binImageYellow, binImageGreen, binImage);
			dilate(binImage, filledImage, element);
			Canny(filledImage, cannyOutput, thresh, thresh *2, 3);
			findContours(cannyOutput, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

			Mat drawing = Mat::zeros( cannyOutput.size(), CV_8UC3);
			for(unsigned int i = 0; i < contours.size(); i++ )
			{
			  Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
			  drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
			}
			//Rect rect = boundingRect(filledImage);
			//rectangle(filledImage, Point(rect.x, rect.y),Point(rect.x + rect.width, rect.y + rect.height), Scalar(255), 1, 4);
			//morphologyEx(binImage, filledImage, MORPH_CLOSE, element);
			//thresholdImage(hsvImage, binImage);
			//thresholdImage(complementedImage, binImage);
			//deteccion_fuego(currentImage);
			//imshow("Camera Feed", currentImage);
			//imshow("Gamma corrected", gammaImage);
			//imshow("Complemented image", complementedImage);
			//morphologyEx(binImage, binImage, MORPH_OPEN, element);
			//imshow("Yellow", binImageYellow);
			//imshow("Green", binImageGreen);
			imshow("Binary", filledImage);
			imshow("Contours", drawing);
		}
		key = waitKey(15);
		//Sleep(15);
	}
	return 0;
}
void print()
{
	cout << "X: " << coordinateX << ", Y: " << coordinateY << endl; 
	cout << "R: " << redClick << ", G: " << greenClick << ", B: " << blueClick << endl;
	cout << "H: " << hueClick << ", S: " << saturationClick << ", V: " << valueClick << endl;
}

void mouseCoordinates(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
	    
	  /*CV_EVENT_MOUSEMOVE - when the mouse pointer moves over the specified window
		CV_EVENT_LBUTTONDOWN - when the left button of the mouse is pressed on the specified window
		CV_EVENT_RBUTTONDOWN - when the right button of the mouse is pressed on the specified window
		CV_EVENT_MBUTTONDOWN - when the middle button of the mouse is pressed on the specified window
		CV_EVENT_LBUTTONUP - when the left button of the mouse is released on the specified window
		CV_EVENT_RBUTTONUP - when the right button of the mouse is released on the specified window
		CV_EVENT_MBUTTONUP - when the middle button of the mouse is released on the specified window */

        case CV_EVENT_LBUTTONDOWN:
            coordinateX = x;
            coordinateY = y;
            redClick = currentImage.at<Vec3b>(y, x)[2];
	        greenClick = currentImage.at<Vec3b>(y, x)[1];
	        blueClick = currentImage.at<Vec3b>(y, x)[0];

			hueClick = hsvImage.at<Vec3b>(y, x)[0];
	        saturationClick = hsvImage.at<Vec3b>(y, x)[1];
	        valueClick = hsvImage.at<Vec3b>(y, x)[2];

			/*  Draw a point */
            //points.push_back(Point(x, y));
            break;
        case CV_EVENT_RBUTTONDOWN:
            break;
    }
}

Mat correctGamma(Mat &img, double gamma) {
	double inverse_gamma = 1.0 / gamma;
 
	Mat lut_matrix(1, 256, CV_8UC1);
	uchar * ptr = lut_matrix.ptr();
	for(int i = 0; i < 256; i++)
	ptr[i] = (int)(pow((double) i / 255.0, inverse_gamma) * 255.0);
 
	Mat result;
	LUT(img, lut_matrix, result);
 
	return result;
}

void complementImage(const Mat &sourceImage, Mat &destinationImage)
{
	if (destinationImage.empty())
		destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());

	for (int y = 0; y < sourceImage.rows; ++y)
		for (int x = 0; x < sourceImage.cols; ++x)
			for (int i = 0; i < sourceImage.channels(); ++i)
			{
				destinationImage.at<Vec3b>(y, x)[i] = 255 - sourceImage.at<Vec3b>(y, x)[i];
				//destinationImage.at<Vec3b>(y, sourceImage.cols - 1 - x)[i] = sourceImage.at<Vec3b>(y, x)[i];
			}
}
Point findFlame(const Mat &sourceImage)
{
	Point flameSeed = Point();
	for (int y = 0; y < sourceImage.rows; ++y)
	{
		for (int x = 0; x < sourceImage.cols; ++x)
		{
			if(hsvImage.at<Vec3b>(y, x)[0] == 0 &&
				hsvImage.at<Vec3b>(y,x)
		}
	}
	return Point();
}
//
//void thresholdImage(const Mat &sourceImage, Mat &destinationImage)
//{
//	if (destinationImage.empty())
//		destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());
//
//	for (int y = 0; y < sourceImage.rows; ++y)
//		for (int x = 0; x < sourceImage.cols; ++x)
//		{
//			//if(sourceImage.at<Vec3b>(y, x)[2] >  194 && sourceImage.at<Vec3b>(y, x)[2] < 255) //Red, V
//			//{
//			//	if(sourceImage.at<Vec3b>(y, x)[1] > 229 && sourceImage.at<Vec3b>(y, x)[1] < 255) //Green, S
//			//	{
//			//		if(sourceImage.at<Vec3b>(y, x)[0] > 95 && sourceImage.at<Vec3b>(y, x)[0] < 106) //Blue, H
//			//		{
//			//			destinationImage.at<uchar>(y, x) = 255;
//			//		}
//			//	}
//			//}
//			if(sourceImage.at<Vec3b>(y, x)[0] > 95 && sourceImage.at<Vec3b>(y, x)[0] < 106)
//			{
//				if(sourceImage.at<Vec3b>(y, x)[1] > 229)
//				{
//					if(sourceImage.at<Vec3b>(y, x)[0] > 194)
//					{
//						destinationImage.at<uchar>(y, x) = 255;
//					}
//				}
//			}
//			else
//			{
//				destinationImage.at<uchar>(y, x) = 0;
//			}
//		}
//}
//
//void deteccion_fuego(Mat frame){
//	int dilation_size = 0;
//	vector<vector<Point> > contours;
//	Mat canny_output;
//	Mat thr1(frame.rows, frame.cols, CV_8UC1);
//	Mat src_gray; 
//	vector<Vec4i> hierarchy;
//	Rect aux;
//	cvtColor(frame, src_gray, CV_RGB2GRAY);
//	adaptiveThreshold(src_gray, thr1, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY , 3, 1);
//	threshold( src_gray, thr1, 250, 255,THRESH_BINARY );
//	//Canny( thr1, canny_output, 255, 255, 3);
//	Mat element = getStructuringElement(MORPH_RECT, Size(2 * dilation_size + 1, 2 * dilation_size + 1), Point(dilation_size, dilation_size));
//	 morphologyEx( thr1, thr1, MORPH_CLOSE , element );
//
//	imshow("Camara...",frame);	
//	imshow("Threshold", thr1);
//	waitKey(30);
//	
// 
//  /// Find contours
//  findContours(thr1, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
// 
//  //cout<<" "<<endl;
//   vector<Rect> boundRect(contours.size());
//   vector<vector<Point>> contours_poly(contours.size());
//
//  /// Draw contours
//  Mat drawing = Mat::zeros(thr1.size(), CV_8UC3);
//  	if (contours.size()>0)
//  //for( int i = 0; i< contours.size()-contours.size()/2; i++ )
//     {
//		double a = contourArea(contours[0],false);  //  Find the area of contour
//		if(a>5){					  //25
//		system("cls");
//		 cout<<"Deteciion de fuego..."<<endl;
//		 //waitKey(150);
//		 //fire(Pic18);
//       drawContours( thr1, contours, 0,  Scalar( 255, 255, 255), 2, 8, hierarchy, 0, Point() );
//	   //cout<<"p1"<<endl;
//	   approxPolyDP( Mat(contours[0]), contours_poly[0], 3, true );
//	   //cout<<"p2"<<endl;
//       boundRect[0] = boundingRect(Mat(contours_poly[0]));
//	   //cout<<"p3"<<endl;
//	   //waitKey();
//	   aux = boundRect[0];
//	   if (boundRect[0].width / boundRect[0].height <= 1){
//		  // cout<<"p4"<<endl;
//		rectangle( drawing, boundRect[0].tl(), boundRect[0].br(), Scalar( 255, 255, 255), 2, 8, 0 );
//		//cout<<"p5"<<endl;
//		rectangle( frame, boundRect[0].tl(), boundRect[0].br(), Scalar( 0, 0, 255), 2, 8, 0 );
//		
//		//cout<<"boundRect[0].x"<<boundRect[0].x<<endl;
//		//cout<<"boundRect[0].y"<<boundRect[0].y<<endl;
//		//waitKey();
//		
//		if(boundRect[0].x> frame.cols/2) 
//		{
//			putText(frame, "Right", Point(5, 440),  FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 1, 8, false);
//		}
//		else if (boundRect[0].x> frame.cols/2-5 && boundRect[0].x< frame.cols/2+5) 
//		{
//			putText(frame, "Center", Point(5, 440),  FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 1, 8, false);
//		}
//		else 
//		{
//			putText(frame, "Left", Point(5, 440),  FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 1, 8, false);
//		}
//
//		imshow("Flama detectada", frame);
//	   	   waitKey(50);
//	   }
//	    }else{
//			//cout<<"p4"<<endl;
//			system("cls");
//			rectangle( frame, aux.tl(), aux.br(), Scalar( 0, 0, 255), 2, 8, 0 );
//		
//			rectangle( drawing, boundRect[0].tl(), boundRect[0].br(), Scalar( 0, 0, 0), 2, 8, 0 );
//			imshow("Se ha detectado la flama", frame);
//	   }
//	   //system("cls");
//	   imshow("Flama detectada", frame);
//	   imshow( "Contours", drawing );
//}
//}