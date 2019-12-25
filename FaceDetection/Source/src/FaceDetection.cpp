#include "FaceDetection.h"

#include <atomic>
#include <filesystem>
#include <mutex>

#include "face_detection.h"

namespace fs = std::experimental::filesystem;
using namespace std;

namespace Ghost
{
	class FaceDetector::Impl
	{
	public:
		Impl()
			:
			m_pDetector(nullptr),
			m_initFlag(false)
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

			if (!FaceDetector::Impl::s_pathFlag.load())
				return EResult::SR_Data_Path_Not_Set;

			m_pDetector = std::make_unique<seeta::FaceDetection>(s_modelPath.c_str());
			/*------------------------------需要读取最近保存的内容-----------------------------*/
			if (!loadParam(s_modelPath))
			{
				m_pDetector->SetMinFaceSize(60);
				m_pDetector->SetScoreThresh(2.f);
				m_pDetector->SetImagePyramidScaleFactor(0.8f);
				m_pDetector->SetWindowStep(4, 4);
			}
			else
			{
				;///
			}
			/*------------------------------需要读取最近保存的内容-----------------------------*/

			m_initFlag.store(true);

			return EResult::SR_OK;
		}

		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_pDetector == nullptr)
				return EResult::SR_Detector_Not_Exist;

			m_pDetector.reset();
			m_pDetector = nullptr;

			m_initFlag.store(false);

			return EResult::SR_OK;
		}

		EResult detect(const cv::Mat& frameIn, cv::Mat& frameShow)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_initFlag.load())
				return EResult::SR_Detector_Not_Exist;

			if (frameIn.empty())
				return EResult::SR_Image_Empty;

			/********************************************************/
			cv::Mat img_gray;
			cv::cvtColor(frameIn, img_gray, cv::COLOR_BGR2GRAY);

			seeta::ImageData img_data;
			img_data.data = img_gray.data;
			img_data.width = img_gray.cols;
			img_data.height = img_gray.rows;
			img_data.num_channels = 1;
			std::vector<seeta::FaceInfo> faces = m_pDetector->Detect(img_data);

			if (!frameShow.empty())
			{
				//绘制检测到的目标
				drawFace(frameShow, faces);

				//如果是人 发出检测到的信号
				if (faces.size() > 0)
				{
					m_faces.clear();
					for (const auto& face : faces)
						m_faces.push_back(Ghost::SRect(face.bbox.x, face.bbox.y, face.bbox.width, face.bbox.height));

					m_SIGNAL_void_rects(m_faces);
				}
			}

			return EResult::SR_OK;
		}

		void drawFace(cv::Mat& mat_img, std::vector<seeta::FaceInfo>& faces, int current_det_fps = -1, int current_cap_fps = -1)
		{
			cv::Rect face_rect;
			int32_t num_face = static_cast<int32_t>(faces.size());

			for (int32_t i = 0; i < num_face; i++)
			{
				face_rect.x = faces[i].bbox.x;
				face_rect.y = faces[i].bbox.y;
				face_rect.width = faces[i].bbox.width;
				face_rect.height = faces[i].bbox.height;

				cv::rectangle(mat_img, face_rect, CV_RGB(0, 0, 255), 4, 8, 0);
			}

			if (current_det_fps >= 0 && current_cap_fps >= 0)
			{
				std::string fps_str = "FPS detection: " + std::to_string(current_det_fps) + "   FPS capture: " + std::to_string(current_cap_fps);
				putText(mat_img, fps_str, cv::Point2f(10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(50, 255, 0), 2);
			}
		}

		bool loadParam(const string& paramPath)
		{
			if (!fs::exists(paramPath))
			{
				return false;
			}

			//加载数据(----------------------------------------------------------------------------------)

			return true;
		}

	public:
		std::unique_ptr<seeta::FaceDetection> m_pDetector;

		std::atomic<bool> m_initFlag;						//初始化标志
		seeta::ImageData m_img_data;						//图像数据
		std::vector<Ghost::SRect> m_faces;				//faces

		std::mutex m_mutex;									//互斥锁

		//互斥锁
		Ghost::signalslot::Signal<void(const std::vector<Ghost::SRect>&)> m_SIGNAL_void_rects;
		Ghost::signalslot::Slot m_SLOT_void_rects;

		const static string s_version;						//版本信息
		static std::atomic<bool> s_pathFlag;				//依赖的数据路径是否被设置
		static string s_modelPath;							//模型文件
	};

	std::atomic<bool> FaceDetector::Impl::s_pathFlag = false;
	string FaceDetector::Impl::s_modelPath = "";

