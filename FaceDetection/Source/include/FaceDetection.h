/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				FaceDetection.h
* \@brief Desc:				Face Identity Recognition
* \@brief prerequisite::	VS2013¡ü
*/
#pragma once

#include "FgIVisionDetect.h"

using namespace std;
using namespace facegood;

#ifndef FACEDETECTOR_API
#define FACEDETECTOR_API
#endif

#ifndef GHOST_SIGNAL
#define GHOST_SIGNAL
#endif

namespace facegood
{
	/**
	* \@brief Face Location Detection
	*/
	class FACEDETECTOR_API FaceDetector final : public IVisionDetecter
	{
	public:
		FaceDetector();
		virtual ~FaceDetector() override;

	public:
		/**
		* \@brief Setting the path of the data file required by the module #####Chinese cannot be included in the path#####
		* \@param modelPath:: model of path
		* \@return Returns the result of execution
		*/
		static EResult setPath(const string& modelPath) noexcept(true);

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
	void bindSlotFaceFind(const std::function<void(const std::vector<facegood::SRect>&)>& func);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}///namespace facegood