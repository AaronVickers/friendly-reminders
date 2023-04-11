#include "pch.h"
#include "FriendlyReminders.h"

// Initialise plugin
BAKKESMOD_PLUGIN(FriendlyReminders, "Friendly Reminders", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

// CVar definitions
const std::string CVAR_SHOW_GOAL_MESSAGES = "show_goal_messages";
const std::string CVAR_SHOW_GAME_FINISHED_MESSAGES = "show_game_finished_messages";

const std::string CVAR_COMBINE_MESSAGES = "combine_messages";

const std::string CVAR_PICK_MESSAGE_METHOD = "pick_message_method";
const std::string CVAR_DISPLAY_MESSAGE_METHOD = "display_message_method";

const std::string CVAR_GOAL_MESSAGES = "goal_messages";
const std::string CVAR_GAME_FINISHED_MESSAGES = "game_finished_messages";

const std::string CVAR_MESSAGE_SCALE = "message_scale";
const std::string CVAR_MESSAGE_POSITION_X = "message_position_x";
const std::string CVAR_MESSAGE_POSITION_Y = "message_position_y";
const std::string CVAR_MESSAGE_ANCHOR_X = "message_anchor_x";
const std::string CVAR_MESSAGE_ANCHOR_Y = "message_anchor_y";

// Plugin load
void FriendlyReminders::onLoad()
{
	_globalCvarManager = cvarManager;

	// Register CVars
	cvar_show_goal_messages = std::make_shared<bool>(true);
	cvar_show_game_finished_messages = std::make_shared<bool>(true);
	cvar_combine_messages = std::make_shared<bool>(false);
	cvar_pick_message_method = std::make_shared<std::string>("Random");
	cvar_display_message_method = std::make_shared<std::string>("Default");
	cvar_message_scale = std::make_shared<float>(5.0f);
	cvar_message_position_x = std::make_shared<float>(0.5f);
	cvar_message_position_y = std::make_shared<float>(0.37f);
	cvar_message_anchor_x = std::make_shared<float>(0.5f);
	cvar_message_anchor_y = std::make_shared<float>(0.5f);

	// Enabled status (boolean)
	cvarManager->registerCvar(CVAR_SHOW_GOAL_MESSAGES, "1", "Show messages when a goal is scored", true, true, 0, true, 1, true)
		.bindTo(cvar_show_goal_messages);

	cvarManager->registerCvar(CVAR_SHOW_GAME_FINISHED_MESSAGES, "1", "Show messages when a game is finished", true, true, 0, true, 1, true)
		.bindTo(cvar_show_game_finished_messages);

	// Combine message lists (boolean)
	cvarManager->registerCvar(CVAR_COMBINE_MESSAGES, "0", "Use both lists of messages for both message events", true, true, 0, true, 1, true)
		.bindTo(cvar_combine_messages);

	// Message picking method (Random, Indexed)
	cvarManager->registerCvar(CVAR_PICK_MESSAGE_METHOD, "Random", "Method for how messages should be picked from the lists", false, false, 0, false, 0, true)
		.bindTo(cvar_pick_message_method);

	// Message display method (Default, Notification)
	cvarManager->registerCvar(CVAR_DISPLAY_MESSAGE_METHOD, "Default", "Method for how messages will be displayed on the screen", false, false, 0, false, 0, true)
		.bindTo(cvar_display_message_method);

	// Text scale of the message
	cvarManager->registerCvar(CVAR_MESSAGE_SCALE, "5", "Text scale of the message", true, true, 0, true, 10, true)
		.bindTo(cvar_message_scale);

	// X position of the message
	cvarManager->registerCvar(CVAR_MESSAGE_POSITION_X, "0.5", "X (horizontal) position of the message", true, true, 0, true, 1, true)
		.bindTo(cvar_message_position_x);

	// Y position of the message
	cvarManager->registerCvar(CVAR_MESSAGE_POSITION_Y, "0.37", "Y (vertical) position of the message", true, true, 0, true, 1, true)
		.bindTo(cvar_message_position_y);

	// X anchor point of the message
	cvarManager->registerCvar(CVAR_MESSAGE_ANCHOR_X, "0.5", "X (horizontal) anchor point of the message", true, true, 0, true, 1, true)
		.bindTo(cvar_message_anchor_x);

	// Y anchor point of the message
	cvarManager->registerCvar(CVAR_MESSAGE_ANCHOR_Y, "0.5", "Y (vertical) anchor point of the message", true, true, 0, true, 1, true)
		.bindTo(cvar_message_anchor_y);

	// Comma separated messages
	cvarManager->registerCvar(CVAR_GOAL_MESSAGES, "Drink some water!,Check your posture!", "Comma separated messages to be displayed when a goal is scored", true, false, 0, false, 0, true)
		.addOnValueChanged([this](std::string oldVal, CVarWrapper cvar)
			{
				std::string messages = cvar.getStringValue();

				goalMessages.clear();

				FriendlyReminders::SplitString(messages, ',', goalMessages);
			}
	);

	cvarManager->registerCvar(CVAR_GAME_FINISHED_MESSAGES, "Do some push-ups!,Do some sit-ups!", "Comma separated messages to be displayed when a game is finished", true, false, 0, false, 0, true)
		.addOnValueChanged([this](std::string oldVal, CVarWrapper cvar)
			{
				std::string messages = cvar.getStringValue();

				gameFinishedMessages.clear();

				FriendlyReminders::SplitString(messages, ',', gameFinishedMessages);
			}
	);

	// Register rendering
	gameWrapper->RegisterDrawable(std::bind(&FriendlyReminders::Render, this, std::placeholders::_1));

	// Register hooks
	gameWrapper->HookEventPost("Function TAGame.Ball_TA.Explode", std::bind(&FriendlyReminders::HookGoalScored, this));

	gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&FriendlyReminders::HookGoalReplayBegin, this));
	gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", std::bind(&FriendlyReminders::HookGoalReplayEnd, this));

	gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.Countdown.BeginState", std::bind(&FriendlyReminders::HookCountdownBegin, this));
	gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&FriendlyReminders::HookMatchEnded, this));

	gameWrapper->HookEventPost("Function TAGame.GFxShell_TA.LeaveMatch", std::bind(&FriendlyReminders::HookLeaveMatch, this));
}

