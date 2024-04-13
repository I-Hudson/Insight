#pragma once

#include "Graphics/Defines.h"
#include "Core/Singleton.h"

#ifdef RENDER_DOC_API
#include "renderdoc_app.h"
#endif

#include <atomic>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		class IS_GRAPHICS RenderDocAPI : public Core::Singleton<RenderDocAPI>
		{
			enum class CaptureState
			{
				None,
				Requested,
				Running,
				Ended
			};

		public:
			RenderDocAPI();
			~RenderDocAPI();

			bool Initialise();
			bool IsInitialised() const;

			void EnableOverlay();
			void DisableOverlay();

			void Capture();
			bool IsCapturing() const;

		private:
			bool CaptureRequested() const;
			void StartCapture();
			void EndCapture();

		private:
			bool m_initialised = false;
			std::atomic<CaptureState> m_captureState = CaptureState::None;

#ifdef RENDER_DOC_API
			void* m_renderDocDll = nullptr;
			RENDERDOC_API_1_1_2*  m_renderDocAPI = nullptr;
#endif

			friend class RenderContext;
		};
	}
}