/*
* GLog.h				- A log for the Trace
* Author				- Ghost Chen
* Date					- 2019/04/24
* Copyright (c) FaceGood. All rights reserved.
*/
#pragma once

//写日志用
#include <fstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>

using namespace std;

namespace Ghost
{
	class GLog final
	{
	public:
		//信息等级
		enum ELogLevel
		{
			//致命错误，应该马上报错，并退出程序
			LOG_LEVEL_FATAL = 0,
			//轻微错误，通过异常捕捉，有可能恢复正常
			LOG_LEVEL_ERROR,
			//警告信息,
			LOG_LEVEL_WARN,
			//普通信息
			LOG_LEVEL_INFO,
			//Debug信息
			LOG_LEVEL_DEBUG,
			//执行路径追踪
			LOG_LEVEL_TRACE
		};

	public:
		explicit GLog() noexcept(true);
		~GLog() noexcept(true);

		GLog(GLog const&) = delete;
		GLog& operator= (GLog const&) = delete;
		GLog(GLog&&) = delete;
		GLog& operator=(GLog&&) = delete;

	public:
		/**
		* \@brief 设置log保存的路径
		*/
		void setLogSavePath(const wstring& savePath);

		/**
		* \@brief 写入信息时调用
		* \@param infor::写入的信息
		* \@param logLevel::日志等级
		* \@param line::日志是哪一行
		* \@param function::日志在哪一个函数
		* \@param file::日志在哪一个文件
		*/
		void writeLog(const string& infor, const ELogLevel logLevel = LOG_LEVEL_INFO, const std::string& file = "", const std::string & function = "", const int line = -1);

	private:
		/**
		* \@brief 打开文件,以追加的形式打开,文件不存在时，会打开失败
		*/
		void openFile();

		/**
		* \@brief 关闭日志文件
		*/
		void closeFile();

		/**
		* \@brief 获取时间戳
		* \@return 返回时间戳
		*/
		std::time_t getTimeStamp();

		/**
		* \@brief 时间戳转日期
		* \@param timestamp::时间戳
		* \@return 时间信息
		*/
		std::tm* getTM(const std::time_t& timestamp);

	private:
		//数据流处理对象
		std::ofstream m_file;
		//文件名
		std::wstring m_fileName;
		//文件路径
		std::wstring m_filePath;
	};

}///namespace FaceGood