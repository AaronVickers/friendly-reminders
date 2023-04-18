#include "pch.h"
#include "FriendlyReminders.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"

std::string FriendlyReminders::GetPluginName()
{
	return PLUGIN_NAME;
}

void FriendlyReminders::RenderSettings()
{
	ImGui::Text("Hello, world!");
}