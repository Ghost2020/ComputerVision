#include "ObjectDetection.h"

#include <atomic>
#include <exception>
#include <filesystem>
#include <mutex>

#include "yolo_v2_class.hpp"

using namespace std;
namespace fs = std::filesystem;
using namespace Ghost::signalslot;


namespace Ghost
{
	/**
	* \@brief 私有类实现用来实现 Object Detection 模块的包装
	*/
	class ObjectDetector::Impl
	{
		/**
		* \@brief 构造函数和析构函数
		*/
	public:
		Impl()
			: m_pDetector(nullptr)
		{}

		~Impl()
		{
			antiModual();
		}
		/**
		* \@brief 公开方法
		*/
	public:
		EResult initModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_pDetector != nullptr)
				return EResult::SR_Detector_Already_Exist;

			if (!ObjectDetector::Impl::s_pathFlag.load())
				return EResult::SR_Data_Path_Not_Set;

			m_pDetector = std::make_unique<Detector>(s_Paths.cfgPath, s_Paths.weightPath);
			if (m_pDetector == nullptr)
			{
				m_States.initFlag.store(false);
				return EResult::SR_Detector_Memory_Allocation_Failed;
			}

			auto funcGetObjectsNamefromFile = [](const std::string& filename) ->std::vector<std::string>
			{
				std::ifstream file(filename);
				std::vector<std::string> file_lines;
				if (!file.is_open()) return file_lines;
				for (std::string line; getline(file, line);) file_lines.push_back(line);
				std::cout << "object names loaded \n";
				return file_lines;
			};

			m_vecObjName = funcGetObjectsNamefromFile(s_Paths.dataPath);

			m_States.initFlag.store(true);

			return EResult::SR_OK;
		}

		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_pDetector == nullptr)
				return EResult::SR_Detector_Not_Exist;

			m_pDetector.reset();
			m_pDetector = nullptr;

			m_States.initFlag.store(false);

			return EResult::SR_OK;
		}

		EResult detect(const cv::Mat& frameIn, cv::Mat& frameShow)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_States.initFlag.load())
				return EResult::SR_Detector_Not_Exist;

			if (frameIn.empty())
				return EResult::SR_Image_Empty;

			m_resultBoxs = m_pDetector->detect(frameIn);

			if (!frameShow.empty())
			{
				//绘制检测到的目标
				drawObject(frameShow, m_resultBoxs, m_vecObjName);

				//如果是人 发出检测到的信号
				m_SIGNAL_void_Objects(m_vecObjName);
			}

			return EResult::SR_OK;
		}

		void drawObject(cv::Mat& mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names, int current_det_fps = -1, int current_cap_fps = -1)
		{
			int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };

			for (auto& i : result_vec)
			{
				cv::Scalar color = obj_id_to_color(i.obj_id);
				cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 2);
				if (obj_names.size() > i.obj_id)
				{
					std::string obj_name = obj_names[i.obj_id];
					if (i.track_id > 0) obj_name += " - " + std::to_string(i.track_id);
					cv::Size const text_size = getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, 0);
					int max_width = (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
					max_width = std::max(max_width, (int)i.w + 2);
					//max_width = std::max(max_width, 283);
					std::string coords_3d;
					if (!std::isnan(i.z_3d))
					{
						std::stringstream ss;
						ss << std::fixed << std::setprecision(2) << "x:" << i.x_3d << "m y:" << i.y_3d << "m z:" << i.z_3d << "m ";
						coords_3d = ss.str();
						cv::Size const text_size_3d = getTextSize(ss.str(), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, 1, 0);
						int const max_width_3d = (text_size_3d.width > i.w + 2) ? text_size_3d.width : (i.w + 2);
						if (max_width_3d > max_width) max_width = max_width_3d;
					}

					cv::rectangle
					(
						mat_img,
						cv::Point2f(std::max((int)i.x - 1, 0), std::max((int)i.y - 35, 0)),
						cv::Point2f(std::min((int)i.x + max_width, mat_img.cols - 1), std::min((int)i.y, mat_img.rows - 1)),
						color,
						CV_FILLED,
						8,
						0
					);
					putText
					(
						mat_img,
						obj_name,
						cv::Point2f(i.x, i.y - 16),
						cv::FONT_HERSHEY_COMPLEX_SMALL,
						1.2,
						cv::Scalar(0, 0, 0),
						2
					);
					if (!coords_3d.empty()) putText(mat_img, coords_3d, cv::Point2f(i.x, i.y - 1), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0, 0, 0), 1);
				}
			}
			if (current_det_fps >= 0 && current_cap_fps >= 0)
			{
				std::string fps_str = "FPS detection: " + std::to_string(current_det_fps) + "   FPS capture: " + std::to_string(current_cap_fps);
				putText(mat_img, fps_str, cv::Point2f(10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(50, 255, 0), 2);
			}
		}

		/**
		* \@brief 成员变量
		*/
	public:
		struct SDataPath
		{
			string dataPath;					//Data Path
			string cfgPath;						//CFG Path
			string weightPath;					//Weight Path

			SDataPath()
				:
				dataPath(""), cfgPath(""), weightPath("")
			{}
		};

		const static string s_version;
		static SDataPath s_Paths;
		static std::atomic<bool> s_pathFlag;		//依赖的数据路径是否被设置

		struct SState
		{
			std::atomic<bool> initFlag;			//初始化标志
			std::atomic<bool> filterFlag;		//kalman滤波标志

			SState()
				:
				initFlag(false), filterFlag(false)
			{}
		};
		SState m_States;

		//检测对象
		std::unique_ptr<Detector> m_pDetector;

		//对象名称
		std::vector<std::string> m_vecObjName;

		//检测结果
		std::vector<bbox_t> m_resultBoxs;

		//信号槽
		Ghost::signalslot::Signal<void(const std::vector<std::string>&)> m_SIGNAL_void_Objects;
		Ghost::signalslot::Slot m_SLOT_void_Objects;

		//锁
		std::mutex m_mutex;

		//kalman 滤波器
		track_kalman_t m_filter;
	};

	ObjectDetector::Impl::SDataPath ObjectDetector::Impl::s_Paths;
	std::atomic<bool> ObjectDetector::Impl::s_pathFlag = false;

