#include "FaceCompare.h"

#include "arcsoft_idcardveri.h"
#include "merror.h"
#include "asvloffscreen.h"
#include "amcomdef.h"

#include <atomic>
#include <mutex>

using namespace std;

namespace facegood
{
	/**
	* \@brief 私有实现类
	*/
	class FaceCompator::Impl
	{
	public:
		Impl()
			:
			m_engineHandle(NULL),
			m_initFlag(false),
			m_threshold(0.82f)
		{}

		~Impl()
		{
			antiModual();
		}

	public:
		/**
		* \@brief 初始化检测环境
		* \@return 返回执行结果
		*/
		EResult initModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_engineHandle == NULL || m_engineHandle)
				return EResult::SR_ASF_Engine_Init_Already;

			MPChar appID = const_cast<MPChar>(s_appID.c_str());
			MPChar sdkKey = const_cast<MPChar>(s_sdkKey.c_str());

			//Step1::激活
			MRESULT res = ArcSoft_FIC_Activate(appID, sdkKey);
			if (res != MOK)
				return EResult::SR_ASF_Activation_Failed;

			//Step2::引擎初始化
			res = ArcSoft_FIC_InitialEngine(&m_engineHandle);
			if (res != MOK)
				return EResult::SR_ASF_Init_Engine_Fail;

			m_initFlag.store(true);
			return EResult::SR_OK;
		}

		/**
		* \@brief 反初始化检测环境
		* \@return 返回执行结果
		*/
		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_engineHandle == NULL)
				return EResult::SR_ASF_Engine_Not_Init;

			m_initFlag.store(false);

			MRESULT res = ArcSoft_FIC_UninitialEngine(m_engineHandle);
			if (res != MOK)
				return EResult::SR_ASF_UnInit_Engine_Fail;

			return EResult::SR_OK;
		}

		/**
		* \@brief 人脸与身份证信息进行比对
		* \@param face 人脸图像数据
		* \@param ID 身份证图像数据
		* \@return 返回执行结果
		*/
		EResult compare(const cv::Mat& face, cv::Mat& ID)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			/* 读取预览静态图片信息，并保存到ASVLOFFSCREEN结构体 （以ASVL_PAF_RGB24_B8G8R8格式为例） 图片数据为BGR原始数据 */
			//图像转换
			IplImage Face = face;
			IplImage id = ID;

			ASVLOFFSCREEN imgInfo0 = { 0 };
			imgInfo0.i32Width = Face.width;
			imgInfo0.i32Height = Face.height;
			imgInfo0.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
			imgInfo0.pi32Pitch[0] = imgInfo0.i32Width * 3;
			imgInfo0.ppu8Plane[0] = (MUInt8*)Face.imageData;

			/* 人脸特征提取 0-静态图片 1-视频 */
			//LPAFIC_FSDK_FACERES pFaceRes = (LPAFIC_FSDK_FACERES)malloc(sizeof(AFIC_FSDK_FACERES));
			AFIC_FSDK_FACERES faceRes = { 0 };
			MRESULT res = ArcSoft_FIC_FaceDataFeatureExtraction(m_engineHandle, 0, &imgInfo0, &faceRes);
			if (res != MOK)
				return EResult::SR_ASF_Face_Feature_Extraction_Failed;

			/* 读取证件照静态图片信息，并保存到ASVLOFFSCREEN结构体 （以ASVL_PAF_RGB24_B8G8R8格式为例） 图片数据为BGR原始数据 */
			ASVLOFFSCREEN imgInfo1 = { 0 };
			imgInfo1.i32Width = id.width;
			imgInfo1.i32Height = id.height;
			imgInfo1.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
			imgInfo1.pi32Pitch[0] = imgInfo1.i32Width * 3;
			imgInfo1.ppu8Plane[0] = (MUInt8*)id.imageData;

			/* 证件照特征提取 */
			res = ArcSoft_FIC_IdCardDataFeatureExtraction(m_engineHandle, &imgInfo1);
			if (res != MOK)
				return EResult::SR_ASF_IdCard_Feature_Extraction_Failed;

			/* 人证比对 */
			MFloat pSimilarScore = 0.0f;
			MInt32 pResult = 0;
			res = ArcSoft_FIC_FaceIdCardCompare(m_engineHandle, m_threshold, &pSimilarScore, &pResult);
			if (res != MOK)
				return EResult::SR_ASF_Face_IdCard_Compare_Failed;

			//发送信号

			return EResult::SR_OK;
		}

		/**
		* \@brief 绘制信息
		* \@param drawtoShow 被绘制的图像数据 传入 传出属性
		*/
		void draw(cv::Mat& drawtoShow)
		{

		}

		/**
		* \@brief 设置检测参数
		* \@param value
		* \@return 执行结果
		*/
		EResult setModualParam(const float value)
		{
			m_threshold = value;
			return EResult::SR_OK;
		}

	public:
		MHandle m_engineHandle;				//引擎句柄

		std::atomic_bool m_initFlag;		//模块初始化标志

		ASVLOFFSCREEN m_infos;

		MFloat m_threshold;					//比对的阈值

		//信号槽
		facegood::signalslot::Signal<void(const bool)> m_SIGNAL_void_bool;
		facegood::signalslot::Slot m_SLOT_void_bool;

		std::mutex m_mutex;					//互斥锁

		const static string s_version;		//版本信息
		const static string s_appID;		//虹软视觉 appID		###有效期为1年 2019/8/8开始使用####
		const static string s_sdkKey;		//虹软视觉 sdkKey	###有效期为1年 2019/8/8开始使用####
	};

