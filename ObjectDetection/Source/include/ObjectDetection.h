/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				ObjectDetection.h
* \@brief Desc:				Object Detection modual
* \@brief prerequisite::	1.Win10-Intel-CPU-i58400 2.NVIDIA-1060¡ü 3.CUDA-v10.1¡ü 4.cudnn v7.0¡ü
*/
#pragma once

#include <string>
#include <memory>
#include <vector>

#include "FgIVisionDetect.h"

using namespace std;
using namespace cv;
using namespace facegood;

#ifndef OBJECTDETECTION_API
#define OBJECTDETECTION_API
#endif

#ifndef GHOST_SIGNAL
#define GHOST_SIGNAL
#endif

namespace facegood
{
	/**
	* \@brief Object Detection Module
	*/
	class OBJECTDETECTION_API ObjectDetector final : public IVisionDetecter
	{
	public:
		ObjectDetector();
		virtual ~ObjectDetector() override;

	public:
		/**
		* \@brief Setting the path of the data file required by the module #####Chinese cannot be included in the path#####
		* \@param dataPath:: data file Path
		* \@param cfgPath:: cfg file Path
		* \@param weightPath:: weight File Path
		* \@return Returns the result of execution
		*/
		static EResult setPath(const string& dataPath, const string& cfgPath, const string& weightPath) noexcept(true);

		/**
		* \@brief Get the version number of the current library
		*/
		static const string& getVersion() noexcept(true);

		/**
		* \@brief Loading modual parameters
		* \@param modualLoadPath::
		* \@return Results of implementation
		*/
		virtual EResult loadModualParam(const string& modualLoadPath) override;

		/**
		* \@brief Save the modual parameters to local
		* \@return Results of implementation
		*/
		virtual EResult saveModelParam() override;

		/**
		* \@brief Initialization of Detection Environment
		* \@return Returns the result of execution
		*/
		virtual EResult initModual() override;

		/**
		* \@brief Anti-initialization of Detection Environment
		* \@return Returns the result of execution
		*/
		virtual EResult antiModual() override;

		/**
		* \@brief Setting Module Parameters
		* \@param value
		* \@return Results of implementation
		*/
		virtual EResult setModualParam(const EModualParamType type, const float value) override;

		/**
		* \@brief Detect the Object  #warning if frameShow is tempty Explains that the detected object does not need to be drawn#
		* \@param frameIn::Image for detection
		* \@param frameShow::Need to draw detected objects #may be emtpy!!!#
		*/
		virtual EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut) override;

		/**
		* \@brief Get the module type
		* \@return module type
		*/
		virtual EDetectModual getModualType() noexcept(true) override;

	public GHOST_SIGNAL:
	/**
	* \@brief
	*/
	void bindSlotObjectFind(const std::function<void(const std::vector<std::string>&)>& func);

	private:
		class Impl;
		unique_ptr<Impl> m_pImpl;
	};
}///namespace facegood