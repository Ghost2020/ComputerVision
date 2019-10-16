/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				VisionManager.h
* \@brief Desc:				Object Detection and Human compare
* \@brief prerequisite::	1.Win10-Intel-CPU-i58400 2.NVIDIA-1060¡ü 3.CUDA-v10.1¡ü 4.cudnn v7.0¡ü 
* \@brief ThirdParty::		Opencv3.20
*/
#pragma once

#include <memory>
#include <string>
#include <functional>

#include "FgUtilities.hpp"

#ifndef VISIONMANAGER_API
#define VISIONMANAGER_API
#endif

#ifndef GHOST_SINAL
#define GHOST_SINAL
#endif

using namespace std;
using namespace facegood;

namespace facegood
{
	/**
	* \@brief Visual Inspection Management Object
	*/
	class VISIONMANAGER_API VisionManager final
	{
	public:
		explicit VisionManager();
		~VisionManager();

	private:
		VisionManager(const VisionManager&) = delete;
		VisionManager& operator=(const VisionManager&) = delete;
		VisionManager(const VisionManager&&) = delete;
		VisionManager& operator=(const VisionManager&&) = delete;

	public:
		/**
		* \@brief Open the webcamera
		* \@param cameraIndex webcamera index to open
		* \@return true::success opended false:: failured to open
		*/
		bool openCamera(const int cameraIndex);

		/**
		* \@brief Close the webcamera
		*/
		void closeCamera();

		/**
		* \@brief Setting Camera Parameters
		* \@param paramType::Types of parameters
		* \@param paramValue::Value of parameters
		* \@return true::successd to set | false:: failured to set
		*/
		bool setCameraParam(const ECameraParamType paramType, const double paramValue);

		/**
		* \@brief Getting Camera Parameters
		* \@param paramType::Types of parameters
		* \@return Value of parameters
		*/
		double getCameraParam(const ECameraParamType paramType);

		/**
		* \@brief
		* \@param modualType
		* \@param showFlag
		*/
		void setShowFlag(const EDetectModual modualType, const bool showFlag);

		/**
		* \@brief
		* \@param modualType Vision module type
		* \@param modualParamType
		* \@param paramValue
		* \@return Returns the result of execution
		*/
		EResult setModualParam(const EDetectModual modualType, const EModualParamType modualParamType, const float paramValue);

		/**
		* \@brief
		*/
		void tick();

		/**
		* \@brief Initialize the corresponding visual module
		* \@param modualType
		* \@return Returns the result of execution
		*/
		EResult initModule(const EDetectModual modualType);

		/**
		* \@brief Anti-Initialize the corresponding visual module
		* \@param modualType
		* \@return Returns the result of execution
		*/
		EResult antiModual(const EDetectModual modualType);

		/**
		* \@biref Loading camera parameters
		* \@param cameraParamPath
		* \@return Returns the result of execution
		*/
		EResult loadCameraParam(const wstring& cameraParamPath);

		/**
		* \@biref Saveing camera parameters
		* \@param cameraParamPath
		* \@return Returns the result of execution
		*/
		EResult saveCameraParam(const wstring& cameraParamPath);

		/**
		* \@biref Getting image width
		* \@return::image width
		*/
		const int& getImageWidth() const { return this->m_nImageWidth; };

		/**
		* \@biref Getting image height
		* \@return:: image height
		*/
		const int& getImageHeight() const { return this->m_nImageHeight; };

		/**
		* \@brief Getting image data
		* \@return::
		*/
		const unsigned char* getImageData() const { return this->data; };

	/**
	* \@brief Event-triggered use Binding slot function
	*/
	public GHOST_SINAL:
		/*
		* \@brief Objects discovered
		* \@param func
		*/
		void bindSlotObjectFind(const std::function<void(const std::vector<std::string>&)>& functor);

		/**
		* \@brief Found someone appearing
		* \@param func::functor that need to be triggered
		*/
		void bindSlotPersonFind(const std::function<void(void)>& functor);

		/**
		* \@brief Found friend appearing
		* \@param func::functor that need to be triggered
		*/
		void bindSlotFriendFind(const std::function<void(const std::vector<facegood::SPersonInfor>&)>& functor);

		/**
		* \@brief Camera status signal
		* \@param func::functor that need to be triggered
		*/
		void bindSlotCameraState(const std::function<void(void)>& functor);

		/**
		* \@brief Emotion Satate Changed
		* \@param func::functor that need to be triggered
		*/
		void bindSlotEmotionState(const std::function<void(const std::vector<facegood::EEmotion>&)>& functor);

	public:
		/**
		* \@brief
		* \@param resourcePath
		* \@return true:: filePath exist false::file path not exist!!!
		*/
		static EResult setResourcePath(const wstring& resourcePath);

		/**
		* \@brief Get the version of the current library
		* \@return version number
		*/
		static const string& getVersion() noexcept(true);

		/**
		* \@brief Get the version of the detection module
		* \@return Get the version of the detection module
		*/
		static const string& getDetectionModualVersion(const EDetectModual modualType) noexcept(true);

		/**
		* \@brief wstring to string
		* \@return string
		*/
		static string wstringTostring(const wstring& wstr);

		/**
		* \@brief string to wstring
		* \@return wstring
		*/
		static wstring stringTowstring(const string& str);

	private:
		//private Implement
		class Impl;
		std::unique_ptr<Impl> m_pImpl;

		//Image data
		unsigned char* data;
		//Image Width/Height
		int m_nImageWidth, m_nImageHeight;
	};
}///namespace facegood