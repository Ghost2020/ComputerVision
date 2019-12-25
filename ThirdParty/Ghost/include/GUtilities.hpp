/*

+	Description:            Utilities
+	FileName:               GUtilities.hpp
+	Author:                 Ghost Chen
+   Date:                   2019/7/3

+	Copyright(C)            Quantum Dynamics Lab.
+

*/
#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <string>

using namespace std;

namespace Ghost
{
	/**
	* \@brief Type of result of function execution
	*/
	enum struct EResult : uint8_t
	{
		SR_OK = 0,

		SR_NG,

		SR_Folder_Not_Exist,
		SR_Data_File_Not_Exist,
		SR_Cfg_File_Not_Exist,
		SR_Weight_File_Not_Exist,
		SR_Model_Path_Not_Exist,

		SR_Prototxt_Path_Not_Exist,
		SR_Caffe_Model_Path_Not_Exist,

		SR_Image_Empty,
		SR_Detector_Memory_Allocation_Failed,
		SR_Detector_Not_Exist,
		SR_Detector_Already_Exist,
		SR_Data_Path_Not_Set,
		SR_Saved_Data_Does_Not_Exist,

		SR_ASF_Activation_Failed,
		SR_ASF_Get_Active_FileInfo_Fail,
		SR_ASF_Init_Engine_Fail,
		SR_ASF_UnInit_Engine_Fail,
		SR_ASF_Engine_Init_Already,
		SR_ASF_Engine_Handle_NULL,
		SR_ASF_Engine_Not_Init,
		SR_ASF_Face_Feature_Extraction_Failed,
		SR_ASF_Get_Age_Failed,
		SR_ASF_Get_Gender_Failed,
		SR_ASF_Get_Face3DAngle_Failed,
		SR_ASF_Get_LivenessScore_Failed,

		SR_ASF_IdCard_Feature_Extraction_Failed,
		SR_ASF_Face_IdCard_Compare_Failed,

		SR_UNDEFINE = 100
	};

	/**
	* \@brief Detection modual Type
	*/
	enum struct EDetectModual : uint8_t
	{
		Object_Detection_Modual = 0,				//物体检测
		Pose_Detection_Modual,						//人体pose检测
		HumanFace_Detection_Modual,					//人脸位置检测
		HumanFace_Compare_Modual,					//人脸身份检测
		HumanFace_Recognition_Modual,				//人脸属性检测
		HumanFace_LandMark,

		Undefine = 20
	};

	/**
	* \@brief Camera parameter type
	*/
	enum struct ECameraParamType : uint8_t
	{
		CAP_PROP_POS_MSEC = 0, //!< Current position of the video file in milliseconds.
		CAP_PROP_POS_FRAMES = 1, //!< 0-based index of the frame to be decoded/captured next.
		CAP_PROP_POS_AVI_RATIO = 2, //!< Relative position of the video file: 0=start of the film, 1=end of the film.
		CAP_PROP_FRAME_WIDTH = 3, //!< Width of the frames in the video stream.
		CAP_PROP_FRAME_HEIGHT = 4, //!< Height of the frames in the video stream.
		CAP_PROP_FPS = 5, //!< Frame rate.
		CAP_PROP_FOURCC = 6, //!< 4-character code of codec. see VideoWriter::fourcc .
		CAP_PROP_FRAME_COUNT = 7, //!< Number of frames in the video file.
		CAP_PROP_FORMAT = 8, //!< Format of the %Mat objects returned by VideoCapture::retrieve().
		CAP_PROP_MODE = 9, //!< Backend-specific value indicating the current capture mode.
		CAP_PROP_BRIGHTNESS = 10, //!< Brightness of the image (only for cameras).
		CAP_PROP_CONTRAST = 11, //!< Contrast of the image (only for cameras).
		CAP_PROP_SATURATION = 12, //!< Saturation of the image (only for cameras).
		CAP_PROP_HUE = 13, //!< Hue of the image (only for cameras).
		CAP_PROP_GAIN = 14, //!< Gain of the image (only for cameras).
		CAP_PROP_EXPOSURE = 15, //!< Exposure (only for cameras).
		CAP_PROP_CONVERT_RGB = 16, //!< Boolean flags indicating whether images should be converted to RGB.
		CAP_PROP_WHITE_BALANCE_BLUE_U = 17, //!< Currently unsupported.
		CAP_PROP_RECTIFICATION = 18, //!< Rectification flag for stereo cameras (note: only supported by DC1394 v 2.x backend currently).
		CAP_PROP_MONOCHROME = 19,
		CAP_PROP_SHARPNESS = 20,
		CAP_PROP_AUTO_EXPOSURE = 21, //!< DC1394: exposure control done by camera, user can adjust reference level using this feature.
		CAP_PROP_GAMMA = 22,
		CAP_PROP_TEMPERATURE = 23,
		CAP_PROP_TRIGGER = 24,
		CAP_PROP_TRIGGER_DELAY = 25,
		CAP_PROP_WHITE_BALANCE_RED_V = 26,
		CAP_PROP_ZOOM = 27,
		CAP_PROP_FOCUS = 28,
		CAP_PROP_GUID = 29,
		CAP_PROP_ISO_SPEED = 30,
		CAP_PROP_BACKLIGHT = 32,
		CAP_PROP_PAN = 33,
		CAP_PROP_TILT = 34,
		CAP_PROP_ROLL = 35,
		CAP_PROP_IRIS = 36,
		CAP_PROP_SETTINGS = 37, //! Pop up video/camera filter dialog (note: only supported by DSHOW backend currently. Property value is ignored)
		CAP_PROP_BUFFERSIZE = 38,
		CAP_PROP_AUTOFOCUS = 39
	};