// Canvas rendering
void FriendlyReminders::Render(CanvasWrapper canvas)
{
	Vector2 canvasSize = canvas.GetSize();

	float textScale = *cvar_message_scale * canvasSize.Y / 1080;

	Vector2F textPosition = { *cvar_message_position_x, *cvar_message_position_y };
	Vector2F textSize = canvas.GetStringSize(currentMessage, textScale, textScale);
	Vector2F anchorOffset = textSize * Vector2F{ *cvar_message_anchor_x, *cvar_message_anchor_y };

	canvas.SetColor(LinearColor{ 255, 255, 255, 255 });
	canvas.SetPosition(textPosition * canvasSize - anchorOffset);
	canvas.DrawString(currentMessage, textScale, textScale, true, false);
}

// Goal scored hook
void FriendlyReminders::HookGoalScored()
{
	// Check goal is not a goal replay
	if (isInGoalReplay) return;
	// Check goal is not after-match replay
	if (!isInMatch) return;
	// Check goal is not replay
	if (gameWrapper->IsInReplay()) return;

	// Fire goal scored event
	FriendlyReminders::OnEvent(EventType::GoalScored);
}

// Goal replay began hook
void FriendlyReminders::HookGoalReplayBegin()
{
	// Check goal replay hasn't already started
	if (!isInGoalReplay)
	{
		isInGoalReplay = true;
	}
}

// Goal replay ended hook
void FriendlyReminders::HookGoalReplayEnd()
{
	// Check goal replay hasn't already finished
	if (isInGoalReplay)
	{
		isInGoalReplay = false;
	}
}

// Countdown began hook
void FriendlyReminders::HookCountdownBegin()
{
	// Check match hasn't already started
	if (!isInMatch)
	{
		isInMatch = true;
	}
}

// Match ended hook
void FriendlyReminders::HookMatchEnded()
{
	// Check match hasn't already finished
	if (isInMatch)
	{
		isInMatch = false;

		// Fire match ended event
		FriendlyReminders::OnEvent(EventType::GameFinished);
	}
}

