#pragma once

#include "GIVisionDetect.h"

#ifndef FACERECOGNITION_API
#define FACERECOGNITION_API
#endif

#ifndef GHOST_SIGNAL 
#define GHOST_SIGNAL 
#endif

using namespace Ghost;
using namespace std;
using namespace cv;

namespace Ghost
{
	/**
	* \@brief Emotion Detector for complex task
	*/
	class FACERECOGNITION_API EmotionDetector final : public IVisionDetecter
	{
	public:
		EmotionDetector();
		virtual ~EmotionDetector() override;

	public:
		/**
		* \@brief Setting the path of the data file required by the module #####Chinese cannot be included in the path#####
		* \@param faceModelPath:: face detection cascade Path
		* \@param emotionXmlPath:: emotion detection cascadePath
		* \@return Returns the result of execution
		*/
		static EResult setPath(const string& faceModelPath, const string& emotionXmlPath, const string& flagPath) noexcept(true);

		/**
		* \@brief Get the version number of the current library
		* \@return Returns the result of execution
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
		* \@consume time 100ms
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

	public GHOST_SIGNAL:
		/**
		* \@brief Found face Landmark appearing
		* \@param func::Functions that need to be triggered
		*/
		void bindSlotEmotionChanged(const std::function<void(const std::vector<Ghost::EEmotion>&)>& functor);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	
	};
}///namespace Ghost
