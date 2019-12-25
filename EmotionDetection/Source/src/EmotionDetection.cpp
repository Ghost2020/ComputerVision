#include "EmotionDetection.h"

#include <mutex>
#include <atomic>
#include <filesystem>
#include <memory>

#include <fstream>

#include "face_detection.h"

using namespace Ghost::signalslot;
namespace fs = std::filesystem;

namespace Ghost
{
	/**
	* \@brief 私有实现类
	*/
	class EmotionDetector::Impl
	{
	public:
		Impl()
			:
			m_faceDetector(nullptr),
			m_initFlag(false)
		{

		}

		~Impl()
		{
			antiModual();
		}

	public:
		/**
		* \@brief 初始化检测模块
		* \return 返回执行结果
		*/
		EResult initModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_initFlag.load())
				return EResult::SR_Detector_Already_Exist;

			if (m_faceDetector != nullptr)
				return EResult::SR_Detector_Already_Exist;

			m_faceDetector = std::make_unique<seeta::FaceDetection>(m_faceModelPath.c_str());
			m_faceDetector->SetMinFaceSize(60);
			m_faceDetector->SetScoreThresh(2.f);
			m_faceDetector->SetImagePyramidScaleFactor(0.8f);

			try
			{
				if (!m_emotionDetector.load(m_emotionXmlPath.c_str()))
					return EResult::SR_NG;
			}
			catch (cv::Exception& except)
			{
				cout << except.what() << endl;
				return EResult::SR_NG;
			}

			m_initFlag.store(true);

			return EResult::SR_OK;
		}

		/**
		* \@brief 反初始化检测模块
		* \return 返回执行结果
		*/
		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_initFlag.load())
				return EResult::SR_Detector_Not_Exist;

			if (m_faceDetector != nullptr)
			{
				m_faceDetector.reset();
				m_faceDetector = nullptr;
			}

			m_initFlag.store(false);

			return EResult::SR_OK;
		}

		/**
		* \@brief 检测图像
		* \@param frameIn 被检测的图像
		* \@param frameOut 需要绘制的图像
		* \return 返回执行结果
		*/
		EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if(!m_initFlag.load())
				return EResult::SR_Detector_Not_Exist;

			cv::Mat img_gray;
			cv::cvtColor(frameOut, img_gray, cv::COLOR_BGR2GRAY);

			seeta::ImageData img_data;
			img_data.data = img_gray.data;
			img_data.width = img_gray.cols;
			img_data.height = img_gray.rows;
			img_data.num_channels = 1;
			std::vector<seeta::FaceInfo> faces = m_faceDetector->Detect(img_data);

			m_faces.clear();
			for (const auto& face : faces)
			{
				const cv::Rect rect(face.bbox.x, face.bbox.y, face.bbox.width, face.bbox.height);
				m_faces.push_back(rect);
			}

			try
			{
				for (const auto& faceRect : m_faces)
				{
					if ( (faceRect.area() > 0)				&&
						 (faceRect.x >= 0)					&&
						 (faceRect.y < frameOut.rows)		&&
						 (faceRect.width < frameOut.cols)	&&
						 (faceRect.height < frameOut.rows)
						)
					{
						//绘制绿色框
						rectangle(frameOut, faceRect, Scalar(255, 0, 0), 2, 8, 0);

						Mat faceROI = frameOut(faceRect);//Bug 爆出点
						std::vector<Rect> smile;

						//-- In each face, detect smile
						m_emotionDetector.detectMultiScale(faceROI, smile, 1.1, 55, CASCADE_SCALE_IMAGE);

						string smileFlag = m_flagPath + "\\SmileFlag";
						if (smile.size() > 0)
						{
							for (const auto& rect : smile)
							{
								Rect rect(faceRect.x + rect.x, faceRect.y + rect.y, rect.width, rect.height);
								rectangle(frameOut, rect, Scalar(0, 0, 255), 2, 8, 0);
							}

							//修改标志
							fs::create_directories(smileFlag);
						}
						else
						{
							//修改标志
							fs::remove(smileFlag);
						}
					}
				}
			}
			catch (std::exception& except)
			{
				cout << except.what() << endl;
			}
		
			return EResult::SR_OK;
		}

	public:
		std::unique_ptr<seeta::FaceDetection> m_faceDetector;

		seeta::ImageData m_img_data;										//!< 图像数据					
		CascadeClassifier m_emotionDetector;								//!< 表情分类器

		std::vector<Rect> m_faces;											//!< 脸的位置

		size_t m_index;

		std::atomic_bool m_initFlag;
		std::mutex m_mutex;													//!< 互斥锁

		Signal<void(const std::vector<Ghost::EEmotion>&)> m_SIGNAL_void_emotion;		//!< 信号槽
		Slot m_SLOT_void_emotions;

		static std::string m_faceModelPath;									//!< Model文件路径
		static std::string m_emotionXmlPath;								//!< ModelXml文件路径
		static std::string m_flagPath;
		const static string s_version;										//!< 版本信息
	};

	std::string EmotionDetector::Impl::m_faceModelPath = "";
	std::string EmotionDetector::Impl::m_emotionXmlPath = "";
	std::string EmotionDetector::Impl::m_flagPath = "";