	/**
	* \@brief Modual parameter type
	*/
	enum struct EModualParamType : uint8_t
	{
		TYPE_XXX = 0,

		TYPE_Face_Detection_MinFaceSize,			//最小脸的大小
		TYPE_Face_Detection_MaxFaceSize,			//最大脸的大小
		TYPE_Face_Detection_ScoreThresh,			//识别为脸的阈值
		TYPE_Face_Detection_ImagePyramidScaleFactor,//图像金字塔
		TYPE_Face_Detection_WindowStep,				//窗口

		TYPE_FACE_RECONGNITION_Age,					//获取年龄
		TYPE_FACE_RECONGNITION_Sex,					//获取性别
		TYPE_FACE_RECONGNITION_3DAngle,				//3D角度
		TYPE_FACE_RECONGNITION_LivenessInfo,		//活体信息

		TYPE_POSE_Dtection_Pose,					//pose设置相关
		TYPE_POSE_Dtection_Face,					//face相关
		TYPE_POSE_Dtection_Hand,					//Hand相关
		TYPE_POSE_Dtection_Extra,					//额外相关
		TYPE_POSE_Dtection_Output,					//输出相关
		TYPE_POSE_Dtection_Gui,						//显示相关

		TYPE_UNDEFINE = 100
	};

	/**
	* \@brief 网络数据传递的数据类型
	*/
	enum struct FaceGoodDataMode : uint8_t
	{
		Bone = 0,
		Facial,
		BoneFacial,
		Control,
		Ios_Facial,
		Voice_Drive,
		Max,
	};

	enum struct EEmotion : uint8_t
	{
		
	};

	/**
	* \@brief 权重计算类型
	*/
	enum struct FaceGoodAlgrithmeType : uint8_t
	{
		Sampling = 0,
		MachineLearning,

		Undefine = 1000
	};

	/**
	* \@语音服务那边传递过来的状态信息，UE4 需要根据对应的状态来做反应
	*/
	enum struct FaceGoodCharacterState : uint8_t
	{
		Idel = 0,
		Wakeup_Start = 1,
		Wakeup_Finish = 3,
		Asr_Start = 4,
		Asr_Finish = 6,
		TextChat_Start = 7,
		TextChat_Finish = 9,
		TTS_Start = 10,
		TTS_End = 12,
		Request_Send_Start = 13,
		Request_Send_Finish = 15,
		Another_State
	};

	/**
	* \@brief personla information for local face database
	*/
	struct SPersonalInformation
	{
		size_t ID;									//ID	全局唯一识别信息
		string name;								//名字	可同名
		uint8_t gender;								//性别	0::未识别性别 1::男性 2::女性 
		uint8_t age;								//年龄
		uint8_t post;								//职位  0::公司最高职位 职位等级依次递减

