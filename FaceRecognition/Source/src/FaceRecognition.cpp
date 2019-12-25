#include "FaceRecognition.h"

#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "merror.h"
#include "direct.h"

#include <atomic>
#include <vector>
#include <thread>
#include <mutex>

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; }

using namespace std;

namespace Ghost
{
	/**
	* \@brief ˽����ʵ��
	*/
	class FaceRecognition::Impl
	{
	public:
		Impl()
			:
			m_handleEngine(NULL)
		{
			//~~~~~~~~~~~~~~~~~~~~~~
			//loadFaceDataBase();
		}

		~Impl()
		{
			antiModual();
		}

	public:
		/**
		* \@brief ��ʼ��ʶ������
		* \@return ����ִ�еĽ��
		*/
		EResult initModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if ((m_checkFlags.initFalg.load()) || (m_handleEngine != NULL))
				return EResult::SR_ASF_Engine_Init_Already;

			MPChar appID = const_cast<MPChar>(s_ArcVisionAppID.c_str());
			MPChar sdkKey = const_cast<MPChar>(s_ArcVisionKey.c_str());

			//Step1::����ӿ�,����������
			MRESULT res = ASFOnlineActivation(appID, sdkKey);
			if (res != MOK)
				res = ASFActivation(appID, sdkKey);

			if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
				return EResult::SR_ASF_Activation_Failed;

			//Step2::��ȡ�����ļ���Ϣ
			res = ASFGetActiveFileInfo(&m_infos.fileInfo);
			if (res != MOK)
				return EResult::SR_ASF_Get_Active_FileInfo_Fail;

			//Step3::��ʼ���ӿ�(!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!)
			MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS;
			res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, 30, 10, mask, &m_handleEngine);
			if (res != MOK)
				return EResult::SR_ASF_Init_Engine_Fail;

			m_checkFlags.initFalg.store(true);

