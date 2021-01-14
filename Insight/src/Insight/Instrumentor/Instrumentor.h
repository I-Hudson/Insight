#pragma once
#include "Insight/Core/Log.h"
#include "Insight/Core/Utils.h"

#if defined(IS_DEBUG)
#if !defined(IS_PROFILE) && !defined(IS_PROFILE_OPTICK)
#ifdef _MSC_VER
#pragma message ("Debug should have profiling. 'IS_PROFILE' has been set as the default.")
#endif
#define IS_PROFILE
#endif
#endif

#if defined(IS_PROFILE) && defined(IS_PROFILE_OPTICK)
#ifdef _MSC_VER
#error 'IS_PROFILE' and 'IS_PROFILE_OPTICK' are defined. Only one can be defined.
#endif
#endif

#if defined(IS_PROFILE_OPTICK)
#include "optick.h"
#endif

#include <chrono>
#include <iomanip>
#include <thread>
#include <mutex>
#include <fstream>
#include <iosfwd>

#define INSIGHT_MAKE_CATEGORY(filter, color) ((Insight::Category::Type)(((uint64_t)(1ull) << (filter + 32)) | (uint64_t)color))

namespace Insight
{
	namespace Profile
	{
		using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

		struct ProfileResult
		{
			std::string Name;

			FloatingPointMicroseconds Start;
			std::chrono::microseconds ElapsedTime;
			std::thread::id ThreadID;
		};

		struct InstrumentationSession
		{
			std::string Name;
		};

		class Instrumentor
		{
		public:
			Instrumentor()
				: m_CurrentSession(nullptr)
			{ }
			~Instrumentor()
			{}

			Instrumentor(const Instrumentor&) = delete;
			Instrumentor(Instrumentor&&) = delete;

			void BeginSession()
			{
				IS_CORE_INFO("Temp profile started!");

				std::lock_guard lock(m_Mutex);
				
				if (&Get() != this)
				{
					Get().AddProfile(this);
				}

				if (m_CurrentSession)
				{
					// If there is already a current session, then close it before beginning new one.
					// Subsequent profiling output meant for the original session will end up in the
					// newly opened session instead.  That's better than having badly formatted
					// profiling output.
					if (Insight::Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
					{
						IS_CORE_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", "", m_CurrentSession->Name);
					}
					return;
				}
				m_CurrentSession = new InstrumentationSession({ "" });
				WriteHeader();
			}

			void EndSession()
			{
				std::lock_guard lock(m_Mutex);
				auto p = &Get();
				if (&Get() != this)
				{
					Get().RemoveProfile(this);
				}
				InternalEndSession();
			}

			void SaveSession(const std::string& filePath)
			{
				m_OutputStream.open(filePath);
				if (m_OutputStream.is_open())
				{
					m_OutputStream << m_dataString;
					m_OutputStream.close();
				}
				else
				{
					IS_CORE_ERROR("Profile could not be saved to file.");
				}
				IS_CORE_INFO("Profile has been saved.");
			}

			void WriteProfile(const ProfileResult& result)
			{
				std::stringstream json;

				json << std::setprecision(3) << std::fixed;
				json << ",{";
				json << "\"cat\":\"function\",";
				json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
				json << "\"name\":\"" << result.Name << "\",";
				json << "\"ph\":\"X\",";
				json << "\"pid\":0,";
				json << "\"tid\":" << result.ThreadID << ",";
				json << "\"ts\":" << result.Start.count();
				json << "}";

				std::lock_guard lock(m_Mutex);
				if (m_CurrentSession)
				{
					m_dataString += json.str();
				}

				for (auto& profile : m_profiles)
				{
					profile->m_dataString += json.str();
				}
			}

			void AddProfile(Instrumentor* ptr)
			{
				std::lock_guard lock(m_Mutex);
				m_profiles.push_back(ptr);
			}

			void RemoveProfile(Instrumentor* ptr)
			{
				std::lock_guard lock(m_Mutex);
				m_profiles.erase(std::find(m_profiles.begin(), m_profiles.end(), ptr));
			}

			static Instrumentor& Get()
			{
				static Instrumentor instance;
				return instance;
			}
		private:
			void WriteHeader()
			{
				m_dataString += "{\"otherData\": {},\"traceEvents\":[{}";
			}

			void WriteFooter()
			{
				m_dataString += "]}";
			}

			// Note: you must already own lock on m_Mutex before
			// calling InternalEndSession()
			void InternalEndSession()
			{
				if (m_CurrentSession)
				{
					WriteFooter();
					
					delete m_CurrentSession;
					m_CurrentSession = nullptr;
				}
				IS_INFO("Logging has finished. Session: {0}.", m_CurrentSession != nullptr);
			}
		private:
			std::mutex m_Mutex;
			InstrumentationSession* m_CurrentSession;
			std::ofstream m_OutputStream;
			std::string m_dataString;

			std::vector<Instrumentor*> m_profiles;
		};

