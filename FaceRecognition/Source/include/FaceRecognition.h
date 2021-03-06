/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				FaceRecognition.h
* \@brief Desc:				Face Identity Recognition
* \@brief prerequisite::	VS2013��
*/
#pragma once

#include "GIVisionDetect.h"

#include <vector>

using namespace Ghost;
using namespace cv;
using namespace std;

#ifndef FACERECOGNITION_API
#define FACERECOGNITION_API
#endif

#ifndef GHOST_SIGNAL
#define GHOST_SIGNAL
#endif

namespace Ghost
{
	/**
	* \@brief Face Attribute Detection
	*/
	class FACERECOGNITION_API FaceRecognition final : public IVisionDetecter
	{
	public:
		FaceRecognition();
		virtual ~FaceRecognition() override;

	public:
		/**
		* \@brief Setting the path of the data file required by the module #####Chinese cannot be included in the path#####
		* \@param dataPath:: data file Path
		* \@param cfgPath:: cfg file Path
		* \@param weightPath:: weight File Path
		* \@return Returns the result of execution
		*/
		static EResult setPath(const string& databasePath, const string& cfgPath, const string& weightPath) noexcept(true);

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
		* \@param type Setting the type of parameter
		* \@param value 0.0::close---1.0::open
		* \@return Results of implementation
		*/
		virtual EResult setModualParam(const EModualParamType type, const float value) override;

		/**
		* \@brief Detect the Object  #warning if frameShow is tempty Explains that the detected object does not need to be drawn#
		* \@brief 50ms
		* \@param frameIn::Image for detection
		* \@param frameShow::Need to draw detected objects #may be emtpy!!!#
		* \@return Results of implementation
		*/
		virtual EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut) override;

		/**
		* \@brief Get the module type
		* \@return module type
		*/
		virtual EDetectModual getModualType() noexcept(true) override;

		/**
		* \@brief Add Face Data to Local Face Database
		* \@param frameSave
		* \@param infor
		* \@return Results of implementation
		*/
		EResult saveFaceToDataBase(const cv::Mat& frameSave, const SPersonInfor& infor);

	public GHOST_SIGNAL:
	/**
	* \@brief Found friend appearing
	* \@param func::Functions that need to be triggered
	*/
	void bindSlotFaceFind(const std::function<void(const std::vector<Ghost::SPersonInfor>&)>& func);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}///namepsace Ghost