			return EResult::SR_OK;
		}

		/**
		* \@brief �ر�ʶ������
		* \@return ����ִ�еĽ��
		*/
		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if ((m_handleEngine == NULL) || (!m_checkFlags.initFalg.load()))
				return EResult::SR_OK;

			m_checkFlags.initFalg.store(false);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));

			//����ʼ��
			MRESULT res = ASFUninitEngine(m_handleEngine);
			m_handleEngine = NULL;
			if (res != MOK)
				return EResult::SR_ASF_UnInit_Engine_Fail;

			return EResult::SR_OK;
		}

		/**
		* \@brief ���ر����������ݿ���Ϣ	###���������ݿ�Ƕ�ʱ�����ö��̲߳�ѯ###
		* \@param databasePath ���������ļ���·��
		* \@return ����ִ�еĽ��
		*/
		EResult loadFaceDataBase(const string& databasePath)
		{
			return EResult::SR_OK;
		}

		/**
		* \@brief ������Ҫ�ӵ����ݿ����Ϣ
		* \@return ����ִ�еĽ��
		*/
		EResult saveFaceToDataBase(const cv::Mat& frameSave, const SPersonInfor& infor)
		{
			return EResult::SR_OK;
		}

		/**
		* \@brief ��������ʶ����
		* \@param frameIn �������Ҫ���м��ͼ�񲻿��޸�
		* \@param frameOut ������Ҫ���Ʊ�ʶ�𵽵���Ϣ
		* \@return ����ִ�еĽ��
		*/
		EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			//�����Ч��
			if (m_handleEngine == NULL)
				return EResult::SR_ASF_Engine_Handle_NULL;

			//����Ƿ������������
			if (!m_checkFlags.initFalg.load())
				return EResult::SR_ASF_Engine_Not_Init;

			//ͼ��ת��
			IplImage temp = frameIn;
			IplImage* cutImg = cvCreateImage(cvSize(temp.width - temp.width % 4, temp.height), IPL_DEPTH_8U, temp.nChannels);
			CutIplImage(&temp, cutImg, 0, 0);

			//�ȼ������
			MRESULT res = MOK;
			m_infos.multiFaceInfos = { 0 };
			res = ASFDetectFaces(m_handleEngine, cutImg->width, cutImg->height, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)cutImg->imageData, &m_infos.multiFaceInfos);

			MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
			res = ASFProcess(m_handleEngine, cutImg->width, cutImg->height, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)cutImg->imageData, &m_infos.multiFaceInfos, processMask);

			//�������
			if (m_checkFlags.age.load())
			{
				m_infos.ageInfos = { 0 };
				res = ASFGetAge(m_handleEngine, &m_infos.ageInfos);
				if (res != MOK)
				{
					cvReleaseImage(&cutImg);
					return EResult::SR_ASF_Get_Age_Failed;
				}
			}
			//����Ա�
			if (m_checkFlags.gender.load())
			{
				m_infos.genderInfos = { 0 };
				res = ASFGetGender(m_handleEngine, &m_infos.genderInfos);
				if (res != MOK)
				{
					cvReleaseImage(&cutImg);
					return EResult::SR_ASF_Get_Gender_Failed;
				}
			}
			//3D�Ƕ�
			if (m_checkFlags.angle.load())
			{
				m_infos.angleInfos = { 0 };
				res = ASFGetFace3DAngle(m_handleEngine, &m_infos.angleInfos);
				if (res != MOK)
				{
					cvReleaseImage(&cutImg);
					return EResult::SR_ASF_Get_Face3DAngle_Failed;
				}
			}
			//������Ϣ
			if (m_checkFlags.liveness.load())
			{
				m_infos.rgbLivenessInfos = { 0 };
				res = ASFGetLivenessScore(m_handleEngine, &m_infos.rgbLivenessInfos);
				if (res != MOK)
				{
					cvReleaseImage(&cutImg);
					return EResult::SR_ASF_Get_LivenessScore_Failed;
				}
			}
			//�����ȶ�
			//if (m_checkFlags.compare.load())
			//{
			//	//��ȡ����
			//	res = ASFFaceFeatureExtract(m_handleEngine, cutImg->width, cutImg->height, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)cutImg->imageData,&m_infos.singleFaceInfos,&m_infos.faceFeature);
			//	if (res != MOK)
			//		return EResult::SR_ASF_Face_Feature_Extraction_Failed;

			//	//MFloat confidenceLevel = ASFFaceFeatureCompare();
			//}

			cvReleaseImage(&cutImg);

			//����
			//����λ����Ϣ
			if (m_infos.multiFaceInfos.faceNum <= 0)
				return EResult::SR_OK;

			draw(frameOut);

			EResult result = EResult::SR_OK;
			if (res != MOK)
				result = EResult::SR_NG;

			return result;
		}

		/**
		* \@brief ��ʶ�𵽵���Ϣ���Ƶ�ͼƬ��
		* \@param tobedraw �����Ƶ�ͼƬ
		*/
		void draw(cv::Mat& tobedraw)
		{
			const cv::Scalar faceRectColor(0, 128, 0), charColor(255, 255, 255);
			const cv::Scalar attributeRectColor(255, 191, 0);

			std::vector<cv::Point> vecPoint;
			std::vector<Ghost::SPersonInfor> persons(m_infos.multiFaceInfos.faceNum);

			int textCount = 0;
			int textHeight = 15;
			//���������򣬺����Կ�
			if (m_infos.multiFaceInfos.faceRect != NULL)
			{
				for (int i = 0; i < m_infos.multiFaceInfos.faceNum; i++)
				{
					int left = m_infos.multiFaceInfos.faceRect[i].left;
					int top = m_infos.multiFaceInfos.faceRect[i].top;
					int width = m_infos.multiFaceInfos.faceRect[i].right - left;
					int height = m_infos.multiFaceInfos.faceRect[i].bottom - top;
					
					Rect faceRect(left, top, width, height);
					Rect attributeRect(left + width + 12, top+ height /2 - textHeight * 1.5, width/3*2, textHeight*4);
					cv::rectangle(tobedraw, faceRect, faceRectColor, 2);					//������

					cv::Point tranglePoints[1][3];
					tranglePoints[0][0] = cv::Point(left + width, top + height/2); 
					tranglePoints[0][1] = cv::Point(left + width + width / 8, top + height / 16 * 7);
					tranglePoints[0][2] = cv::Point(left + width+ +width / 8, top + height / 16 * 9);

					vecPoint.push_back(cv::Point(left + width + width / 8, top + height / 2 - textHeight * 1.5));
					int npt[1] = { 3 };
					const Point* ppt[1] = { tranglePoints[0] };
					cv::fillPoly(tobedraw, ppt, npt, 1, attributeRectColor);
					cv::rectangle(tobedraw, attributeRect, attributeRectColor, CV_FILLED);	//���Կ�
				}
			}
			//����������Ϣ
			if (m_infos.ageInfos.ageArray != NULL)
			{
				for (int i = 0; i < m_infos.ageInfos.num; i++)
				{
					const cv::String name = std::string("age:") + std::to_string(m_infos.ageInfos.ageArray[i]).c_str();
					cv::putText(tobedraw, name, cv::Point(vecPoint[i].x, vecPoint[i].y + textHeight) , FONT_HERSHEY_PLAIN, 0.8, charColor);
					persons[i].age = m_infos.ageInfos.ageArray[i];
				}
			}
			//�����Ա���Ϣ
			if (m_infos.genderInfos.genderArray != NULL)
			{
				for (int i = 0; i < m_infos.genderInfos.num; i++)
				{
					const signed int igender = m_infos.genderInfos.genderArray[i];
					const cv::String gender = std::string("gender:") + string( (igender == 0) ? "male" : ( (igender == 1) ? "female" : "not sure"));
					cv::putText(tobedraw, gender, cv::Point(vecPoint[i].x, vecPoint[i].y + textHeight * 2), FONT_HERSHEY_PLAIN, 0.8, charColor);
					persons[i].gender = m_infos.genderInfos.genderArray[i];
				}
			}
			//���ƽǶ���Ϣ
			if ((m_infos.angleInfos.roll != NULL) && (m_infos.angleInfos.yaw != NULL) && (m_infos.angleInfos.pitch != NULL) && (m_infos.angleInfos.status != NULL))
			{
				for (int i = 0; i < m_infos.angleInfos.num; i++)
				{
					const cv::String angle =
					(
						std::string("roll-")	+ std::to_string(m_infos.angleInfos.roll[i])	+ " " +
						std::string("yaw-")		+ std::to_string(m_infos.angleInfos.yaw[i])		+ " " +
						std::string("pitch-")	+ std::to_string(m_infos.angleInfos.pitch[i])	+ " " +
						std::string("status-")	+ std::to_string(m_infos.angleInfos.status[i])	+ " "
					).c_str();
					cv::putText(tobedraw, angle, cv::Point(vecPoint[i].x, vecPoint[i].y + textHeight * 3), FONT_HERSHEY_PLAIN, 0.8, charColor);
				}
			}
			//���ƻ�����Ϣ
			if (m_infos.rgbLivenessInfos.isLive != NULL)
			{
				for (int i = 0; i < m_infos.rgbLivenessInfos.num; i++)
				{
					cv::String liveness = "Liveness:no sure";
					if (m_infos.rgbLivenessInfos.isLive[i] == 0)
						liveness = "Liveness:Not";
					else if (m_infos.rgbLivenessInfos.isLive[i] == 1)
						liveness = "Liveness:True";

					cv::putText(tobedraw, liveness, cv::Point(vecPoint[i].x, vecPoint[i].y + textHeight * 3), FONT_HERSHEY_PLAIN, 0.8, charColor);
				}
			}

			//�����ź�
			m_SINGNAL_void_persons(persons);
		}

		/**
		* \@brief				�뱾���������ݽ��бȶ�
		* \@param confidence	ƥ������Ŷ�
		* \@param infor			��ƥ�䵽����˭
		*/
		void compareToLocalDataBase(float& confidence, SPersonInfor& infor)
		{

		}

		/**
		* \@brief
		* \@param src			����ͼ������
		* \@param dst			����ͼ������
		* \@param x				���
		* \@param y				�߶�
		*/
		void CutIplImage(IplImage* src, IplImage* dst, int x, int y)
		{
			CvSize size = cvSize(dst->width, dst->height);
			cvSetImageROI(src, cvRect(x, y, size.width, size.height));
			cvCopy(src, dst);
			cvResetImageROI(src);
		}

	public:
		//ArcVision������
		MHandle m_handleEngine;

		//����־��Ϣ
		struct SCheckFlag
		{
			std::atomic_bool initFalg;				//����������־
			std::atomic_bool detectFlag;			//����־(�����ϲ��õ�)
			std::atomic_bool age;					//����
			std::atomic_bool gender;				//�Ա�
			std::atomic_bool angle;					//3D�Ƕ�
			std::atomic_bool liveness;				//������Ϣ
			std::atomic_bool compare;				//�����ȶ�

			SCheckFlag()
				: initFalg(false), detectFlag(false), age(true), gender(true),
				angle(false), liveness(true), compare(true)
			{}
		};
		SCheckFlag m_checkFlags;

		//�����Ϣ����
		struct Sinfor
		{
			ASF_ActiveFileInfo fileInfo;			//�ļ���Ϣ
			ASF_SingleFaceInfo singleFaceInfos;		//��������������Ϣ
			ASF_FaceFeature	faceFeature;			//��ǰ����
			ASF_MultiFaceInfo multiFaceInfos;		//����λ����Ϣ
			ASF_AgeInfo ageInfos;					//������Ϣ��
			ASF_GenderInfo genderInfos;				//�Ա���Ϣ��
			ASF_Face3DAngle angleInfos;				//3D�Ƕ���Ϣ��
			ASF_LivenessInfo rgbLivenessInfos;		//RGB������Ϣ��
			ASF_LivenessInfo irLivenessInfos;		//IL������Ϣ��

			Sinfor()
			{
				fileInfo = { 0 };	singleFaceInfos = { 0 }; faceFeature = { 0 }; multiFaceInfos = { 0 };
				ageInfos = { 0 };	genderInfos = { 0 }; angleInfos = { 0 }; rgbLivenessInfos = { 0 };
				irLivenessInfos = { 0 };
			}
		};
		Sinfor m_infos;

		//��
		std::mutex m_mutex;

		//�źŲ�
		Ghost::signalslot::Signal<void(const std::vector<Ghost::SPersonInfor>&)> m_SINGNAL_void_persons;
		Ghost::signalslot::Slot m_SLOT_void_rects;

		//�汾��Ϣ
		const static string s_version;
		//����SDK app ID
		const static string s_ArcVisionAppID;
		//����SDK Key
		const static string s_ArcVisionKey;
	};

