// TestLib.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <memory>
#include <string>

#include <filesystem>
#include <functional>
#include <thread>
#include <opencv2/opencv.hpp>

#include "GUtilities.hpp"

#include <Windows.h>

#define FACE_RECOGNITION 0
#define FACE_COMPARE 0
#define FACE_DETECTION 0 
#define OBJECT_DETECTION 0
#define POSE_DETECTION 0
#define FACE_LANDMARK 0
#define EMOTION_DETECTION 1

#if(FACE_RECOGNITION == 1)
	#include "FaceRecognition.h"
#elif(FACE_LANDMARK == 1)
	#include "FaceLandmark.h"
#elif(EMOTION_DETECTION == 1)
	#include "EmotionDetection.h"
#elif(FACE_DETECTION == 1)
#include "FaceDetection.h"
#endif

using namespace std;
using namespace Ghost;
using namespace cv;

// 使用互斥体保证单体运行
BOOL IsAlreadyRun()
{
	HANDLE hMutex = NULL;
	hMutex = CreateMutex(NULL, FALSE, L"EmotionDetection");
	if (hMutex != NULL)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			ReleaseMutex(hMutex);
			return TRUE;
		}
	}
	return FALSE;
}

int main(int argc, char* argv[])
{
	if (IsAlreadyRun())
		return -1;

	EResult result = EResult::SR_OK;

#if( FACE_RECOGNITION == 1)
	FaceRecognition detector;
#elif (FACE_COMPARE == 1)
	FaceCompator detector;
	result = FaceCompator::setPath();
#elif (FACE_DETECTION == 1)
	FaceDetector detector;
	const string modelPath = "D:\\Project\\UE4\\PluginDevelop_4_21\\FaceGoodLiveLink\\Plugins\\FaceGoodRuntime\\Content\\ComputerVision-Res\\FaceDetection\\Dependents\\model\\seeta_fd_frontal_v1.0.bin";
	result = FaceDetector::setPath(modelPath);
#elif( OBJECT_DETECTION == 1)
	ObjectDetector detector;
	result = ObjectDetector::setPath();
#elif( POSE_DETECTION == 1)
	PoseDetector detector;
	result = PoseDetector::setPath();
#elif( FACE_LANDMARK == 1)
	FaceLandmark detector;
	const string modelPath = "F:\\Src\\C++\\VS\\VS2019\\ComputerVision\\x64\\Release\\roboman-landmark-model.bin";
	const string modelXmlPath = "F:\\Src\\C++\\VS\\VS2019\\ComputerVision\\x64\\Release\\haar_roboman_ff_alt2.xml";
	result = FaceLandmark::setPath(modelPath, modelXmlPath);
#elif( EMOTION_DETECTION == 1)
	EmotionDetector detector;
	const string path = argv[1];
	const string modelPath = path + "\\seeta_fd_frontal_v1.0.bin";
	const string modelXmlPath = path + "\\haarcascade_smile.xml";
	const string flagPath = path + "";
	result = EmotionDetector::setPath(modelPath, modelXmlPath, flagPath);
#endif

	if (result != EResult::SR_OK)
	{
		cout << "Failured to Set Path" << endl;
		system("pause");
		return -1;
	}

	result = detector.initModual();
	if (result != EResult::SR_OK)
	{
		cout << "Failured to init" << endl;
		system("pause");
		return -1;
	}

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout << "Failured to open camera" << endl;
		system("pause");
		return -1;
	}

	const cv::Scalar textColor(0, 128, 0);

	size_t index = 1;
	
	cv::Mat mat, show;
	while (cap.isOpened())
	{
		cap >> mat;
		if (mat.empty())
		{
			break;
		}
		show = mat;

		clock_t start = clock();
		detector.detect(mat, show);
		clock_t ends = clock();

		std::string time = std::to_string((ends - start)) + ":ms";

		cv::putText(show, cv::String(time), cv::Point(10, 10), FONT_HERSHEY_PLAIN, 0.8, textColor);

		cv::imshow("show", show);
		int key = cv::waitKey(1);
		if (key == 27)
		{
			break;
		}
	}

	cap.release();
	detector.antiModual();

}