		class InstrumentationTimer
		{
		public:
			InstrumentationTimer(const char* name)
				: m_Name(name), m_Stopped(false)
			{
				m_StartTimepoint = std::chrono::steady_clock::now();
			}

			~InstrumentationTimer()
			{
				if (!m_Stopped)
					Stop();
			}

			void Stop()
			{
				auto endTimepoint = std::chrono::steady_clock::now();
				auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
				auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

				Instrumentor::Get().WriteProfile({ m_Name, highResStart, elapsedTime, std::this_thread::get_id() });

				m_Stopped = true;
			}
		private:
			const char* m_Name;
			std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
			bool m_Stopped;
		};

		namespace InstrumentorUtils
		{

			template <size_t N>
			struct ChangeResult
			{
				char Data[N];
			};

			template <size_t N, size_t K>
			constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
			{
				ChangeResult<N> result = {};

				size_t srcIndex = 0;
				size_t dstIndex = 0;
				while (srcIndex < N)
				{
					size_t matchIndex = 0;
					while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
						matchIndex++;
					if (matchIndex == K - 1)
						srcIndex += matchIndex;
					result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
					srcIndex++;
				}
				return result;
			}
		}
	}

	// Source: http://msdn.microsoft.com/en-us/library/system.windows.media.colors(v=vs.110).aspx
	// Image:  http://i.msdn.microsoft.com/dynimg/IC24340.png
	struct Color
	{
		enum
		{
			Null = 0x00000000,
			AliceBlue = 0xFFF0F8FF,
			AntiqueWhite = 0xFFFAEBD7,
			Aqua = 0xFF00FFFF,
			Aquamarine = 0xFF7FFFD4,
			Azure = 0xFFF0FFFF,
			Beige = 0xFFF5F5DC,
			Bisque = 0xFFFFE4C4,
			Black = 0xFF000000,
			BlanchedAlmond = 0xFFFFEBCD,
			Blue = 0xFF0000FF,
			BlueViolet = 0xFF8A2BE2,
			Brown = 0xFFA52A2A,
			BurlyWood = 0xFFDEB887,
			CadetBlue = 0xFF5F9EA0,
			Chartreuse = 0xFF7FFF00,
			Chocolate = 0xFFD2691E,
			Coral = 0xFFFF7F50,
			CornflowerBlue = 0xFF6495ED,
			Cornsilk = 0xFFFFF8DC,
			Crimson = 0xFFDC143C,
			Cyan = 0xFF00FFFF,
			DarkBlue = 0xFF00008B,
			DarkCyan = 0xFF008B8B,
			DarkGoldenRod = 0xFFB8860B,
			DarkGray = 0xFFA9A9A9,
			DarkGreen = 0xFF006400,
			DarkKhaki = 0xFFBDB76B,
			DarkMagenta = 0xFF8B008B,
			DarkOliveGreen = 0xFF556B2F,
			DarkOrange = 0xFFFF8C00,
			DarkOrchid = 0xFF9932CC,
			DarkRed = 0xFF8B0000,
			DarkSalmon = 0xFFE9967A,
			DarkSeaGreen = 0xFF8FBC8F,
			DarkSlateBlue = 0xFF483D8B,
			DarkSlateGray = 0xFF2F4F4F,
			DarkTurquoise = 0xFF00CED1,
			DarkViolet = 0xFF9400D3,
			DeepPink = 0xFFFF1493,
			DeepSkyBlue = 0xFF00BFFF,
			DimGray = 0xFF696969,
			DodgerBlue = 0xFF1E90FF,
			FireBrick = 0xFFB22222,
			FloralWhite = 0xFFFFFAF0,
			ForestGreen = 0xFF228B22,
			Fuchsia = 0xFFFF00FF,
			Gainsboro = 0xFFDCDCDC,
			GhostWhite = 0xFFF8F8FF,
			Gold = 0xFFFFD700,
			GoldenRod = 0xFFDAA520,
			Gray = 0xFF808080,
			Green = 0xFF008000,
			GreenYellow = 0xFFADFF2F,
			HoneyDew = 0xFFF0FFF0,
			HotPink = 0xFFFF69B4,
			IndianRed = 0xFFCD5C5C,
			Indigo = 0xFF4B0082,
			Ivory = 0xFFFFFFF0,
			Khaki = 0xFFF0E68C,
			Lavender = 0xFFE6E6FA,
			LavenderBlush = 0xFFFFF0F5,
			LawnGreen = 0xFF7CFC00,
			LemonChiffon = 0xFFFFFACD,
			LightBlue = 0xFFADD8E6,
			LightCoral = 0xFFF08080,
			LightCyan = 0xFFE0FFFF,
			LightGoldenRodYellow = 0xFFFAFAD2,
			LightGray = 0xFFD3D3D3,
			LightGreen = 0xFF90EE90,
			LightPink = 0xFFFFB6C1,
			LightSalmon = 0xFFFFA07A,
			LightSeaGreen = 0xFF20B2AA,
			LightSkyBlue = 0xFF87CEFA,
			LightSlateGray = 0xFF778899,
			LightSteelBlue = 0xFFB0C4DE,
			LightYellow = 0xFFFFFFE0,
			Lime = 0xFF00FF00,
			LimeGreen = 0xFF32CD32,
			Linen = 0xFFFAF0E6,
			Magenta = 0xFFFF00FF,
			Maroon = 0xFF800000,
			MediumAquaMarine = 0xFF66CDAA,
			MediumBlue = 0xFF0000CD,
			MediumOrchid = 0xFFBA55D3,
			MediumPurple = 0xFF9370DB,
			MediumSeaGreen = 0xFF3CB371,
			MediumSlateBlue = 0xFF7B68EE,
			MediumSpringGreen = 0xFF00FA9A,
			MediumTurquoise = 0xFF48D1CC,
			MediumVioletRed = 0xFFC71585,
			MidnightBlue = 0xFF191970,
			MintCream = 0xFFF5FFFA,
			MistyRose = 0xFFFFE4E1,
			Moccasin = 0xFFFFE4B5,
			NavajoWhite = 0xFFFFDEAD,
			Navy = 0xFF000080,
			OldLace = 0xFFFDF5E6,
			Olive = 0xFF808000,
			OliveDrab = 0xFF6B8E23,
			Orange = 0xFFFFA500,
			OrangeRed = 0xFFFF4500,
			Orchid = 0xFFDA70D6,
			PaleGoldenRod = 0xFFEEE8AA,
			PaleGreen = 0xFF98FB98,
			PaleTurquoise = 0xFFAFEEEE,
			PaleVioletRed = 0xFFDB7093,
			PapayaWhip = 0xFFFFEFD5,
			PeachPuff = 0xFFFFDAB9,
			Peru = 0xFFCD853F,
			Pink = 0xFFFFC0CB,
			Plum = 0xFFDDA0DD,
			PowderBlue = 0xFFB0E0E6,
			Purple = 0xFF800080,
			Red = 0xFFFF0000,
			RosyBrown = 0xFFBC8F8F,
			RoyalBlue = 0xFF4169E1,
			SaddleBrown = 0xFF8B4513,
			Salmon = 0xFFFA8072,
			SandyBrown = 0xFFF4A460,
			SeaGreen = 0xFF2E8B57,
			SeaShell = 0xFFFFF5EE,
			Sienna = 0xFFA0522D,
			Silver = 0xFFC0C0C0,
			SkyBlue = 0xFF87CEEB,
			SlateBlue = 0xFF6A5ACD,
			SlateGray = 0xFF708090,
			Snow = 0xFFFFFAFA,
			SpringGreen = 0xFF00FF7F,
			SteelBlue = 0xFF4682B4,
			Tan = 0xFFD2B48C,
			Teal = 0xFF008080,
			Thistle = 0xFFD8BFD8,
			Tomato = 0xFFFF6347,
			Turquoise = 0xFF40E0D0,
			Violet = 0xFFEE82EE,
			Wheat = 0xFFF5DEB3,
			White = 0xFFFFFFFF,
			WhiteSmoke = 0xFFF5F5F5,
			Yellow = 0xFFFFFF00,
			YellowGreen = 0xFF9ACD32,
		};
	};

