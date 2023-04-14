#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

// CVars declaration
extern const std::string CVAR_SHOW_GOAL_MESSAGES;
extern const std::string CVAR_SHOW_GAME_FINISHED_MESSAGES;

extern const std::string CVAR_COMBINE_MESSAGES;

extern const std::string CVAR_PICK_MESSAGE_METHOD;
extern const std::string CVAR_DISPLAY_MESSAGE_METHOD;
extern const std::string CVAR_DISPLAY_METHOD_CONFIGURABLE;

extern const std::string CVAR_GOAL_MESSAGES;
extern const std::string CVAR_GAME_FINISHED_MESSAGES;

extern const std::string CVAR_MESSAGE_SCALE;
extern const std::string CVAR_MESSAGE_POSITION_X;
extern const std::string CVAR_MESSAGE_POSITION_Y;
extern const std::string CVAR_MESSAGE_ANCHOR_X;
extern const std::string CVAR_MESSAGE_ANCHOR_Y;

enum class EventType
{
	GoalScored,
	GameFinished
};

class FriendlyReminders: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
	// BakkesModPlugin methods
	virtual void onLoad();
	virtual void onUnload();

private:
	// CVars
	std::shared_ptr<bool> show_goal_messages;
	std::shared_ptr<bool> show_game_finished_messages;
	std::shared_ptr<bool> combine_messages;
	std::shared_ptr<std::string> pick_message_method;
	std::shared_ptr<std::string> display_message_method;
	std::shared_ptr<float> message_scale;
	std::shared_ptr<float> message_position_x;
	std::shared_ptr<float> message_position_y;
	std::shared_ptr<float> message_anchor_x;
	std::shared_ptr<float> message_anchor_y;

	// CVar post-process results
	std::vector<std::string> goalMessages = { "Drink some water!", "Check your posture!" };
	std::vector<std::string> gameFinishedMessages = { "Do some push-ups!", "Do some sit-ups!" };

	// Plugin flags
	bool pluginLoaded = false;
	bool isInMatch = false;
	bool isInGoalReplay = false;
	int goalMessageIndex = 0;
	int gameFinishedMessageIndex = 0;
	int combinedMessageIndex = 0;
	std::string currentMessage = "";
	int currentMessageIndex = 0;

	// Canvas interface rendering
	void RenderCanvas(CanvasWrapper);

	// Hooks
	void HookGoalScored();
	void HookGoalReplayBegin();
	void HookGoalReplayEnd();
	void HookCountdownBegin();
	void HookMatchEnded();
	void HookLeaveMatch();

	// Plugin methods
	void UpdateDisplayMethod();
	void OnEvent(EventType);
	std::string GetNextMessage(EventType);
	void DisplayExampleMessage(bool);
	void DisplayMessage(std::string&, float);

	// Utility methods
	void SplitString(std::string&, char, std::vector<std::string>&);

	// ImGui interface rendering
	ImFont* menuFont;
	std::string menuName = "friendlyreminders";

	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;
	void Render() override;
};
