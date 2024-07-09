#include "Graphics/RenderDocAPI.h"

#include "Core/Logger.h"

#include "Core/CommandLineArgs.h"
#include "Platforms/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		constexpr const char* renderdocapi_enabled = "renderdocapi_enabled";
		constexpr bool c_OverrideRenderDocAPIEnabled = false;

		RenderDocAPI::RenderDocAPI()
		{ }

		RenderDocAPI::~RenderDocAPI()
		{
			if (m_initialised)
			{
#ifdef RENDER_DOC_API
				m_initialised = false;
				m_captureState = CaptureState::None;

				m_renderDocAPI->RemoveHooks();

				Platform::FreeDynamicLibrary(m_renderDocDll);
				m_renderDocAPI = nullptr;
				m_renderDocDll = nullptr;
#endif
			}
		}

		bool RenderDocAPI::Initialise()
		{
			if (m_initialised)
			{
				return true;
			}

			if (!c_OverrideRenderDocAPIEnabled && !Core::CommandLineArgs::GetCommandLineValue(renderdocapi_enabled)->GetBool())
			{
				IS_LOG_CORE_WARN("[RenderDocAPI::Initialise] Unable to Initialise as '{}' cmd is not true.", renderdocapi_enabled);
				return false;
			}

#ifdef RENDER_DOC_API
			m_renderDocDll = Platform::LoadDynamicLibrary("renderdoc.dll");
			if (!m_renderDocDll)
			{
				IS_LOG_CORE_ERROR("[RenderDocAPI::Initialise] Unable to load renderdoc.dll.");
				return false;
			}

			pRENDERDOC_GetAPI getApiFunc =
				Platform::GetDynamicFunction<int, RENDERDOC_Version, void**>(m_renderDocDll, "RENDERDOC_GetAPI");
			if (!getApiFunc)
			{
				IS_LOG_CORE_ERROR("[RenderDocAPI::Initialise] Unable to get RenderDoc's 'GetAPI' function from renderdoc.dll.");
				return false;
			}


			int getAPIResult = getApiFunc(eRENDERDOC_API_Version_1_1_2, (void**)&m_renderDocAPI);
			if (getAPIResult != 1)
			{
				IS_LOG_CORE_ERROR("[RenderDocAPI::Initialise] RenderDoc GetAPI returned '{}', either the RenderDoc vesion is not supported or invalid argurments.", getAPIResult);
				return false;
			}

			m_renderDocAPI->SetCaptureFilePathTemplate("RenderDoc Captures/cap");
#endif
			m_initialised = true;
			DisableOverlay();
			return m_initialised;
		}

		bool RenderDocAPI::IsInitialised() const
		{
			return m_initialised;
		}

		void RenderDocAPI::EnableOverlay()
		{
			if (m_initialised)
			{
#ifdef RENDER_DOC_API
				const u32 bits = RENDERDOC_OverlayBits::eRENDERDOC_Overlay_Enabled
					| RENDERDOC_OverlayBits::eRENDERDOC_Overlay_Default;
				m_renderDocAPI->MaskOverlayBits(0, bits);
#endif
			}
		}

		void RenderDocAPI::DisableOverlay()
		{
			if (m_initialised)
			{
#ifdef RENDER_DOC_API
				const u32 bits = ~RENDERDOC_OverlayBits::eRENDERDOC_Overlay_Enabled;
				m_renderDocAPI->MaskOverlayBits(bits, 0);
#endif
			}
		}

		void RenderDocAPI::Capture()
		{
			if (m_initialised
				&& m_captureState == CaptureState::None)
			{
				m_captureState = CaptureState::Requested;
			}
		}

		bool RenderDocAPI::IsCapturing() const
		{
			u32 frameCapturing = 0;
			if (m_captureState == CaptureState::Running && m_initialised)
			{
#ifdef RENDER_DOC_API
				frameCapturing = m_renderDocAPI->IsFrameCapturing();
#endif
			}
			return frameCapturing == 1;
		}

		bool RenderDocAPI::CaptureRequested() const
		{
			return m_captureState == CaptureState::Requested;
		}

		void RenderDocAPI::StartCapture()
		{
			if (m_captureState == CaptureState::Requested && m_initialised)
			{
#ifdef RENDER_DOC_API
				m_renderDocAPI->StartFrameCapture(NULL, NULL);
#endif
				m_captureState = CaptureState::Running;
			}
		}

		void RenderDocAPI::EndCapture()
		{
			if (m_captureState == CaptureState::Running && m_initialised)
			{
#ifdef RENDER_DOC_API
				const u32 captureSuccessful = m_renderDocAPI->EndFrameCapture(NULL, NULL);
				if (captureSuccessful == 0)
				{
					IS_LOG_CORE_ERROR("[RenderDocAPI::EndCapture] RenderDoc capture unsuccessful.");
				}
				m_renderDocAPI->LaunchReplayUI(1, "");
#endif
				m_captureState = CaptureState::None;

			}
		}
	}
}
