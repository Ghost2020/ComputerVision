#include "GLog.h"

#include <filesystem>

namespace fs = std::experimental::filesystem;

namespace Ghost
{
	GLog::GLog() noexcept(true)
	{
		//设置为中文路径
		std::locale::global(std::locale(""));
	}

	GLog::~GLog() noexcept(true)
	{
		closeFile();
	}

	void GLog::openFile()
	{
		if (!m_file.is_open())
		{
			m_file.open(m_fileName, ios::app);
			m_file.imbue(std::locale("chs"));
		}

		fs::permissions(m_fileName, fs::perms::owner_all);
	}

	std::time_t GLog::getTimeStamp()
	{
		std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
		std::time_t timestamp = tmp.count();
		return timestamp;
	}

	std::tm* GLog::getTM(const std::time_t& timestamp)
	{
		//此处转化为东八区北京时间，如果是其它时区需要按需求修改
		time_t milli = timestamp + time_t(8 * 60 * 60 * 1000);
		auto mTime = std::chrono::milliseconds(milli);
		auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
		auto tt = std::chrono::system_clock::to_time_t(tp);
		std::tm* now = std::gmtime(&tt);
		return now;
	}

	void GLog::setLogSavePath(const wstring& savePath)
	{
		if (!fs::exists(savePath))
		{
			throw std::exception("Path not exist!!!");
		}

		m_fileName = savePath + wstring(L"\\FaceGood-UE4-ComputerVision.log");
		m_file.open(m_fileName, ios::out);
		fs::path path = fs::canonical(m_fileName);
		m_fileName = path.wstring();
	}

	void GLog::writeLog(const string& infor, const ELogLevel logLevel, const std::string& file, const std::string& function, const int line)
	{
		openFile();

		if (m_file.is_open())
		{
			//输入时间
			std::tm* now = this->getTM(this->getTimeStamp());
			m_file << setw(4) << now->tm_year + 1900 << "Y-"
				   << setw(2) << now->tm_mon + 1	 << "M-"
				   << setw(2) << now->tm_mday		 << "D "
				   << setw(2) << now->tm_hour		 << ":"
				   << setw(2) << now->tm_min		 << ":"
				   << setw(2) << now->tm_sec		 << " "
				   << setw(2) << file				 << ":"
				   << setw(2) << function			 << ":"
				   << setw(2) << line				 << ":";

			//写入日志信息
			switch (logLevel)
			{
			case LOG_LEVEL_FATAL:
				m_file << setw(7) << L"<FATAL> " << setw(8) << L"MESSAGE::" << infor << endl;
				break;
			case LOG_LEVEL_ERROR:
				m_file << setw(7) << L"<ERROR> " << setw(8) << L"MESSAGE::" << infor << endl;
				break;
			case LOG_LEVEL_WARN:
				m_file << setw(7) << L"<WARN> " << setw(8) << L"MESSAGE::" << infor << endl;
				break;
			case LOG_LEVEL_INFO:
				m_file << setw(7) << "<INFO> " << setw(8) << "MESSAGE::" << infor << endl;
				break;
			case LOG_LEVEL_DEBUG:
				m_file << setw(7) << "<DEBUG> " << setw(8) << "MESSAGE::" << infor << endl;
				break;
			case LOG_LEVEL_TRACE:
				m_file << setw(7) << "<TRACE> " << setw(8) << "MESSAGE::" << infor << endl;
				break;
			}
		}
		else
		{
			//临时存储的位置
			fs::path logPath = L"D:\\Log";
			if (!fs::exists(logPath))
			{
				fs::create_directory(logPath);
				setLogSavePath(logPath);
			}
			return;
		}

		closeFile();
	}

	void GLog::closeFile()
	{
		if (m_file.is_open())
		{
			m_file.close();
		}
	}

} ///namespace FaceGood