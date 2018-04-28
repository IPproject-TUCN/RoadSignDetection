// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <unordered_set>

/*
 * Sets the labels to zero (set them as background) if the label is contained in labels_to_remove
 */
void removeLabels(Mat labeled_image, std::unordered_set<int> labels_to_remove)
{
	for(int rowIndex = 0; rowIndex < labeled_image.rows; rowIndex++)
	{
		for(int colIndex = 0; colIndex < labeled_image.cols; colIndex++)
		{
			int label = labeled_image.at<int>(rowIndex, colIndex);
			const bool remove_label = labels_to_remove.find(label) != labels_to_remove.end();
			if (remove_label)
			{
				labeled_image.at<int>(rowIndex, colIndex) = 0;
			}
		}
	}
}

Mat filterSmallAndLargeObjects(Mat binary_image, int min_area, int max_area)
{
	Mat labeled_image;
	Mat stats;
	Mat centroids;
	int labelCount = connectedComponentsWithStats(binary_image, labeled_image, stats, centroids);

	const int total_image_area = binary_image.cols * binary_image.rows;
	std::unordered_set<int> labels_to_remove;

	for(int labelIndex = 0; labelIndex < labelCount; labelIndex++)
	{
		int area = stats.at<int>(labelIndex, CC_STAT_AREA);
		if (area < min_area || max_area < area)
		{
			labels_to_remove.insert(labelIndex);
		}
	}

	removeLabels(labeled_image, labels_to_remove);

	Mat converted_labeled;
	labeled_image.convertTo(converted_labeled, CV_8UC1);
	return converted_labeled;
}

void convertToBinaryRed() 
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname);
		int height = src.rows;
		int width = src.cols;

		Mat maskR1, maskR2, maskR_eroded,maskR_opened;

		//HSV image
		Mat hsvImg;
		cv::cvtColor(src, hsvImg, CV_BGR2HSV);

		//define lower and upper bounds for meaningful colors in hsv

		cv::Scalar  lower_red(160, 100, 100);
		cv::Scalar  middle1_red(179, 255, 255);
		cv::Scalar  middle2_red(0, 100, 100);
		cv::Scalar  upper_red(10, 255, 255);

		//filter out less meaningful colors
		cv::inRange(hsvImg, lower_red, middle1_red, maskR1);
		cv::inRange(hsvImg, middle2_red, upper_red, maskR2);

		//opening
		const int dialtion_size = 2;
		const int element_size = 2 * dialtion_size + 1;

		Mat element = getStructuringElement(MORPH_RECT, Size(element_size, element_size));

		erode(maskR1|maskR2, maskR_eroded, element);
		dilate(maskR_eroded, maskR_opened, element);

		int total_area = src.rows * src.cols;
		int max_area = total_area / 10;
		int min_area = total_area / 200;
		Mat filtered = filterSmallAndLargeObjects(maskR_opened, min_area, max_area);
		
		Mat filtered_equalized;
		equalizeHist(filtered, filtered_equalized);

		Mat labeled;
		applyColorMap(filtered_equalized, labeled, COLORMAP_JET);

		imshow("input image", src);
		imshow("only red", maskR_opened);
		imshow("labeled", labeled);
		waitKey();
	}
	
}


void convertToBinaryYellow() {
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname);
		int height = src.rows;
		int width = src.cols;

		Mat maskY,maskY_eroded, maskY_opened;

		//HSV image
		Mat hsvImg;
		cv::cvtColor(src, hsvImg, CV_BGR2HSV);

		//define lower and upper bounds for meaningful colors in hsv
		cv::Scalar  lower_yellow(20, 100, 100);
		cv::Scalar  upper_yellow(40, 255, 255);


		//filter out less meaningful colors
		cv::inRange(hsvImg, lower_yellow, upper_yellow, maskY);

		//opening
		const int dialtion_size = 2;
		const int element_size = 2 * dialtion_size + 1;

		Mat element = getStructuringElement(MORPH_CROSS, Size(element_size, element_size));
		/*
		erode(maskY, maskY_eroded, element);
		dilate(maskY_eroded, maskY_opened, element);
		*/

		imshow("input image", src);
		imshow("only yellow", maskY);


	}
}