#if( _MSC_TOOLSET_VER_ == 140 )
#ifdef NDEBUG
	const string FaceCompator::Impl::s_version = "vc140-r.0";
#else
	const string FaceCompator::Impl::s_version = "vc140-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 141)
#ifdef NDEBUG
	const string FaceCompator::Impl::s_version = "vc141-r.0";
#else
	const string FaceCompator::Impl::s_version = "vc141-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 142)
#ifdef NDEBUG
	const string FaceCompator::Impl::s_version = "vc142-r.0";
#else
	const string FaceCompator::Impl::s_version = "vc142-d.0";
#endif
#endif

	const string FaceCompator::Impl::s_appID = "9Wi3M1eb6QN8rxraQsuXSgTap4DHRExgJb4natX6ct2F";
	const string FaceCompator::Impl::s_sdkKey = "3sjr3AJJHi3oL8GGbGtDURZbiXptp1Q5hPvm1j87yCt1";

	FaceCompator::FaceCompator()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("FaceCompator::FaceCompator::make_uniqu::failured!");
		}
	}

	FaceCompator::~FaceCompator()
	{
		antiModual();
	}

	EResult FaceCompator::setPath(const string& dataPath, const string& cfgPath, const string& weightPath) noexcept(true)
	{
		return EResult::SR_OK;
	}

	const string& FaceCompator::getVersion() noexcept(true)
	{
		return FaceCompator::Impl::s_version;
	}

	EResult FaceCompator::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult FaceCompator::saveModelParam()
	{

		return EResult::SR_OK;
	}

	EResult FaceCompator::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult FaceCompator::antiModual()
	{
		this->saveModelParam();

		return m_pImpl->antiModual();
	}

	EResult FaceCompator::setModualParam(const EModualParamType type, const float value)
	{
		return m_pImpl->setModualParam(value);
	}

	EResult FaceCompator::detect(const cv::Mat& frameIn, cv::Mat& frameOut)
	{
		return m_pImpl->compare(frameIn, frameOut);
	}

	EDetectModual FaceCompator::getModualType() noexcept(true)
	{
		return EDetectModual::HumanFace_Compare_Modual;
	}

	void FaceCompator::bindSlotFaceCompare(const std::function<void(const bool)>& func)
	{
		m_pImpl->m_SLOT_void_bool = m_pImpl->m_SIGNAL_void_bool.connect(func);
	}
}///namespace facegood