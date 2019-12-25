/**
* \@brief Author			Ghost Chen
* \@brief Email				cxx2020@outlook.com
* \@brief Date				2019/07/27
* \@brief File				FgIVisionDetect.h
* \@brief Desc:				Face Identity Recognition
* \@brief prerequisite::	VS2013↑
*/
#pragma once

#include "GUtilities.hpp"
#include "opencv2/opencv.hpp"

using namespace Ghost;

class IVisionDetecter
{
public:
	virtual ~IVisionDetecter() = 0 {};

	IVisionDetecter(const IVisionDetecter&) = delete;
	IVisionDetecter& operator=(const IVisionDetecter&) = delete;

protected:
	IVisionDetecter() = default;

public:
	/**
	* \@brief Loading modual parameters
	* \@param modualLoadPath::
	* \@return Results of implementation
	*/
	virtual EResult loadModualParam(const string& modualLoadPath) = 0;

	/**
	* \@brief Save the modual parameters to local
	* \@return Results of implementation
	*/
	virtual EResult saveModelParam() = 0;

	/**
	* \@brief Initialization of Detection Environment
	* \@return Returns the result of execution
	*/
	virtual EResult initModual() = 0;

	/**
	* \@brief Anti-initialization of Detection Environment
	* \@return Returns the result of execution
	*/
	virtual EResult antiModual() = 0;

	/**
	* \@brief Setting Module Parameters
	* \@param value
	* \@return Results of implementation
	*/
	virtual EResult setModualParam(const EModualParamType type, const float value) = 0;

	/**
	* \@brief Detect the Object  #warning if frameShow is tempty Explains that the detected object does not need to be drawn#
	* \@param frameIn::Image for detection
	* \@param frameShow::Need to draw detected objects #may be emtpy!!!#
	*/
	virtual EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut) = 0;

	/**
	* \@brief Get the module type
	* \@return module type
	*/
	virtual EDetectModual getModualType() noexcept(true) = 0;
};