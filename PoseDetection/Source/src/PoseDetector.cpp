#include "PoseDetector.h"

#include <atomic>
#include <filesystem>
#include <mutex>

#define OPENPOSE_FLAGS_DISABLE_PRODUCER
#define OPENPOSE_FLAGS_DISABLE_DISPLAY

#include <openpose/flags.hpp>
#include <openpose/headers.hpp>

using namespace op;
namespace fs = std::experimental::filesystem;
using namespace std;
using namespace facegood::signalslot;

// Display
DEFINE_bool(no_display, false, "Enable to disable the visual display.");

namespace facegood
{
	class PoseDetector::Impl
	{
	public:
		Impl()
			:
			m_pDetector(nullptr)
		{}

		~Impl()
		{
			antiModual();
		}

	public:
		/**
		* \@brief Initialization of Detection Environment
		* \@return Returns the result of execution
		*/
		EResult initModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_flags.initFlag.load())
				return EResult::SR_Detector_Already_Exist;

			m_pDetector = std::make_unique<op::Wrapper>(op::ThreadManagerMode::Asynchronous);	//!!!!!!!!!!!
			if (m_pDetector == nullptr)
				return EResult::SR_Detector_Memory_Allocation_Failed;

			this->configure();

			m_pDetector->start();

			m_flags.initFlag.store(true);

