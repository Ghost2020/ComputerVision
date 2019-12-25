/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				FaceCompator.h
* \@brief Desc:				Face Identity Recognition
* \@brief prerequisite::	VS2013¡ü
*/
#pragma once

#include "GIVisionDetect.h"

#ifndef FACECOMPATOR_API
#define FACECOMPATOR_API
#endif

#ifndef GHOST_SIGNAL
#define GHOST_SIGNAL
#endif

namespace Ghost
{
	/**
	* \@brief Face and Identity Card Verification
	*/
	class FACECOMPATOR_API FaceCompator final : public IVisionDetecter
	{
	public:
		FaceCompator();
		virtual ~FaceCompator() override;

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
		* \@brief ID Comparasion
		* \@param frameIn:: face image
		* \@param frameOut:: ID Image
		*/
		virtual EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut) override;

		/**
		* \@brief Get the module type
		* \@return module type
		*/
		virtual EDetectModual getModualType() noexcept(true) override;

		/**
		* \@brief ÐÅºÅ´¥·¢
		*/
		public GHOST_SIGNAL:
		/**
		* \@brief
		*/
		void bindSlotFaceCompare(const std::function<void(const bool)>& func);

	private:
		class Impl;
		unique_ptr<Impl> m_pImpl;
	};
}///namespace Ghost
