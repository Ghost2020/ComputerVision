#include "VisionManager.h"
#include "FgLog.h"

#include <exception>
#include <atomic>
#include <future>
#include <filesystem>
#include <list>
#include <mutex>

#include <Windows.h>
#include "stringapiset.h"

#include <opencv2/opencv.hpp>

#include "ObjectDetection.h"
#include "PoseDetector.h"
#include "FaceDetection.h"
#include "FaceCompare.h"
#include "FaceRecognition.h"
#include "FaceLandmark.h"
#include "EmotionDetection.h"


#ifndef GHOST_SLOT
#define GHOST_SLOT
#endif

using namespace std;
using namespace cv;
using namespace facegood;
using namespace facegood::signalslot;
namespace fs = std::experimental::filesystem;

namespace facegood
{
	/**
	* \@brief Private implementation of each detection module
	* \@desc 各个模块的执行顺序	1.模块为物体检测->2.人体pose检测->3.人脸位置检测->4.人脸数据库比对
	* \@desc 各个模块的优先级		照上依次降低
	*/
	class VisionManager::Impl
	{
	public:
		/**
		* \@brief 各个模块的状态
		*/
		struct SState
		{
			std::atomic<bool> m_bObjectDetectFlag, m_bObjectDetectShowFlag;	//目标检测模块显示标志
			std::atomic<bool> m_bPoseDetectFlag, m_bPoseDetectShowFlag;		//人体pose标志
			std::atomic<bool> m_bHumanFaceFlag, m_bHumanFaceShowFlag;		//人脸检测模块标志
			std::atomic<bool> m_bHumanCompareFlag, m_bHumanCompareShowFlag;	//人脸匹配检测模块
			std::atomic<bool> m_bInitFlag;									//模块初始化标志
			std::atomic<bool> m_bLogFlag;									//日志开启标志

			SState()
				: m_bObjectDetectFlag(false), m_bObjectDetectShowFlag(false), m_bPoseDetectFlag(false), m_bPoseDetectShowFlag(false),
				m_bHumanFaceFlag(false), m_bHumanFaceShowFlag(false), m_bHumanCompareFlag(false), m_bHumanCompareShowFlag(false),
				m_bInitFlag(false), m_bLogFlag(false)
			{}
		};
		//各个模块的状态
		SState m_State;

		/**
		* \@brief 相机参数
		*/
		struct SCameraParam
		{
			int CameraID;													//相机编号
			bool isStreamOpen;												//是否连接
			cv::VideoCapture m_capture;										//相机捕获对象
			cv::Mat m_frameDetect;											//检测用图像帧
			cv::Mat m_frameShow;											//显示用图像帧
			float RefreshRate;												//刷新率
			double width, height;											//图片宽度/高度
			double brightness, contrast, saturation, tone;					//相机显示参数
			SCameraParam()
				:
				CameraID(-1),
				isStreamOpen(false),
				RefreshRate(15),
				width(0.0), height(0.0),
				brightness(0.5f), contrast(0.5f), saturation(0.5f), tone(0.5f)
			{
				m_capture = cv::VideoCapture();
				m_frameDetect = cv::Mat();
				m_frameShow = cv::Mat();
			}
		};
		//相机对象
		SCameraParam m_Camera;

		//视觉检测工具箱
		std::list<std::unique_ptr<IVisionDetecter>> m_detectors;

		////信号槽 检索到物体
		Signal<void(const std::vector<std::string>&)> m_SIGNAL_void_objects;
		Slot m_SLOT_Objects;
		//信号槽 发现人
		Signal<void(void)> m_SIGNAL_void_void;
		Slot m_SLOT_Person;
		//信号槽 发现熟人
		Signal<void(const std::vector<facegood::SPersonInfor>&)> m_SIGNAL_void_person;
		Slot m_SLOT_Friend;
		//信号槽 发现人脸(外框)
		Signal<void(const std::vector<facegood::SRect>&)> m_SIGNAL_void_rects;
		Slot m_SLOT_rects;
		//信号槽 pose
		Signal<void(const std::vector<facegood::SPoint2D>&)> m_SIGNAL_void_pose;
		Slot m_SLOT_Poses;
		//信号槽 脸与 身份证ID 比对
		Signal<void(const bool)> m_SIGNAL_void_bool;
		Slot m_SLOT_Compare;
		//信号槽 相机状态事件
		Signal<void(void)> m_SIGNAL_void_Camera;
		Slot m_SLOT_Camera;
		//信号槽 情感状态
		Signal<void(const std::vector<facegood::EEmotion>&)> m_SIGNAL_void_emotions;
		Slot m_SLOT_Emotions;

