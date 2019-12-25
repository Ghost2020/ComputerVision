/*
* GLog.h				- A log for the Trace
* Author				- Ghost Chen
* Date					- 2019/04/24
* Copyright (c) FaceGood. All rights reserved.
*/
#pragma once

//д��־��
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
		//��Ϣ�ȼ�
		enum ELogLevel
		{
			//��������Ӧ�����ϱ������˳�����
			LOG_LEVEL_FATAL = 0,
			//��΢����ͨ���쳣��׽���п��ָܻ�����
			LOG_LEVEL_ERROR,
			//������Ϣ,
			LOG_LEVEL_WARN,
			//��ͨ��Ϣ
			LOG_LEVEL_INFO,
			//Debug��Ϣ
			LOG_LEVEL_DEBUG,
			//ִ��·��׷��
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
		* \@brief ����log�����·��
		*/
		void setLogSavePath(const wstring& savePath);

		/**
		* \@brief д����Ϣʱ����
		* \@param infor::д�����Ϣ
		* \@param logLevel::��־�ȼ�
		* \@param line::��־����һ��
		* \@param function::��־����һ������
		* \@param file::��־����һ���ļ�
		*/
		void writeLog(const string& infor, const ELogLevel logLevel = LOG_LEVEL_INFO, const std::string& file = "", const std::string & function = "", const int line = -1);

	private:
		/**
		* \@brief ���ļ�,��׷�ӵ���ʽ��,�ļ�������ʱ�����ʧ��
		*/
		void openFile();

		/**
		* \@brief �ر���־�ļ�
		*/
		void closeFile();

		/**
		* \@brief ��ȡʱ���
		* \@return ����ʱ���
		*/
		std::time_t getTimeStamp();

		/**
		* \@brief ʱ���ת����
		* \@param timestamp::ʱ���
		* \@return ʱ����Ϣ
		*/
		std::tm* getTM(const std::time_t& timestamp);

	private:
		//�������������
		std::ofstream m_file;
		//�ļ���
		std::wstring m_fileName;
		//�ļ�·��
		std::wstring m_filePath;
	};

}///namespace FaceGood