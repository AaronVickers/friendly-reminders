#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

// CVars declaration
extern const std::string CVAR_SHOW_GOAL_MESSAGES;
extern const std::string CVAR_SHOW_GAME_FINISHED_MESSAGES;

extern const std::string CVAR_COMBINE_MESSAGES;

extern const std::string CVAR_PICK_MESSAGE_METHOD;
extern const std::string CVAR_DISPLAY_MESSAGE_METHOD;

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

class FriendlyReminders: public BakkesMod::Plugin::BakkesModPlugin
{
	// BakkesModPlugin methods
	virtual void onLoad();
	virtual void onUnload();

private:
	// CVars
	std::shared_ptr<bool> cvar_show_goal_messages;
	std::shared_ptr<bool> cvar_show_game_finished_messages;
	std::shared_ptr<bool> cvar_combine_messages;
	std::shared_ptr<std::string> cvar_pick_message_method;
	std::shared_ptr<std::string> cvar_display_message_method;
	std::shared_ptr<float> cvar_message_scale;
	std::shared_ptr<float> cvar_message_position_x;
	std::shared_ptr<float> cvar_message_position_y;
	std::shared_ptr<float> cvar_message_anchor_x;
	std::shared_ptr<float> cvar_message_anchor_y;

	// CVar post-process results
	std::vector<std::string> goalMessages = { "Drink some water!", "Check your posture!" };
	std::vector<std::string> gameFinishedMessages = { "Do some push-ups!", "Do some sit-ups!" };

	// Plugin flags
	bool isInMatch = false;
	bool isInGoalReplay = false;
	int goalMessageIndex = 0;
	int gameFinishedMessageIndex = 0;
	int combinedMessageIndex = 0;
	std::string currentMessage = "";
	int currentMessageIndex = 0;

	// Interface rendering
	void Render(CanvasWrapper);

	// Hooks
	void HookGoalScored();
	void HookGoalReplayBegin();
	void HookGoalReplayEnd();
	void HookCountdownBegin();
	void HookMatchEnded();
	void HookLeaveMatch();

	// Plugin methods
	void OnEvent(EventType);
	std::string GetNextMessage(EventType);
	void DisplayMessage(std::string&, float);

	// Utility methods
	void SplitString(std::string&, char, std::vector<std::string>&);
};