void convertToBinaryBlue() {
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname);
		int height = src.rows;
		int width = src.cols;

		Mat maskB, maskB_eroded,maskB_opened;

		//HSV image
		Mat hsvImg;
		cv::cvtColor(src, hsvImg, CV_BGR2HSV);

		//define lower and upper bounds for meaningful colors in hsv
		cv::Scalar lower_blue(100, 100, 100);
		cv::Scalar upper_blue(140, 255, 255);


		//filter out less meaningful colors
		cv::inRange(hsvImg, lower_blue, upper_blue, maskB);


		//opening
		const int dialtion_size = 2;
		const int element_size = 2 * dialtion_size + 1;

		Mat element = getStructuringElement(MORPH_CROSS, Size(element_size, element_size));
		/*
		erode(maskB, maskB_eroded, element);
		dilate(maskB_eroded, maskB_opened, element);
		*/
		imshow("input image", src);
		imshow("only blue", maskB);
	}
}

int hue_start = 160;
int hue_offset = 40;
int min_value = 100;
int min_saturation = 100;
int opening_size = 2;
int min_area_perc = 5;
int max_area_perc = 100;

Mat convertToBinaryColor(Mat src, int hue_start, int hue_offset, int min_saturation, int min_value)
{
	
	Mat hsvImg;
	cv::cvtColor(src, hsvImg, CV_BGR2HSV);

	//define lower and upper bounds for meaningful colors in hsv

	Mat mask;
	cv::Scalar lower(hue_start, min_saturation, min_value);
	if (hue_start + hue_offset < 180)
	{
		cv::Scalar upper(hue_start + hue_offset, 255, 255);
		cv::inRange(hsvImg, lower, upper, mask);
		return mask;
	}

	int remaining_hue = (hue_start + hue_offset) % 180;

	cv::Scalar  middle1(179, 255, 255);
	cv::Scalar  middle2(0, min_saturation, min_value);
	cv::Scalar  upper(remaining_hue, 255, 255);

	Mat mask_extended;

	//filter out less meaningful colors
	cv::inRange(hsvImg, lower, middle1, mask);
	cv::inRange(hsvImg, middle2, upper, mask_extended);
	return mask | mask_extended;
}

void detectRoadSignCallback(int event, int x, int y, int flags, void* data)
{
	if (event != CV_EVENT_LBUTTONDOWN)
	{
		return;
	}
	Mat src = *(Mat*)data;

	//HSV image
	Mat mask = convertToBinaryColor(src, hue_start, hue_offset, min_saturation, min_value);

	//opening
	const int dialtion_size = opening_size;
	const int element_size = 2 * dialtion_size + 1;

	Mat element = getStructuringElement(MORPH_RECT, Size(element_size, element_size));

	Mat mask_eroded,mask_opened;
	erode(mask, mask_eroded, element);
	dilate(mask_eroded, mask_opened, element);

	int total_area = src.rows * src.cols;
	int min_area = (total_area * min_area_perc) / 1000;
	int max_area = (total_area * max_area_perc) / 1000;
	Mat filtered = filterSmallAndLargeObjects(mask_opened, min_area, max_area);
	
	Mat filtered_equalized;
	equalizeHist(filtered, filtered_equalized);

	Mat labeled;
	applyColorMap(filtered_equalized, labeled, COLORMAP_JET);

	imshow("input image", src);
	imshow("only red", mask_opened);
	imshow("labeled", labeled);
}

void guiDetectRoadSign()
{
	
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname);
		const std::string trackbar_window_name = "trackbars";
		namedWindow(trackbar_window_name);
		createTrackbar("hue start", trackbar_window_name, &hue_start, 179);
		createTrackbar("hue offset", trackbar_window_name, &hue_offset, 179);
		createTrackbar("min value", trackbar_window_name, &min_value, 255);
		createTrackbar("min staturation", trackbar_window_name, &min_saturation, 255);
		createTrackbar("opening size", trackbar_window_name, &opening_size, 20);
		createTrackbar("min area percentage", trackbar_window_name, &min_area_perc, 100);
		createTrackbar("max area percentage", trackbar_window_name, &max_area_perc, 500);

		namedWindow("buttonwindow");
		setMouseCallback("buttonwindow", detectRoadSignCallback, &src);
		Mat button = Mat(100, 100, CV_8UC1);
		imshow("buttonwindow", button);

		detectRoadSignCallback(0, 0, 0, 0, &src);
		waitKey();
	}
}

int main()
{
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Convert image to binary based on red\n");
		printf(" 2 - Convert image to binary based on blue\n");
		printf(" 3 - Convert image to binary based on yellow\n");
		printf(" 4 - GUI road sign detection\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				convertToBinaryRed();
				break;
			case 2:
				convertToBinaryBlue();
				break;
			case 3:
				convertToBinaryYellow();
				break;
			case 4:
				guiDetectRoadSign();
				break;

				
		}
	}
	while (op!=0);
	return 0;
}