			return EResult::SR_OK;
		}

		/**
		* \@brief Anti-initialization of Detection Environment
		* \@return Returns the result of execution
		*/
		EResult antiModual()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if ((!m_flags.initFlag.load()))
				return EResult::SR_Detector_Not_Exist;

			if (m_pDetector)
			{
				m_pDetector.reset();
				m_pDetector = nullptr;
			}

			m_flags.initFlag.store(false);

			return EResult::SR_OK;
		}

		/**
		* \@brief
		* \@return Returns the result of execution
		*/
		EResult configure()
		{
			// logging_level
			op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
				__LINE__, __FUNCTION__, __FILE__);
			op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
			op::Profiler::setDefaultX(FLAGS_profile_speed);

			// Applying user defined configuration - GFlags to program variables
			// outputSize
			const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
			// netInputSize
			const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "-1x368");
			// faceNetInputSize
			const auto faceNetInputSize = op::flagsToPoint(FLAGS_face_net_resolution, "368x368 (multiples of 16)");
			// handNetInputSize
			const auto handNetInputSize = op::flagsToPoint(FLAGS_hand_net_resolution, "368x368 (multiples of 16)");
			// poseMode
			const auto poseMode = op::flagsToPoseMode(FLAGS_body);
			// poseModel
			const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
			// JSON saving
			if (!FLAGS_write_keypoint.empty())
				op::log("Flag `write_keypoint` is deprecated and will eventually be removed." " Please, use `write_json` instead.", op::Priority::Max);
			// keypointScaleMode
			const auto keypointScaleMode = op::flagsToScaleMode(FLAGS_keypoint_scale);
			// heatmaps to add
			const auto heatMapTypes = op::flagsToHeatMaps(FLAGS_heatmaps_add_parts, FLAGS_heatmaps_add_bkg,
				FLAGS_heatmaps_add_PAFs);
			const auto heatMapScaleMode = op::flagsToHeatMapScaleMode(FLAGS_heatmaps_scale);
			// >1 camera view?
			const auto multipleView = (FLAGS_3d || FLAGS_3d_views > 1);
			// Face and hand detectors
			const auto faceDetector = op::flagsToDetector(FLAGS_face_detector);
			const auto handDetector = op::flagsToDetector(FLAGS_hand_detector);
			// Enabling Google Logging
			const bool enableGoogleLogging = false;

			// Pose configuration (use WrapperStructPose{} for default and recommended configuration)
			const op::WrapperStructPose wrapperStructPose
			{
				poseMode, netInputSize, outputSize, keypointScaleMode, FLAGS_num_gpu, FLAGS_num_gpu_start,
				FLAGS_scale_number, (float)FLAGS_scale_gap, op::flagsToRenderMode(FLAGS_render_pose, multipleView),
				poseModel, !FLAGS_disable_blending, (float)FLAGS_alpha_pose, (float)FLAGS_alpha_heatmap,
				FLAGS_part_to_show, PoseDetector::Impl::s_modelPath, heatMapTypes, heatMapScaleMode, FLAGS_part_candidates,
				(float)FLAGS_render_threshold, FLAGS_number_people_max, FLAGS_maximize_positives, FLAGS_fps_max,
				PoseDetector::Impl::s_prototxtName, PoseDetector::Impl::s_caffeModelName, (float)FLAGS_upsampling_ratio, enableGoogleLogging
			};
			m_pDetector->configure(wrapperStructPose);

			// Face configuration (use op::WrapperStructFace{} to disable it)
			const op::WrapperStructFace wrapperStructFace
			{
				false, faceDetector, faceNetInputSize,
				op::flagsToRenderMode(FLAGS_face_render, multipleView, FLAGS_render_pose),
				(float)FLAGS_face_alpha_pose, (float)FLAGS_face_alpha_heatmap, (float)FLAGS_face_render_threshold
			};
			m_pDetector->configure(wrapperStructFace);

			// Hand configuration (use op::WrapperStructHand{} to disable it)
			const op::WrapperStructHand wrapperStructHand
			{
				false, handDetector, handNetInputSize, FLAGS_hand_scale_number, (float)FLAGS_hand_scale_range,
				op::flagsToRenderMode(FLAGS_hand_render, multipleView, FLAGS_render_pose), (float)FLAGS_hand_alpha_pose,
				(float)FLAGS_hand_alpha_heatmap, (float)FLAGS_hand_render_threshold
			};
			m_pDetector->configure(wrapperStructHand);

			// Extra functionality configuration (use op::WrapperStructExtra{} to disable it)
			const op::WrapperStructExtra wrapperStructExtra
			{
				false, FLAGS_3d_min_views, FLAGS_identification, FLAGS_tracking, FLAGS_ik_threads
			};
			m_pDetector->configure(wrapperStructExtra);

			// Output (comment or use default argument to disable any output)
			const op::WrapperStructOutput wrapperStructOutput
			{
				FLAGS_cli_verbose, FLAGS_write_keypoint, op::stringToDataFormat(FLAGS_write_keypoint_format),
				FLAGS_write_json, FLAGS_write_coco_json, FLAGS_write_coco_json_variants, FLAGS_write_coco_json_variant,
				FLAGS_write_images, FLAGS_write_images_format, FLAGS_write_video, FLAGS_write_video_fps,
				FLAGS_write_video_with_audio, FLAGS_write_heatmaps, FLAGS_write_heatmaps_format, FLAGS_write_video_3d,
				FLAGS_write_video_adam, FLAGS_write_bvh, FLAGS_udp_host, FLAGS_udp_port
			};
			m_pDetector->configure(wrapperStructOutput);

			// GUI
			const op::WrapperStructGui wrapperStructGui
			{
				DisplayMode::NoDisplay, false, false
			};
			m_pDetector->configure(wrapperStructGui);

			// No GUI. Equivalent to: opWrapper.configure(op::WrapperStructGui{});
			// Set to single-thread (for sequential processing and/or debugging and/or reducing latency)

			return EResult::SR_OK;
		}

		/**
		* \@brief Detect the Object  #warning if frameShow is tempty Explains that the detected object does not need to be drawn#
		* \@param frameIn::Image for detection
		* \@param frameShow::Need to draw detected objects #may be emtpy!!!#
		*/
		EResult detect(const cv::Mat& frameIn, cv::Mat& frameOut)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_flags.initFlag.load())
				EResult::SR_Detector_Not_Exist;

			auto datumProcessed = m_pDetector->emplaceAndPop(frameIn);

			frameOut = datumProcessed->at(0)->cvOutputData;

			return EResult::SR_OK;
		}

	public:
		//pose检测对象
		std::unique_ptr<op::Wrapper> m_pDetector;

		struct SFlags
		{
			std::atomic_bool initFlag;				//初始化标志
			std::atomic_bool poseFlag;				//pose 标志
			std::atomic_bool faceFlag;				//face 标志
			std::atomic_bool handFlag;				//hand 标志
			std::atomic_bool extraFlag;				//extra 标志
			std::atomic_bool outputFlag;			//output 标志

			SFlags() :
				initFlag(false), poseFlag(true), faceFlag(false),
				handFlag(false), extraFlag(false), outputFlag(false)
			{}
		};
		SFlags m_flags;

		//信号槽
		facegood::signalslot::Signal<void(const std::vector<facegood::SPoint2D>&)> m_SIGNAL_void_points2D;
		facegood::signalslot::Slot m_SLOT_void_points2D;

		//锁
		std::mutex m_mutex;

		//model文件夹
		static string s_modelPath;
		static string s_prototxtName;
		static string s_caffeModelName;

		//版本
		const static string s_version;
	};

	string PoseDetector::Impl::s_modelPath = "";
	string PoseDetector::Impl::s_prototxtName = "";
	string PoseDetector::Impl::s_caffeModelName = "";