#if( _MSC_TOOLSET_VER_ == 140 )
#ifdef NDEBUG
	const string FaceDetector::Impl::s_version = "vc140-r.0";
#else
	const string FaceDetector::Impl::s_version = "vc140-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 141)
#ifdef NDEBUG
	const string FaceDetector::Impl::s_version = "vc141-r.0";
#else
	const string FaceDetector::Impl::s_version = "vc141-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 142)
#ifdef NDEBUG
	const string FaceDetector::Impl::s_version = "vc142-r.0";
#else
	const string FaceDetector::Impl::s_version = "vc142-d.0";
#endif
#endif

	FaceDetector::FaceDetector()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("FaceDetector::FaceDetector::make_unique::Failured!!!");
		}
	}

	FaceDetector::~FaceDetector()
	{
		m_pImpl.reset();
	}

	EResult FaceDetector::setPath(const string& modelPath) noexcept(true)
	{
		if (!fs::exists(modelPath))
		{
			FaceDetector::Impl::s_pathFlag.store(false);
			return EResult::SR_Model_Path_Not_Exist;
		}

		FaceDetector::Impl::s_modelPath = modelPath;

		FaceDetector::Impl::s_pathFlag.store(true);

		return EResult::SR_OK;
	}

	const string& FaceDetector::getVersion() noexcept(true)
	{
		return FaceDetector::Impl::s_version;
	}

	EResult FaceDetector::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult FaceDetector::saveModelParam()
	{
		return EResult::SR_OK;
	}

	EResult FaceDetector::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult FaceDetector::antiModual()
	{
		this->saveModelParam();

		return m_pImpl->antiModual();
	}

	EResult FaceDetector::setModualParam(const EModualParamType type, const float value)
	{
		if (m_pImpl->m_pDetector == nullptr)
			return EResult::SR_Detector_Not_Exist;

		switch (type)
		{
		case EModualParamType::TYPE_Face_Detection_MinFaceSize:
		{
			m_pImpl->m_pDetector->SetMinFaceSize(static_cast<int32_t>(value));
			break;
		}
		case EModualParamType::TYPE_Face_Detection_MaxFaceSize:
		{
			m_pImpl->m_pDetector->SetMaxFaceSize(static_cast<int32_t>(value));
			break;
		}
		case EModualParamType::TYPE_Face_Detection_ScoreThresh:
		{
			m_pImpl->m_pDetector->SetScoreThresh(value);
			break;
		}
		case EModualParamType::TYPE_Face_Detection_ImagePyramidScaleFactor:
		{
			m_pImpl->m_pDetector->SetImagePyramidScaleFactor(value);
			break;
		}
		case EModualParamType::TYPE_Face_Detection_WindowStep:
		{
			m_pImpl->m_pDetector->SetWindowStep(static_cast<int32_t>(value), static_cast<int32_t>(value));
			break;
		}
		default:
			break;
		}

		return EResult::SR_OK;
	}

	EResult FaceDetector::detect(const cv::Mat& frameIn, cv::Mat& frameOut)
	{
		return m_pImpl->detect(frameIn, frameOut);
	}

	EDetectModual FaceDetector::getModualType() noexcept(true)
	{
		return EDetectModual::HumanFace_Detection_Modual;
	}

	void FaceDetector::bindSlotFaceFind(const std::function<void(const std::vector<Ghost::SRect>&)>& func)
	{
		m_pImpl->m_SLOT_void_rects = m_pImpl->m_SIGNAL_void_rects.connect(func);
	}
}/// namespace Ghost