		SPersonalInformation()
			:
			ID(0), name(""), gender(0), age(0), post(127)
		{}
	};

	struct SRect
	{

	};

	

	namespace signal
	{
		//---------------------------------------------------------------------
	// bind_member
	//---------------------------------------------------------------------
		template<class Return, class Type, class... Args>
		std::function<Return(Args...)> bind_member(Type* instance, Return(Type::* method)(Args...))
		{
			return [=](Args&& ... args) -> Return
			{
				return (instance->*method)(std::forward<Args>(args)...);
			};
		}

		//---------------------------------------------------------------------
		// SignalImpl
		//---------------------------------------------------------------------
		template<class SlotImplType>
		class SignalImpl
		{
		public:
			std::vector<std::weak_ptr<SlotImplType>> slots;
		};


		//---------------------------------------------------------------------
		// SlotImpl
		//---------------------------------------------------------------------
		class SlotImpl
		{
		public:
			SlotImpl() {}

			virtual ~SlotImpl() {}

			SlotImpl(const SlotImpl&) = delete;
			SlotImpl& operator= (const SlotImpl&) = delete;
		};


		//---------------------------------------------------------------------
		// SlotImplT
		//---------------------------------------------------------------------
		template<class FuncType>
		class SlotImplT : public SlotImpl
		{
		public:
			SlotImplT(const std::weak_ptr<SignalImpl<SlotImplT>>& signal, const std::function<FuncType>& callback)
				: signal(signal)
				, callback(callback)
			{
			}

			~SlotImplT()
			{
				std::shared_ptr<SignalImpl<SlotImplT>> sig = signal.lock();
				if (sig == nullptr) return;

				for (auto it = sig->slots.begin(); it != sig->slots.end(); ++it) {
					if (it->expired() || it->lock().get() == this) {
						it = sig->slots.erase(it);
						if (it == sig->slots.end()) {
							break;
						}
					}
				}
			}

			std::weak_ptr<SignalImpl<SlotImplT>> signal;
			std::function<FuncType> callback;
		};

		//---------------------------------------------------------------------
		// Slot
		//---------------------------------------------------------------------
		class Slot
		{
		public:
			Slot() {}

			~Slot() {}

			template<class T>
			explicit Slot(T impl) : impl(impl) {}

			operator bool() const
			{
				return static_cast<bool>(impl);
			}

		private:
			std::shared_ptr<SlotImpl> impl;
		};


		//---------------------------------------------------------------------
		// Signal
		//---------------------------------------------------------------------
		template<class FuncType>
		class Signal
		{
		public:
			Signal() : impl(std::make_shared<SignalImpl<SlotImplT<FuncType>>>()) {}

			template<class... Args>
			void operator()(Args&& ... args)
			{
				std::vector<std::weak_ptr<SlotImplT<FuncType>>> slotVector = impl->slots;
				for (std::weak_ptr<SlotImplT<FuncType>>& weak_slot : slotVector)
				{
					std::shared_ptr<SlotImplT<FuncType>> slot = weak_slot.lock();
					if (slot)
						slot->callback(std::forward<Args>(args)...);
				}
			}

			Slot connect(const std::function<FuncType>& func)
			{
				std::shared_ptr<SlotImplT<FuncType>> slotImpl = std::make_shared<SlotImplT<FuncType>>(impl, func);

				/* 由于 SignalImpl 使用的是 std::weak_ptr，push_back 操作不会增加引用计数。
				   因此，如果调用函数 connect 后的返回值没有赋值给 Slot 对象，过了这个函数的
				   作用域 slotImpl 对象就会被释放掉 */
				impl->slots.push_back(slotImpl);

				return Slot(slotImpl);
			}

			template<class InstanceType, class MemberFuncType>
			Slot connect(InstanceType instance, MemberFuncType func)
			{
				return connect(bind_member(instance, func));
			}

		private:
			std::shared_ptr<SignalImpl<SlotImplT<FuncType>>> impl;
		};
	}///namespace signal

}/// namespace facegood