// Leave match hook
void FriendlyReminders::HookLeaveMatch()
{
	// Check match hasn't already finished
	if (isInMatch)
	{
		isInMatch = false;

		// Fire match ended event
		FriendlyReminders::OnEvent(EventType::GameFinished);
	}
}

// Method to handle message events
void FriendlyReminders::OnEvent(EventType eventType)
{
	// Check event happened in online game
	if (!gameWrapper->IsInOnlineGame()) return;

	// Return if event type is disabled
	if (eventType == EventType::GoalScored && *cvar_show_goal_messages.get() == false) return;
	if (eventType == EventType::GameFinished && *cvar_show_game_finished_messages.get() == false) return;

	// Get next message for even type
	std::string message = FriendlyReminders::GetNextMessage(eventType);

	// Display message
	FriendlyReminders::DisplayMessage(message, 4);
}

// Method to get message for event type
std::string FriendlyReminders::GetNextMessage(EventType eventType)
{
	size_t goalMessagesLength = goalMessages.size();
	size_t gameFinishedMessagesLength = gameFinishedMessages.size();
	size_t maxIndex = 0;
	int* messageIndex = 0;
	int displayMessageIndex = 0;

	// Calculate maximum message index
	if (*cvar_combine_messages.get() == true)
	{
		maxIndex = goalMessagesLength + gameFinishedMessagesLength;

		messageIndex = &combinedMessageIndex;
	}
	else
	{
		if (eventType == EventType::GoalScored)
		{
			maxIndex = goalMessagesLength;

			messageIndex = &goalMessageIndex;
		}
		else if (eventType == EventType::GameFinished)
		{
			maxIndex = gameFinishedMessagesLength;

			messageIndex = &gameFinishedMessageIndex;
		}
	}

	// Make sure there is a message
	if (maxIndex == 0) return "";

	// Determine next message index
	if (*cvar_pick_message_method.get() == "Random")
	{
		displayMessageIndex = rand() % maxIndex;
	}
	else if (*cvar_pick_message_method.get() == "Indexed")
	{
		if (*messageIndex >= maxIndex)
		{
			*messageIndex = 0;
		}

		displayMessageIndex = *messageIndex;

		(*messageIndex)++;
	}

	// Get message at message index
	if (*cvar_combine_messages.get() == true)
	{
		if (displayMessageIndex < goalMessagesLength)
		{
			return goalMessages.at(displayMessageIndex);
		}
		else
		{
			return gameFinishedMessages.at(displayMessageIndex - goalMessagesLength);
		}
	}
	else
	{
		if (eventType == EventType::GoalScored)
		{
			return goalMessages.at(displayMessageIndex);
		}
		else if (eventType == EventType::GameFinished)
		{
			return gameFinishedMessages.at(displayMessageIndex);
		}
	}

	// No message
	return "";
}

// Method to display message to user
void FriendlyReminders::DisplayMessage(std::string& message, float displayTime)
{
	currentMessageIndex++;

	// Check display method
	if (*cvar_display_message_method.get() == "Default")
	{
		currentMessage = message;

		int thisMessageIndex = currentMessageIndex;

		// Clear message if it hasn't changed
		gameWrapper->SetTimeout(
			[this, thisMessageIndex](GameWrapper* gameWrapper)
			{
				if (currentMessageIndex != thisMessageIndex) return;

				currentMessage = "";
			},
			displayTime
		);
	}
	else if (*cvar_display_message_method.get() == "Notification")
	{
		// Get notifications enabled state
		bool notificiationsEnabled = cvarManager->getCvar("cl_notifications_enabled_beta").getBoolValue();

		// Enable notifications if currently disabled
		if (!notificiationsEnabled)
		{
			cvarManager->executeCommand("cl_notifications_enabled_beta 1");
		}

		// Display notification
		gameWrapper->Toast("Friendly Reminder", message, "default", displayTime);

		// Restore notifications enabled state
		if (!notificiationsEnabled)
		{
			cvarManager->executeCommand("cl_notifications_enabled_beta 0");
		}
	}
}

// String split method
void FriendlyReminders::SplitString(std::string& str, char delim, std::vector<std::string>& resultVector)
{
	std::istringstream iss(str);
	std::string subStr;

	while (std::getline(iss, subStr, delim))
	{
		resultVector.emplace_back(subStr);
	}
}
