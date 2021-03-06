#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	// File path for input image
	std::string projectSrcDir = PROJECT_SOURCE_DIR;
	std::string inputImagePath = projectSrcDir + "/Data/serval.png";
	string outputGrayImgPath = projectSrcDir + "/Data/serval_gray.png";

	//// Task 1: Basisc //////////////////////////////////////////////////
	// Step 1: Load image using cv::imread
	 // second argument: specifiy image color (1=color, 0=gray)
    // or: CV_LOAD_IMAGE_COLOR, CV_LOAD_IMAGE_GRAYSCALE
    Mat img = imread(inputImagePath, CV_LOAD_IMAGE_COLOR);
    if (!img.data)
    {
        cout << "Could not open or find the image";
        return -1;
    }
	// Step 2: Display image using cv::imshow
	imshow("img", img);
	waitKey();
	// Step 3: Convert to gray image using cv::cvtColor function
	Mat imggray;
	cvtColor(img, imggray, CV_RGB2GRAY);
	// Step 4: Save gray image by using cv::imwrite
	imwrite(outputGrayImgPath, imggray);




	//// Task 2-1: Contrast stretching //////////////////////////////////
	// Step 1: Apply contrast streching operator for a gray image
	double min, max;
	cv::minMaxLoc(imggray, &min, &max);
	Mat stretched = 255 / (max - min)*(imggray - min);
	// Step 2: Show the result
	imshow("Contrast stretching", stretched);
	waitKey();

	//// Task 2-2: Color inverting /////////////////////////////////////
	// Step 1: Apply invert operator for a color image
	Mat inverted(img.rows, img.cols, CV_8UC3);
	for (auto i = 0; i < img.rows; i++)
	{
		for (auto j = 0; j < img.cols; j++)
		{
			inverted.at<Vec3b>(i, j) = Vec3b(255, 255, 255) - img.at<Vec3b>(i, j);
		}

	}

	//1. Shortcut:
	//bitwise_not(img, inverted);
	//2. Shortcut:
	// inverted = cv::Scalar::all(255) - img;
	// Step 2: Show the result
	imshow("inverted", inverted);
	waitKey();



	//// Task 3: Local operaters ///////////////////////////////////////
	// Step 1: Apply mean filter and show the reuslt
	Mat meanF;
	boxFilter(img, meanF, -1, Size(5, 5));
	// alternatively:
	// blur( img, meanF, Size( 9, 9 ), Point(-1,-1) );
	imshow("mean filter", meanF);
	waitKey();

	// Step 2: Apply Gaussian filter and show the reuslt
	Mat gaussian;
	GaussianBlur(img, gaussian, Size(5, 5), 5);
	imshow("gaussian", gaussian);
	waitKey();

	// Step 3: Apply Bilateral filter and show the reuslt
	Mat bilateral;
	bilateralFilter(img, bilateral, 15, 80, 80);
	imshow("bilateral", bilateral);
	waitKey();



	//// Task 4: Image Subtraction /////////////////////////////////////
	// Step 1: load two image
	std::string inputImagePathA = projectSrcDir + "/Data/imgA.png";
	std::string inputImagePathB = projectSrcDir + "/Data/imgB.png";
	Mat imga = imread(inputImagePathA, CV_LOAD_IMAGE_COLOR);
	Mat imgb = imread(inputImagePathB, CV_LOAD_IMAGE_COLOR);
	// Step 2: Subtract images and save the result
	Mat imgSubtracted = Vec3b(255, 255, 255) - abs(imgb - imga);
	imshow("subtracted",imgSubtracted );
	imwrite("imgSubtracted.png",imgSubtracted);
	waitKey();

	return 0;
}