#if( _MSC_TOOLSET_VER_ == 140 )
#ifdef NDEBUG
	const string PoseDetector::Impl::s_version = "vc140-r.0";
#else
	const string PoseDetector::Impl::s_version = "vc140-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 141)
#ifdef NDEBUG
	const string PoseDetector::Impl::s_version = "vc141-r.0";
#else
	const string PoseDetector::Impl::s_version = "vc141-d.0";
#endif
#elif(_MSC_TOOLSET_VER_ == 142)
#ifdef NDEBUG
	const string PoseDetector::Impl::s_version = "vc142-r.0";
#else
	const string PoseDetector::Impl::s_version = "vc142-d.0";
#endif
#endif

	PoseDetector::PoseDetector()
		:
		m_pImpl(std::make_unique<Impl>())
	{
		if (m_pImpl == nullptr)
		{
			throw std::exception("PoseDetector::PoseDetector::make_unique::failured!!!");
		}
	}

	PoseDetector::~PoseDetector()
	{
		antiModual();
	}

	EResult PoseDetector::loadModualParam(const string& modualLoadPath)
	{
		return EResult::SR_OK;
	}

	EResult  PoseDetector::saveModelParam()
	{
		return EResult::SR_OK;
	}

	EResult PoseDetector::initModual()
	{
		return m_pImpl->initModual();
	}

	EResult PoseDetector::antiModual()
	{
		this->saveModelParam();

		return m_pImpl->antiModual();
	}

	EResult PoseDetector::setModualParam(const EModualParamType type, const float value)
	{
		return EResult::SR_OK;
	}

	EResult PoseDetector::detect(const cv::Mat& frameIn, cv::Mat& frameOut)
	{
		return m_pImpl->detect(frameIn, frameOut);
	}

	EDetectModual PoseDetector::getModualType() noexcept(true)
	{
		return EDetectModual::Pose_Detection_Modual;
	}

	void PoseDetector::bindSlotPoseFind(const std::function<void(const std::vector<facegood::SPoint2D>&)>& func)
	{
		m_pImpl->m_SLOT_void_points2D = m_pImpl->m_SIGNAL_void_points2D.connect(func);
	}

	EResult PoseDetector::setPath(const string& modelsFolderPath, const string& prototxtName, const string& caffeModelName) noexcept(true)
	{
		if (!fs::exists(modelsFolderPath))
			return EResult::SR_Model_Path_Not_Exist;

		if (!fs::exists(modelsFolderPath + "pose_deploy.prototxt"))
			return EResult::SR_Prototxt_Path_Not_Exist;

		if (!fs::exists(modelsFolderPath + "pose_iter_584000.caffemodel"))
			return EResult::SR_Caffe_Model_Path_Not_Exist;

		PoseDetector::Impl::s_modelPath = modelsFolderPath;
		PoseDetector::Impl::s_prototxtName = prototxtName;
		PoseDetector::Impl::s_caffeModelName = caffeModelName;

		return EResult::SR_OK;
	}

	const string& PoseDetector::getVersion() noexcept(true)
	{
		return PoseDetector::Impl::s_version;
	}
}///namespace facegood