#if( _MSC_TOOLSET_VER_ == 140 )
#ifdef NDEBUG
	const string ObjectDetector::Impl::s_version = "vc140-r.0";
#else
	const string ObjectDetector::Impl::s_version = "vc140-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 141)
#ifdef NDEBUG
	const string ObjectDetector::Impl::s_version = "vc141-r.0";
#else
	const string ObjectDetector::Impl::s_version = "vc141-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 142)
#ifdef NDEBUG
	const string ObjectDetector::Impl::s_version = "vc142-r.0";
#else
	const string ObjectDetector::Impl::s_version = "vc142-d.0";
#endif
#endif

	ObjectDetector::ObjectDetector()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("ObjectDetector::ObjectDetector::make_unique<Imple>::Failured!!!");
		}
	}

	ObjectDetector::~ObjectDetector()
	{
		m_pImpl.reset();
	}

	EResult ObjectDetector::setPath(const string& dataPath, const string& cfgPath, const string& weightPath) noexcept(true)
	{
		if (!fs::exists(dataPath))
		{
			ObjectDetector::Impl::s_pathFlag.store(false);
			return EResult::SR_Data_File_Not_Exist;
		}

		if (!fs::exists(cfgPath))
		{
			ObjectDetector::Impl::s_pathFlag.store(false);
			return EResult::SR_Cfg_File_Not_Exist;
		}

		if (!fs::exists(weightPath))
		{
			ObjectDetector::Impl::s_pathFlag.store(false);
			return EResult::SR_Weight_File_Not_Exist;
		}

		ObjectDetector::Impl::s_Paths.dataPath = dataPath;
		ObjectDetector::Impl::s_Paths.cfgPath = cfgPath;
		ObjectDetector::Impl::s_Paths.weightPath = weightPath;

		ObjectDetector::Impl::s_pathFlag.store(true);

		return EResult::SR_OK;
	}

	const string& ObjectDetector::getVersion() noexcept(true)
	{
		return ObjectDetector::Impl::s_version;
	}

	EResult ObjectDetector::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult ObjectDetector::saveModelParam()
	{
		return EResult::SR_OK;
	}

	EResult ObjectDetector::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult ObjectDetector::antiModual()
	{
		this->saveModelParam();

		return m_pImpl->antiModual();
	}

	EResult ObjectDetector::setModualParam(const EModualParamType type, const float value)
	{
		switch (type)
		{
		case ::EModualParamType::TYPE_XXX:
			break;
		case EModualParamType::TYPE_UNDEFINE:
			break;
		default:
			break;
		}

		return EResult::SR_OK;
	}

	EResult ObjectDetector::detect(const cv::Mat& frameIn, cv::Mat& frameShow)
	{
		if (frameIn.empty()) return EResult::SR_Image_Empty;

		return m_pImpl->detect(frameIn, frameShow);
	}

	EDetectModual ObjectDetector::getModualType() noexcept(true)
	{
		return EDetectModual::Object_Detection_Modual;
	};

	void ObjectDetector::bindSlotObjectFind(const std::function<void(const std::vector<std::string>&)>& func)
	{
		m_pImpl->m_SLOT_void_Objects = m_pImpl->m_SIGNAL_void_Objects.connect(func);
	}
}///namespace Ghost
