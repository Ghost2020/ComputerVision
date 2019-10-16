/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				PoseDetection.h
* \@brief Desc:				Face Identity Recognition
* \@brief prerequisite::	VS2013¡ü
*/
#pragma once

#include "FgIVisionDetect.h"

#include <vector>
#include <string>

#ifndef POSEDETECTOR_API
#define POSEDETECTOR_API
#endif

#ifndef GHOST_SIGNAL
#define GHOST_SIGNAL
#endif

using namespace std;

namespace facegood
{
	/**
	* \@brief Pose detection in human body
	*/
	class POSEDETECTOR_API PoseDetector final : public IVisionDetecter
	{
	public:
		PoseDetector();
		virtual ~PoseDetector() override;

	public:
		/**
		* \@brief Setting the path of the data file required by the module #####Chinese cannot be included in the path#####
		* \@param modelsFolderPath:: Folders for storing models
		* \@param prototxtName:: Folders for protxtPath
		* \@param caffeModelName:: Folders for storing caffe models
		* \@return Returns the result of execution
		*/
		static EResult setPath(const string& modelsFolderPath, const string& prototxtName, const string& caffeModelName) noexcept(true);

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
		* \@consume time::
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
	void bindSlotPoseFind(const std::function<void(const std::vector<facegood::SPoint2D>&)>& func);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}///namespace facegood