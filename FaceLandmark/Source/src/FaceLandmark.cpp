#include "FaceLandmark.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <atomic>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include "ldmarkmodel.h"

namespace fs = std::experimental::filesystem;
using namespace facegood::signalslot;

namespace facegood
{
	class FaceLandmark::Impl
	{
	public:
		Impl()
			:m_initFlag(false),
			m_pDetector(nullptr)
		{

		}

		~Impl()
		{
			antiModual();
		}

	public:
		/**
		* \@brief ��ʼ�����ģ��
		* \return ����ִ�н��
		*/
		EResult initModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_initFlag.load())
				return EResult::SR_Detector_Already_Exist;

			if (m_pDetector == nullptr)
			{
				m_pDetector = std::make_unique<ldmarkmodel>();

				if (!load_ldmarkmodel(Impl::m_modelPath, Impl::m_modelXmlPath, *m_pDetector))
					return EResult::SR_NG;
			}

			m_initFlag.store(true);

			return EResult::SR_OK;
		}

		/**
		* \@brief ����ʼ�����ģ��
		* \return ����ִ�н��
		*/
		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_initFlag.load())
				return EResult::SR_Detector_Not_Exist;

			if (m_pDetector != nullptr)
			{
				m_pDetector.reset();
				m_pDetector = nullptr;
			}

			m_initFlag.store(false);

			return EResult::SR_OK;
		}

		/**
		* \@brief ���ͼ��
		* \@param frameIn ������ͼ��
		* \@param frameOut ��Ҫ���Ƶ�ͼ��
		* \return ����ִ�н��
		*/
		EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_initFlag.load())
				return EResult::SR_Detector_Not_Exist;

			if (m_pDetector != nullptr)
			{
				m_pDetector->track(frameIn, m_currentShape);

				if (!frameOut.empty())
				{
					draw(frameOut, m_currentShape);
				}
			}

			return EResult::SR_OK;
		}

		/**
		* \@brief ����ͼ��
		* \@param frameDraw ����
		* \@param data ��Ҫ���Ƶ�����
		*/
		void draw(cv::Mat& frameDraw, const cv::Mat& data)
		{
			int numLandmarks = data.cols / 2;
			for (int j = 0; j < numLandmarks; j++)
			{
				int x = data.at<float>(j);
				int y = data.at<float>(j + numLandmarks);
				std::stringstream ss;
				ss << j;

				cv::circle(frameDraw, cv::Point(x, y), 2, cv::Scalar(0, 0, 255), -1);
			}
		}

	public:
		std::unique_ptr<ldmarkmodel> m_pDetector;							//!< ������
		cv::Mat m_currentShape;												//!< ������������
		std::vector<float> m_matrix;										//!< landmark����

		std::atomic_bool m_initFlag;
		std::mutex m_mutex;													//!< ������

		Signal<void(const std::vector<float>&)> m_SIGNAL_void_vecFloat;		//!< �źŲ�
		Slot m_SLOT_void_vecFloat;

		static std::string m_modelPath;										//!< Model�ļ�·��
		static std::string m_modelXmlPath;									//!< ModelXml�ļ�·��
		const static string s_version;										//!< �汾��Ϣ
	};

	
	std::string FaceLandmark::Impl::m_modelPath = "";
	std::string FaceLandmark::Impl::m_modelXmlPath = "";

#if( _MSC_TOOLSET_VER_ == 140 )
	#ifdef NDEBUG
		const string FaceLandmark::Impl::s_version = "vc140-r.0";
	#else
		const string FaceLandmark::Impl::s_version = "vc140-d.0";
	#endif
#elif(_MSC_TOOLSET_VER_ == 141)
	#ifdef NDEBUG
		const string FaceLandmark::Impl::s_version = "vc141-r.0";
	#else
		const string FaceLandmark::Impl::s_version = "vc141-d.0";
	#endif
#elif(_MSC_TOOLSET_VER_ == 142)
	#ifdef NDEBUG
		const string FaceLandmark::Impl::s_version = "vc142-r.0";
	#else
		const string FaceLandmark::Impl::s_version = "vc142-d.0";
	#endif
#endif

	FaceLandmark::FaceLandmark()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("FaceLandmark::FaceLandmark::make_unique::Failured!!!");
		}
	}

	FaceLandmark::~FaceLandmark()
	{

	}
	
	EResult FaceLandmark::setPath(const string& modelPath, const string& modelXmlPath) noexcept(true)
	{
		if (!fs::exists(modelPath))
			return EResult::SR_Model_Path_Not_Exist;

		if (!fs::exists(modelXmlPath))
			return EResult::SR_Model_Path_Not_Exist;

		FaceLandmark::Impl::m_modelPath = modelPath;
		FaceLandmark::Impl::m_modelXmlPath = modelXmlPath;

		return EResult::SR_OK;
	}

	const string& FaceLandmark::getVersion() noexcept(true)
	{
		return FaceLandmark::Impl::s_version;
	}

	EResult FaceLandmark::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult FaceLandmark::saveModelParam()
	{
		return EResult::SR_OK;
	}

	EResult FaceLandmark::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult FaceLandmark::antiModual()
	{
		return m_pImpl->antiModual();
	}

	EResult FaceLandmark::setModualParam(const EModualParamType type, const float value)
	{
		return EResult::SR_OK;
	}

	EResult FaceLandmark::detect(const cv::Mat& frameIn, cv::Mat& frameOut)
	{
		return m_pImpl->detect(frameIn, frameOut);
	}

	EDetectModual FaceLandmark::getModualType() noexcept(true)
	{
		return EDetectModual::HumanFace_LandMark;
	}

	EResult FaceLandmark::saveFaceToDataBase(const cv::Mat& frameSave, const SPersonInfor& infor)
	{
		return EResult::SR_OK;
	}

	void FaceLandmark::bindSlotLandMarkFind(const std::function<void(const std::vector<float>&)>& func)
	{

	}
}///namespace facegood