	struct Filter
	{
		enum Type : uint32_t
		{
			None,

			// CPU
			AI,
			Animation,
			Audio,
			Debug,
			Camera,
			Cloth,
			GameLogic,
			Input,
			Navigation,
			Network,
			Physics,
			Rendering,
			Scene,
			Script,
			Streaming,
			UI,
			VFX,
			Visibility,
			Wait,

			// IO
			IO,

			// GPU
			GPU_Cloth,
			GPU_Lighting,
			GPU_PostFX,
			GPU_Reflections,
			GPU_Scene,
			GPU_Shadows,
			GPU_UI,
			GPU_VFX,
			GPU_Water,
			GPU_CRITICAL
		};
	};

	struct Category
	{
		enum Type : uint64_t
		{
			// CPU
			None = INSIGHT_MAKE_CATEGORY(Filter::None, Color::White),
			AI = INSIGHT_MAKE_CATEGORY(Filter::AI, Color::Purple),
			Animation = INSIGHT_MAKE_CATEGORY(Filter::Animation, Color::LightSkyBlue),
			Audio = INSIGHT_MAKE_CATEGORY(Filter::Audio, Color::HotPink),
			Debug = INSIGHT_MAKE_CATEGORY(Filter::Debug, Color::Black),
			Camera = INSIGHT_MAKE_CATEGORY(Filter::Camera, Color::Black),
			Cloth = INSIGHT_MAKE_CATEGORY(Filter::Cloth, Color::DarkGreen),
			GameLogic = INSIGHT_MAKE_CATEGORY(Filter::GameLogic, Color::RoyalBlue),
			Input = INSIGHT_MAKE_CATEGORY(Filter::Input, Color::Ivory),
			Navigation = INSIGHT_MAKE_CATEGORY(Filter::Navigation, Color::Magenta),
			Network = INSIGHT_MAKE_CATEGORY(Filter::Network, Color::Olive),
			Physics = INSIGHT_MAKE_CATEGORY(Filter::Physics, Color::LawnGreen),
			Rendering = INSIGHT_MAKE_CATEGORY(Filter::Rendering, Color::BurlyWood),
			Scene = INSIGHT_MAKE_CATEGORY(Filter::Scene, Color::RoyalBlue),
			Script = INSIGHT_MAKE_CATEGORY(Filter::Script, Color::Plum),
			Streaming = INSIGHT_MAKE_CATEGORY(Filter::Streaming, Color::Gold),
			UI = INSIGHT_MAKE_CATEGORY(Filter::UI, Color::PaleTurquoise),
			VFX = INSIGHT_MAKE_CATEGORY(Filter::VFX, Color::SaddleBrown),
			Visibility = INSIGHT_MAKE_CATEGORY(Filter::Visibility, Color::Snow),
			Wait = INSIGHT_MAKE_CATEGORY(Filter::Wait, Color::Tomato),
			WaitEmpty = INSIGHT_MAKE_CATEGORY(Filter::Wait, Color::White),
			// IO
			IO = INSIGHT_MAKE_CATEGORY(Filter::IO, Color::Khaki),
			// GPU
			GPU_Cloth = INSIGHT_MAKE_CATEGORY(Filter::GPU_Cloth, Color::DarkGreen),
			GPU_Lighting = INSIGHT_MAKE_CATEGORY(Filter::GPU_Lighting, Color::Khaki),
			GPU_PostFX = INSIGHT_MAKE_CATEGORY(Filter::GPU_PostFX, Color::Maroon),
			GPU_Reflections = INSIGHT_MAKE_CATEGORY(Filter::GPU_Reflections, Color::CadetBlue),
			GPU_Scene = INSIGHT_MAKE_CATEGORY(Filter::GPU_Scene, Color::RoyalBlue),
			GPU_Shadows = INSIGHT_MAKE_CATEGORY(Filter::GPU_Shadows, Color::LightSlateGray),
			GPU_UI = INSIGHT_MAKE_CATEGORY(Filter::GPU_UI, Color::PaleTurquoise),
			GPU_VFX = INSIGHT_MAKE_CATEGORY(Filter::GPU_VFX, Color::SaddleBrown),
			GPU_Water = INSIGHT_MAKE_CATEGORY(Filter::GPU_Water, Color::SteelBlue),
			GPU_CRITICAL = INSIGHT_MAKE_CATEGORY(Filter::GPU_CRITICAL, Color::Red),
		};
	};
}

#ifdef IS_PROFILE
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define IS_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define IS_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define IS_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define IS_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define IS_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define IS_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define IS_FUNC_SIG __func__
#else
#define IS_FUNC_SIG "IS_FUNC_SIG unknown!"
#endif

#define IS_PROFILE_BEGIN_SESSION() ::Insight::Profile::Instrumentor::Get().BeginSession()
#define IS_PROFILE_END_SESSION() ::Insight::Profile::Instrumentor::Get().EndSession()
#define IS_PROFILE_SAVE_SESSION(filePath) ::Insight::Profile::Instrumentor::Get().SaveSession(filePath)
#define IS_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Insight::Profile::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Insight::Profile::InstrumentationTimer timer##line(fixedName##line.Data)

#define IS_PROFILE_FRAME(name) IS_PROFILE_SCOPE(name)
#define IS_PROFILE_SCOPE_LINE(name, line) IS_PROFILE_SCOPE_LINE2(name, line)
#define IS_PROFILE_SCOPE(name) IS_PROFILE_SCOPE_LINE(name, __LINE__)
#define IS_PROFILE_FUNCTION() IS_PROFILE_SCOPE(IS_FUNC_SIG)
#define IS_PROFILE_CATEGORY(name, category) 
#define IS_PROFILE_THREAD(threadName)
#define IS_PROFILE_TAG(name, ptr)
#define IS_PROFILE_STATE_CHANGED(callbackFunc)
#define IS_PROFILE_SET_MEMORY_ALLOCATOR(allocFunc, deAllocFunc, callbackFunc)

#define IS_PROFILE_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS)
#define IS_PROFILE_GPUI_SHUTDOWN()
#define IS_PROFILE_GPU_CONTEXT(commandList)
#define IS_PROFILE_GPU_FUNCTION(name)
#define IS_PROFILE_GPU_FLIP(swapchain)

#define IS_PROFILE_START_CAPTURE() ::Insight::Profile::Instrumentor temp; temp.BeginSession()
#define IS_PROFILE_STOP_CAPTURE() temp.EndSession()
#define IS_PROFILE_SAVE_CAPTURE(filePath) temp.SaveSession(CheckAndAppend(".json", filePath))

#elif defined(IS_PROFILE_OPTICK)
#define INSIGHT_PROFILE_CATEGORY_LINE(name, cat) Optick::Category::Type optickCat = (Optick::Category::Type)((uint32_t)cat); OPTICK_CATEGORY(name, optickCat)

#define IS_PROFILE_FRAME(name) OPTICK_FRAME(name)
#define IS_PROFILE_BEGIN_SESSION()
#define IS_PROFILE_END_SESSION() OPTICK_SHUTDOWN()
#define IS_PROFILE_SAVE_SESSION(filePath)
#define IS_PROFILE_SCOPE(name) INSIGHT_PROFILE_CATEGORY_LINE(name, Insight::Category::Type::None)
#define IS_PROFILE_SCOPE_CAT(name, cat) INSIGHT_PROFILE_CATEGORY_LINE(name, cat)
#define IS_PROFILE_FUNCTION() OPTICK_EVENT()
#define IS_PROFILE_CATEGORY(name, category) INSIGHT_PROFILE_CATEGORY_LINE(name, category)

#define IS_PROFILE_THREAD(threadName) OPTICK_THREAD(threadName)
#define IS_PROFILE_TAG(name, ptr) OPTICK_TAG(name, ptr)
#define IS_PROFILE_STATE_CHANGED(callbackFunc) OPTICK_SET_STATE_CHANGED_CALLBACK(callbackFunc)
#define IS_PROFILE_SET_MEMORY_ALLOCATOR(allocFunc, deAllocFunc, callbackFunc) OPTICK_SET_MEMORY_ALLOCATOR(allocFunc, deAllocFunc, callbackFunc)

#define IS_PROFILE_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS) OPTICK_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS);
#define IS_PROFILE_GPUI_SHUTDOWN() OPTICK_GPU_SHUTDOWN_VULKAN()
#define IS_PROFILE_GPU_CONTEXT(commandList) OPTICK_GPU_CONTEXT(commandList)
#define IS_PROFILE_GPU_FUNCTION(name) OPTICK_GPU_EVENT(name)
#define IS_PROFILE_GPU_FLIP(swapchain) OPTICK_GPU_FLIP(swapchain)

#define IS_PROFILE_START_CAPTURE() OPTICK_START_CAPTURE()
#define IS_PROFILE_STOP_CAPTURE() OPTICK_STOP_CAPTURE()
#define IS_PROFILE_SAVE_CAPTURE(filePath) OPTICK_SAVE_CAPTURE(CheckAndAppend(".opt", filePath).c_str())
#else
#define IS_PROFILE_FRAME(name)
#define IS_PROFILE_BEGIN_SESSION()
#define IS_PROFILE_END_SESSION()
#define IS_PROFILE_SAVE_SESSION(filePath)
#define IS_PROFILE_SCOPE(name)
#define IS_PROFILE_FUNCTION()
#define IS_PROFILE_CATEGORY(name, category)
#define IS_PROFILE_THREAD(threadName)
#define IS_PROFILE_TAG(name, ptr)
#define IS_PROFILE_STATE_CHANGED(callbackFunc)
#define IS_PROFILE_SET_MEMORY_ALLOCATOR(allocFunc, deAllocFunc, callbackFunc)

#define IS_PROFILE_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS)
#define IS_PROFILE_GPUI_SHUTDOWN()
#define IS_PROFILE_GPU_CONTEXT(commandList)
#define IS_PROFILE_GPU_FUNCTION(name)
#define IS_PROFILE_GPU_FLIP(swapchain)

#define IS_PROFILE_START_CAPTURE()
#define IS_PROFILE_STOP_CAPTURE()
#define IS_PROFILE_SAVE_CAPTURE()
#endif