#if( _MSC_TOOLSET_VER_ == 140 )
#ifdef NDEBUG
	const string FaceRecognition::Impl::s_version = "vc140-r.0";
#else
	const string FaceRecognition::Impl::s_version = "vc140-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 141)
#ifdef NDEBUG
	const string FaceRecognition::Impl::s_version = "vc141-r.0";
#else
	const string FaceRecognition::Impl::s_version = "vc141-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 142)
#ifdef NDEBUG
	const string FaceRecognition::Impl::s_version = "vc142-r.0";
#else
	const string FaceRecognition::Impl::s_version = "vc142-d.0";
#endif
#endif

	const string FaceRecognition::Impl::s_ArcVisionAppID = "9Wi3M1eb6QN8rxraQsuXSgTTeex42goNbtCHCTgvve4z";
	const string FaceRecognition::Impl::s_ArcVisionKey = "8v36mvg9Ee4x9quV8sPcTqHMw2FukHvTWnnpCH2k5A3P";

	FaceRecognition::FaceRecognition()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("FaceRecognition::FaceRecognition::make_unique::Failured!!!");
		}
	}

	FaceRecognition::~FaceRecognition()
	{
		antiModual();
	}

	EResult FaceRecognition::setPath(const string& databasePath, const string& cfgPath, const string& weightPath) noexcept(true)
	{
		return EResult::SR_OK;
	}

	const string& FaceRecognition::getVersion() noexcept(true)
	{
		return FaceRecognition::Impl::s_version;
	}

	EResult FaceRecognition::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult FaceRecognition::saveModelParam()
	{
		return EResult::SR_OK;
	}

	EResult FaceRecognition::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult FaceRecognition::antiModual()
	{
		this->saveModelParam();

		return m_pImpl->antiModual();
	}

	EResult FaceRecognition::setModualParam(const EModualParamType type, const float value)
	{
		bool switchFlag = true;
		if (value < 0.001f && value > -0.001f)
			switchFlag = false;

		switch (type)
		{
		case EModualParamType::TYPE_FACE_RECONGNITION_Age:
		{
			m_pImpl->m_checkFlags.age.store(switchFlag);
			break;
		}
		case EModualParamType::TYPE_FACE_RECONGNITION_Sex:
		{
			m_pImpl->m_checkFlags.gender.store(switchFlag);
			break;
		}
		case EModualParamType::TYPE_FACE_RECONGNITION_3DAngle:
		{
			m_pImpl->m_checkFlags.angle.store(switchFlag);
			break;
		}
		case EModualParamType::TYPE_FACE_RECONGNITION_LivenessInfo:
		{
			m_pImpl->m_checkFlags.liveness.store(switchFlag);
			break;
		}
		default:
			break;
		}

		return EResult::SR_OK;
	}

	EResult FaceRecognition::detect(const cv::Mat& frameIn, cv::Mat& frameOut)
	{
		return m_pImpl->detect(frameIn, frameOut);
	}

	EDetectModual FaceRecognition::getModualType() noexcept(true)
	{
		return EDetectModual::HumanFace_Recognition_Modual;
	}

	EResult FaceRecognition::saveFaceToDataBase(const cv::Mat& frameSave, const SPersonInfor& infor)
	{
		return m_pImpl->saveFaceToDataBase(frameSave, infor);
	}

	void FaceRecognition::bindSlotFaceFind(const std::function<void(const std::vector<Ghost::SPersonInfor>&)>& func)
	{
		m_pImpl->m_SLOT_void_rects = m_pImpl->m_SINGNAL_void_persons.connect(func);
	}

}///namespace Ghost