		//互斥锁
		std::mutex m_mutex;

		//资源路径
		static wstring s_resourceBasePath;
		//可用的相机数量
		static size_t s_nCameraNum;
		//log对象
		static FgLog s_log;
		//版本
		const static string s_version;

	public:
		Impl()
		{
			s_log.writeLog("VisionManager::Impl::Impl()", FgLog::LOG_LEVEL_TRACE);

			m_detectors.clear();
		};

		~Impl()
		{
			for (const auto& detector : m_detectors)
				detector->antiModual();

			s_log.writeLog("VisionManager::Impl::~Impl()", FgLog::LOG_LEVEL_TRACE);
		};

	public:
		/**
		* \@brief Turn on the camera
		* \@param cameraIndex::相机的索引(默认从0开始)
		* \@return
		*/
		bool openCamera(const int cameraIndex)
		{
			if (m_Camera.m_capture.isOpened())
				return true;

			m_Camera.CameraID = cameraIndex;

			return m_Camera.m_capture.open(cameraIndex);
		}
		/**
		* \@brief shutdown the camera
		*/
		void closeCamera()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_Camera.m_capture.release();
		}

		/**
		* \@brief camera Setter
		* \@param propId::Parameter type
		* \@param param::
		* \@return Get the value of the camera's specified parameters
		*/
		bool setCameraParam(int propId, const double param)
		{
			return m_Camera.m_capture.set(propId, param);
		}

		/**
		* \@brief camera Getter
		* \@param propId::Parameter type
		* \@return Get the value of the camera's specified parameters
		*/
		double getCameraParam(int propId)
		{
			return m_Camera.m_capture.get(propId);
		}

