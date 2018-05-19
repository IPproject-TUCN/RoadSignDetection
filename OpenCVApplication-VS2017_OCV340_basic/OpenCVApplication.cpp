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
int opening_size = 1;
int min_area_perc = 1;
int max_area_perc = 100;

Mat convertToBinaryColor(Mat src, int hue_start, int hue_offset, int min_saturation, int min_value, int max_value = 255)
{
	Mat hsvImg;
	cv::cvtColor(src, hsvImg, CV_BGR2HSV);

	Mat mask;
	cv::Scalar lower(hue_start, min_saturation, min_value);
	if (hue_start + hue_offset < 180)
	{
		cv::Scalar upper(hue_start + hue_offset, 255, max_value);
		cv::inRange(hsvImg, lower, upper, mask);
		return mask;
	}

	int remaining_hue = (hue_start + hue_offset) % 180;

	cv::Scalar  middle1(179, 255, max_value);
	cv::Scalar  middle2(0, min_saturation, min_value);
	cv::Scalar  upper(remaining_hue, 255, max_value);

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

std::vector<Rect> markRoadSigns(Mat binary_image, int min_area, int max_area)
{
	Mat labeled_image;
	Mat stats;
	Mat centroids;
	int labelCount = connectedComponentsWithStats(binary_image, labeled_image, stats, centroids);

	const int total_image_area = binary_image.cols * binary_image.rows;

	std::vector<Rect> road_sign_boxes;

	for(int labelIndex = 0; labelIndex < labelCount; labelIndex++)
	{
		int area = stats.at<int>(labelIndex, CC_STAT_AREA);
		if (area < min_area || max_area < area)
		{
			// if component is too small or too large don't draw it.
			continue;
		}

		int left = stats.at<int>(labelIndex, CC_STAT_LEFT);
		int top = stats.at<int>(labelIndex, CC_STAT_TOP);
		int width = stats.at<int>(labelIndex, CC_STAT_WIDTH);
		int height = stats.at<int>(labelIndex, CC_STAT_HEIGHT);
		Rect bounding_box = Rect(left, top, width, height);
		
		double ratio_h_w = (double)height / width;
		double ratio_w_h = (double)width / height;

		double const max_ratio = 5;

		if (ratio_h_w > max_ratio || ratio_w_h > max_ratio)
		{
			// if rectangle is too elonagated, don't draw it.
			continue;
		}

		road_sign_boxes.push_back(bounding_box);
	}

	return road_sign_boxes;
}

const int red_hue_start = 165;
const int yellow_hue_start = 20;
const int blue_hue_start = 100;

const int red_offset = 25;
const int yellow_offset = 20;
const int blue_offset = 35;

Mat adaptiveColorSegmentation(Mat image, Mat mask)
{
	Mat hsvImg;
	cv::cvtColor(image, hsvImg, CV_BGR2HSV);

	Vec3d means;
	Vec3d stds;
	meanStdDev(hsvImg, means, stds, mask);
	std::cout << "mean" << means << std::endl;
	std::cout << "stds" << stds << std::endl;

	int mean_sat = (int)means[1];
	int mean_value = (int)means[2];
	int std_sat = (int)stds[1];
	int std_value = (int)stds[2];

	// saturation should be higher
	int lower_sat = mean_sat - std_sat;
	int lower_value = mean_value - std_value;
	int upper_value = mean_value + std_value;

	lower_sat = max(lower_sat, 100);
	lower_value = max(lower_value, 15);

	CV_DbgAssert(0 <= lower_sat && lower_sat <= 255);
	CV_DbgAssert(0 <= lower_value && lower_value <= 255);
	CV_DbgAssert(0 <= upper_value && upper_value <= 255);

	Vec3b lower = Vec3b(0, lower_sat, lower_value);
	Vec3b upper = Vec3b(179, 255, upper_value);

	Mat adaptiveMask;
	inRange(hsvImg, lower, upper, adaptiveMask);

	Mat finalMask = adaptiveMask & mask;

	return finalMask;
}

Mat autoDetectRoadSignCore(Mat src)
{
	// where the detected road signs will appear.
	int hue_starts[] = {
		red_hue_start,
		yellow_hue_start,
		blue_hue_start
	};

	int hue_offsets[] = {
		red_offset,
		yellow_offset,
		blue_offset
	};

	Vec3b colors[] = {
		Vec3b(0, 0, 255),
		Vec3b(0, 255, 255),
		Vec3b(255, 0, 0)
	};

	Mat dst = src.clone();

	for(int i = 0; i < 3; i++)
	{
		int start_hue = hue_starts[i];
		int offset_hue = hue_offsets[i];

		Mat mask = convertToBinaryColor(src, start_hue, offset_hue, 100, 25, 250);
		Mat mask_adapted = adaptiveColorSegmentation(src, mask);

		int total_area = src.rows * src.cols;
		int min_area = (total_area * min_area_perc) / 1000;
		int max_area = (total_area * max_area_perc) / 1000;

		std::vector<Rect> signs = markRoadSigns(mask_adapted, min_area, max_area);

		for(Rect bounding_box : signs)
		{
			rectangle(dst, bounding_box, colors[i]);
		}
	}
	
	return dst;
}

void autoDetectRoadSign()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname);

		Mat with_road_signs = autoDetectRoadSignCore(src);

		imshow("roadsigns", with_road_signs);
		waitKey();
	}
}

void testDetectRoadSign()
{
	char folder_name[MAX_PATH];
	openFolderDlg(folder_name);
	FileGetter getter = FileGetter(folder_name, "jpg");

	char fname[MAX_PATH];
	while (getter.getNextAbsFile(fname))
	{
		Mat src = imread(fname);
		Mat result = autoDetectRoadSignCore(src);

		imshow("result", result);
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
		printf(" 5 - Auto road sign detection\n");
		printf(" 6 - Test sign detection\n");
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
			case 5:
				autoDetectRoadSign();
				break;
			case 6:
				testDetectRoadSign();
				break;
				
		}
	}
	while (op!=0);
	return 0;
}