#if( _MSC_TOOLSET_VER_ == 140 )
	#ifdef NDEBUG
		const string EmotionDetector::Impl::s_version = "vc140-r.0";
	#else
		const string EmotionDetector::Impl::s_version = "vc140-d.0";
	#endif
#elif(_MSC_TOOLSET_VER_ == 141)
	#ifdef NDEBUG
		const string EmotionDetector::Impl::s_version = "vc141-r.0";
	#else
		const string EmotionDetector::Impl::s_version = "vc141-d.0";
	#endif
#elif(_MSC_TOOLSET_VER_ == 142)
	#ifdef NDEBUG
		const string EmotionDetector::Impl::s_version = "vc142-r.0";
	#else
		const string EmotionDetector::Impl::s_version = "vc142-d.0";
	#endif
#endif

	EmotionDetector::EmotionDetector()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("EmotionDetector::EmotionDetector::make_unique::Failured!!!");
		}
	}

	EmotionDetector::~EmotionDetector()
	{

	}

	EResult EmotionDetector::setPath(const string& faceModelPath, const string& emotionXmlPath, const string& flagPath) noexcept(true)
	{
		if (!fs::exists(faceModelPath))
			return EResult::SR_Face_Cascade_Xml_Not_Exist;

		if (!fs::exists(emotionXmlPath))
			return EResult::SR_Emotion_Cascade_Xml_Not_Exist;

		EmotionDetector::Impl::m_faceModelPath = faceModelPath;
		EmotionDetector::Impl::m_emotionXmlPath = emotionXmlPath;
		EmotionDetector::Impl::m_flagPath = flagPath;

		return EResult::SR_OK;
	}

	const string& EmotionDetector::getVersion() noexcept(true)
	{
		return EmotionDetector::Impl::s_version;
	}

	EResult EmotionDetector::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult EmotionDetector::saveModelParam()
	{
		return EResult::SR_OK;
	}

	EResult EmotionDetector::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult EmotionDetector::antiModual()
	{
		return m_pImpl->antiModual();
	}

	EResult EmotionDetector::setModualParam(const EModualParamType type, const float value)
	{
		return EResult::SR_OK;
	}

	EResult EmotionDetector::detect(const cv::Mat& frameIn, cv::Mat& frameOut)
	{
		return m_pImpl->detect(frameIn, frameOut);
	}

	EDetectModual EmotionDetector::getModualType() noexcept(true)
	{
		return EDetectModual::HumanFace_LandMark;
	}

	void EmotionDetector::bindSlotEmotionChanged(const std::function<void(const std::vector<Ghost::EEmotion>&)>& functor)
	{
		m_pImpl->m_SLOT_void_emotions  = m_pImpl->m_SIGNAL_void_emotion.connect(functor);
	}
}///namespace Ghost