		/**
		* \@brief
		*/
		void tick()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_Camera.m_capture.isOpened())
			{
				m_Camera.m_capture.read(m_Camera.m_frameDetect);
				m_Camera.m_frameShow = m_Camera.m_frameDetect;

				for (const auto& detector : m_detectors)
				{
					detector->detect(m_Camera.m_frameDetect, m_Camera.m_frameShow);
				}
			}
			else
			{
				this->m_SIGNAL_void_Camera();	//触发相机丢失信号事件
				m_Camera.isStreamOpen = false;
			}
		}

		/**
		* \@brief 初始化各个模块，需要是异步进行的，各个模块都太耗时了
		* \@warning 多个模块，各个GPU达不到要求！！！
		* \@return Returns the result of execution
		*/
		EResult initModule(const EDetectModual type)
		{
			EResult reuslt = EResult::SR_OK;
			for (const auto& detector : m_detectors)
			{
				if (detector->getModualType() == type)
					return reuslt = EResult::SR_Detector_Already_Exist;
			}

			s_log.writeLog("VisionManager::Impl::initModule()::start", FgLog::LOG_LEVEL_TRACE);

			switch (type)
			{
				case EDetectModual::Object_Detection_Modual:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<ObjectDetector>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					ObjectDetector* p = static_cast<ObjectDetector*>(Detector.get());
					p->bindSlotObjectFind(std::bind(&Impl::SlotObjectsFind, this, placeholders::_1));
					m_detectors.push_back(std::move(Detector));
					break;
				}
				case EDetectModual::Pose_Detection_Modual:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<PoseDetector>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					PoseDetector* p = static_cast<PoseDetector*>(Detector.get());
					p->bindSlotPoseFind(std::bind(&Impl::SlotPoseFind, this, placeholders::_1));
					m_detectors.push_back(std::move(Detector));
					break;
				}
				case EDetectModual::HumanFace_Detection_Modual:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<FaceDetector>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					FaceDetector* p = static_cast<FaceDetector*>(Detector.get());
					p->bindSlotFaceFind(std::bind(&Impl::SlotFaceRectFind, this, placeholders::_1));
					m_detectors.push_back(std::move(Detector));
					break;
				}
				case EDetectModual::HumanFace_Compare_Modual:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<FaceCompator>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					FaceCompator* p = static_cast<FaceCompator*>(Detector.get());
					p->bindSlotFaceCompare(std::bind(&Impl::SlotFaceIDCompare, this, placeholders::_1));
					m_detectors.push_back(std::move(Detector));
					break;
				}
				case EDetectModual::HumanFace_Recognition_Modual:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<FaceRecognition>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					FaceRecognition* p = static_cast<FaceRecognition*>(Detector.get());
					p->bindSlotFaceFind(std::bind(&Impl::SlotFriendsFind, this, placeholders::_1));
					m_detectors.push_back(std::move(Detector));
					break;
				}
				case EDetectModual::HumanFace_LandMark:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<FaceLandmark>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					/*FaceLandmark* p = static_cast<FaceLandmark*>(Detector.get());
					p->bindSlotLandMarkFind(std::bind(&Impl::SlotFaceLandmark, this, placeholders::_1));*/
					m_detectors.push_back(std::move(Detector));
					break;
				}
				case EDetectModual::HumanFace_Emotion:
				{
					std::unique_ptr<IVisionDetecter> Detector = std::make_unique<EmotionDetector>();
					if (Detector == nullptr)
					{
						reuslt = EResult::SR_Detector_Memory_Allocation_Failed;
						break;
					}
					reuslt = Detector->initModual();
					EmotionDetector* p = static_cast<EmotionDetector*>(Detector.get());
					p->bindSlotEmotionChanged(std::bind(&Impl::SlotFaceEmotion, this, placeholders::_1));
					m_detectors.push_back(std::move(Detector));
					break;
				}
				default:
				{
					reuslt = EResult::SR_UNDEFINE;
					break;
				}
			}

			s_log.writeLog("VisionManager::Impl::initModule()::over", FgLog::LOG_LEVEL_TRACE);

			return reuslt;
		}

		/**
		* \@brief shutdown 各个模块
		* \@return Returns the result of execution
		*/
		EResult antiModual(const EDetectModual type)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			s_log.writeLog("VisionManager::Impl::antiModual()::start", FgLog::LOG_LEVEL_TRACE);

			EResult res = EResult::SR_OK;
			for (auto iter = m_detectors.begin(); iter != m_detectors.end(); iter++)
			{
				if ((*iter)->getModualType() == type)
				{
					res = (*iter)->antiModual();
					(*iter).reset();
					m_detectors.erase(iter);
					break;
				}
			}

			s_log.writeLog("VisionManager::Impl::antiModual()::over", FgLog::LOG_LEVEL_TRACE);

			return res;
		}

		/**
		* \@brief Setting Module Parameters
		* \@param type
		* \@param modualParamType
		* \@param paramValue
		* \@return Returns the result of execution
		*/
		EResult setModualParam(const EDetectModual type, const EModualParamType modualParamType, const float paramValue)
		{
			EResult res = EResult::SR_UNDEFINE;
			for (const auto& detector : m_detectors)
			{
				if (detector->getModualType() == type)
				{
					res = detector->setModualParam(modualParamType, paramValue);
					break;
				}
			}

			return res;
		}

	public GHOST_SLOT:
		/**
		* \@brief 触发Objects发现的信号
		*/
		void SlotObjectsFind(const std::vector<std::string>& objects)
		{
			m_SIGNAL_void_objects(objects);
		}
		/**
		* \@brief 触发 发现人的信号
		*/
		void SlotFriendsFind(const std::vector<facegood::SPersonInfor>& friends)
		{
			m_SIGNAL_void_person(friends);
		}
		/**
		* \@brief 触发
		*/
		void bindSlotFaceFind()
		{
			m_SIGNAL_void_void();
		}
		/**
		* \@brief 触发
		*/
		void SlotPoseFind(const std::vector<facegood::SPoint2D>& points)
		{
			m_SIGNAL_void_pose(points);
		}
		/**
		* \@brief 触发
		*/
		void SlotFaceRectFind(const std::vector<facegood::SRect>& faces)
		{
			m_SIGNAL_void_rects(faces);
		}

		/**
		* \@brief 触发 人脸与身份证比对结果
		*/
		void SlotFaceIDCompare(const bool result)
		{
			m_SIGNAL_void_bool(result);
		}

		/**
		* \@biref 触发人脸Landmark
		*/
		void SlotFaceLandmark(const std::vector<float>& points)
		{

		}

		/**
		* \@brief 触发表情检测
		*/
		void SlotFaceEmotion(const std::vector<facegood::EEmotion>& emotions)
		{
			m_SIGNAL_void_emotions(emotions);
		}
	};


	size_t VisionManager::Impl::s_nCameraNum = 0;
	wstring VisionManager::Impl::s_resourceBasePath = L"";
	FgLog VisionManager::Impl::s_log;
#if  (_MSC_TOOLSET_VER_ == 140)
	#ifdef NDEBUG
		const string VisionManager::Impl::s_version = "vc140-r.0";
	#else
		const string VisionManager::Impl::s_version = "vc140-d.0";
	#endif
