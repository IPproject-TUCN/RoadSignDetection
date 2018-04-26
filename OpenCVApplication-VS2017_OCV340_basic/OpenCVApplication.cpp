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

Mat filterSmallAndLargeObjects(Mat binary_image)
{
	Mat labeled_image;
	Mat stats;
	Mat centroids;
	int labelCount = connectedComponentsWithStats(binary_image, labeled_image, stats, centroids);

	// a road sign should not occupy more than 10 % of the image area
	const int total_image_area = binary_image.cols * binary_image.rows;
	const int max_area = total_image_area / 10;
	// a road sign should be at least 0.5% of pixels to be recognizable
	const int min_area = (total_image_area) / 200;
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

		Mat filtered = filterSmallAndLargeObjects(maskR_opened);
		
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

				
		}
	}
	while (op!=0);
	return 0;
}