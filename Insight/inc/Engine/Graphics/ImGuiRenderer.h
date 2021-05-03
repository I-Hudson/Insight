#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Templates/TSingleton.h"

class ImGuiRenderer : public TSingleton<ImGuiRenderer>
{
public:
	ImGuiRenderer();
	virtual ~ImGuiRenderer();

	virtual void Init() = 0;
	virtual void NewFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void Render() = 0;

	static ImGuiRenderer* New();
	bool IsInit() const { return m_init; }

protected:
	bool m_init = false;
};