#elif(_MSC_TOOLSET_VER_ == 141)
	#ifdef NDEBUG
		const string VisionManager::Impl::s_version = "vc141-r.0";
	#else
		const string VisionManager::Impl::s_version = "vc141-d.0";
	#endif
#elif(_MSC_TOOLSET_VER_ == 142)
	#ifdef NDEBUG
		const string VisionManager::Impl::s_version = "vc142-r.0";
	#else
		const string VisionManager::Impl::s_version = "vc142-d.0";
	#endif
#endif

	EResult VisionManager::setResourcePath(const wstring& resourcePath)
	{
		if (!fs::exists(resourcePath))
		{
			return EResult::SR_Folder_Not_Exist;
		}

		EResult result = EResult::SR_UNDEFINE;

		//日志保存位置
		Impl::s_log.setLogSavePath(resourcePath + L"\\Log");
		Impl::s_resourceBasePath = resourcePath;

		string ResPath = VisionManager::wstringTostring(resourcePath) + "\\ComputerVision-Res";

		//ObjectDetection资源路径
		string objectDetectionResPath = ResPath + "\\ObjectDetection\\Dependents";
		result = ObjectDetector::setPath
		(
			objectDetectionResPath + "\\data\\coco.names",
			objectDetectionResPath + "\\cfg\\yolov3.cfg",
			objectDetectionResPath + "\\weight\\yolov3.weights"
		);

		//PoseDetection资源路径
		string openPoseResPath = ResPath + "\\PoseDetection\\Dependents";
		result = PoseDetector::setPath
		(
			openPoseResPath + "\\models\\pose\\body_25\\",
			"pose_deploy.prototxt",
			"pose_iter_584000.caffemodel"
		);

		//FaceDetection资源路径
		string faceResPath = ResPath + "\\FaceDetection\\Dependents";
		result = FaceDetector::setPath
		(
			faceResPath + "\\model\\seeta_fd_frontal_v1.0.bin"
		);

		//FaceRecongnition资源路径
		string faceRecongnitionResPath = ResPath + "\\FaceRecognition\\Dependents";
		result = FaceRecognition::setPath
		(
			"",
			"",
			""
		);

		//FaceCompare资源路径
		string faceCompareResPath = ResPath + "\\FaceCompare\\Dependents";
		result = FaceCompator::setPath
		(
			"",
			"",
			""
		);

		//FaceLandmark资源路径
		string faceLandmarkResPath = ResPath + "\\FaceLandmark\\Dependents";
		result = FaceLandmark::setPath
		(
			faceLandmarkResPath + "\\roboman-landmark-model.bin",
			faceLandmarkResPath + "\\haar_roboman_ff_alt2.xml"
		);

		//EmotionDetection资源路径
		string faceEmotionResPath = ResPath + "\\EmotionDetection\\Dependents";
		result = EmotionDetector::setPath
		(
			faceEmotionResPath + "\\seeta_fd_frontal_v1.0.bin",
			"D:\\Project\\UE4\\PluginDevelop_4_21\\FaceGoodLiveLink\\Plugins\\FaceGoodRuntime\\Content\\ComputerVision-Res\\EmotionDetection\\Dependents\\haarcascade_smile.xml"
			/*faceEmotionResPath + "\\haarcascade_smile.xml"*/
		);

		return result;
	}

	const string& VisionManager::getVersion() noexcept(true)
	{
		return VisionManager::Impl::s_version;
	}

	const string& VisionManager::getDetectionModualVersion(const EDetectModual modualType) noexcept(true)
	{
		switch (modualType)
		{
		case EDetectModual::Object_Detection_Modual:
			return ObjectDetector::getVersion();
		case EDetectModual::Pose_Detection_Modual:
			return PoseDetector::getVersion();
		case EDetectModual::HumanFace_Detection_Modual:
			return FaceDetector::getVersion();
		case EDetectModual::HumanFace_Compare_Modual:
			return FaceCompator::getVersion();
		case EDetectModual::HumanFace_Recognition_Modual:
			return FaceRecognition::getVersion();
		case EDetectModual::HumanFace_LandMark:
			return FaceLandmark::getVersion();
		case EDetectModual::HumanFace_Emotion:
			return EmotionDetector::getVersion();
		}

		const static string& version = "0.0.0";

		return version;
	}

	VisionManager::VisionManager()
		: 
		m_pImpl(std::make_unique<Impl>()),
		data(nullptr),
		m_nImageWidth(0),
		m_nImageHeight(0)
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("VisionManager::VisionManager::make_uniqu::failured!");
		}
	}

	VisionManager::~VisionManager()
	{
		m_pImpl.reset();
	}

	bool VisionManager::openCamera(const int cameraIndex)
	{
		return m_pImpl->openCamera(cameraIndex);
	}

	void VisionManager::closeCamera()
	{
		m_pImpl->closeCamera();
	}

	bool VisionManager::setCameraParam(const ECameraParamType paramType, const double paramValue)
	{
		return m_pImpl->setCameraParam(static_cast<int>(paramType), paramValue);
	}

	double VisionManager::getCameraParam(const ECameraParamType paramType)
	{
		return m_pImpl->getCameraParam(static_cast<int>(paramType));
	}

	void VisionManager::setShowFlag(const EDetectModual modualType, const bool showFlag)
	{
		switch (modualType)
		{
		case EDetectModual::Object_Detection_Modual:
		{
			m_pImpl->m_State.m_bObjectDetectShowFlag.store(showFlag);
			break;
		}
		case EDetectModual::Pose_Detection_Modual:
		{
			m_pImpl->m_State.m_bPoseDetectShowFlag.store(showFlag);
			break;
		}
		case EDetectModual::HumanFace_Detection_Modual:
		{
			m_pImpl->m_State.m_bHumanFaceShowFlag.store(showFlag);
			break;
		}
		case EDetectModual::HumanFace_Compare_Modual:
		{
			m_pImpl->m_State.m_bHumanCompareShowFlag.store(showFlag);
			break;
		}
		default:
			break;
		}
	}

	EResult VisionManager::setModualParam(const EDetectModual modualType, const EModualParamType modualParamType, const float paramValue)
	{
		return m_pImpl->setModualParam(modualType, modualParamType, paramValue);
	}

	void VisionManager::tick()
	{
		m_pImpl->tick();

		if (!m_pImpl->m_Camera.m_frameDetect.empty())
		{
			data = m_pImpl->m_Camera.m_frameShow.data;
			m_nImageWidth = m_pImpl->m_Camera.m_frameDetect.cols;
			m_nImageHeight = m_pImpl->m_Camera.m_frameDetect.rows;
		}
		else
		{
			data = nullptr;
			m_nImageWidth = 0;
			m_nImageHeight = 0;
		}
	}

	EResult VisionManager::initModule(const EDetectModual modualType)
	{
		return m_pImpl->initModule(modualType);
	}

	EResult VisionManager::antiModual(const EDetectModual modualType)
	{
		return m_pImpl->antiModual(modualType);
	}

	EResult VisionManager::loadCameraParam(const wstring& cameraParamPath)
	{
		return EResult::SR_OK;
	}

	EResult VisionManager::saveCameraParam(const wstring& cameraParamPath)
	{
		return EResult::SR_OK;
	}

	void VisionManager::bindSlotObjectFind(const std::function<void(const std::vector<std::string>&)>& functor)
	{
		m_pImpl->m_SLOT_Objects = m_pImpl->m_SIGNAL_void_objects.connect(functor);
	}

	void VisionManager::bindSlotPersonFind(const std::function<void(void)>& func)
	{
		m_pImpl->m_SLOT_Person = m_pImpl->m_SIGNAL_void_void.connect(func);
	}

	void VisionManager::bindSlotFriendFind(const std::function<void(const std::vector<facegood::SPersonInfor>&)>& functor)
	{
		m_pImpl->m_SLOT_Friend = m_pImpl->m_SIGNAL_void_person.connect(functor);
	}

	void VisionManager::bindSlotCameraState(const std::function<void(void)>& functor)
	{
		m_pImpl->m_SLOT_Camera = m_pImpl->m_SIGNAL_void_Camera.connect(functor);
	}

	void VisionManager::bindSlotEmotionState(const std::function<void(const std::vector<facegood::EEmotion>&)>& functor)
	{
		m_pImpl->m_SLOT_Emotions = m_pImpl->m_SIGNAL_void_emotions.connect(functor);
	}

	string VisionManager::wstringTostring(const wstring& wstr)
	{
		std::string str;
		int nLen = (int)wstr.length();
		str.resize(nLen, ' ');
		int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);
		if (nResult == 0)
			return "";

		return str;
	}

	std::wstring VisionManager::stringTowstring(const std::string& str)
	{
		int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* wide = new wchar_t[num];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
		std::wstring w_str(wide);
		delete[] wide;
		return w_str;
	}
}///namespace facegood
