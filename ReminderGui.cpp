#include "pch.h"
#include "FriendlyReminders.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"

std::string FriendlyReminders::GetMenuName()
{
	return PLUGIN_MENU_NAME;
}

std::string FriendlyReminders::GetMenuTitle()
{
	return PLUGIN_NAME;
}

bool FriendlyReminders::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

bool FriendlyReminders::IsActiveOverlay()
{
	return false;
}

void FriendlyReminders::OnOpen(){}

void FriendlyReminders::OnClose(){}

void FriendlyReminders::Render()
{
	if (!menuFont) {
		auto gui = gameWrapper->GetGUIManager();
		menuFont = gui.GetFont("Ubuntu-Bold-72");
	}

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize;

	std::string text = currentMessage;

	if (menuFont)
	{
		menuFont->Scale = 16.0f / 72.0f * *message_scale;
		ImGui::PushFont(menuFont);
	}

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
	ImGui::SetNextWindowPos(ImVec2(
		displaySize.x * *message_position_x - textSize.x * *message_anchor_x,
		displaySize.y * *message_position_y - textSize.y * *message_anchor_y
	));

	ImGui::Begin(PLUGIN_MENU_NAME.c_str(), (bool*)1, windowFlags);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);

	ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
	ImGui::Text(text.c_str());

	ImGui::PopStyleVar(2);

	if (menuFont)
	{
		ImGui::PopFont();
	}

	ImGui::End();
}
