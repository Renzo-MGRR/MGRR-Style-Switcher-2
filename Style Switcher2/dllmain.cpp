#include "gui.h"
#include <Trigger.h>
#include "imgui/imgui.h"
#include <Events.h>
#include <string>
#include <Pl0000.h>
#include <PlayerManagerImplement.h>
#include <cGameUIManager.h>
#include <shared.h>
#include "IniReader.h"
#include <format>
#include <Hooks.h>
#include <Windows.h>
using namespace std;
bool IsButtonPressed(const std::string& button)
{
	XINPUT_STATE state;
	DWORD dwResult = XInputGetState(0, &state);
	
	if (dwResult == ERROR_SUCCESS)
	{
		if (button == "A/CROSS" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0)
			return true;
		else if (button == "B/CIRCLE" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0)
			return true;
		else if (button == "X/SQUARE" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0)
			return true;
		else if (button == "Y/TRIANGLE" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0)
			return true;
		else if (button == "LB/L1" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0)
			return true;
		else if (button == "RB/R1" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0)
			return true;
		else if (button == "BACK/SELECT" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0)
			return true;
		else if (button == "START" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0)
			return true;
		else if (button == "LEFT_STICK_PRESS" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0)
			return true;
		else if (button == "RIGHT_STICK_PRESS" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0)
			return true;
		else if (button == "DPAD_UP" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
			return true;
		else if (button == "DPAD_DOWN" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
			return true;
		else if (button == "DPAD_LEFT" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
			return true;
		else if (button == "DPAD_RIGHT" && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
			return true;
		else if (button == "LEFT_STICK_UP" && state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			return true;
		else if (button == "LEFT_STICK_DOWN" && state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			return true;
		else if (button == "LEFT_STICK_LEFT" && state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			return true;
		else if (button == "LEFT_STICK_RIGHT" && state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			return true;
		else if (button == "RIGHT_STICK_UP" && state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			return true;
		else if (button == "RIGHT_STICK_DOWN" && state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			return true;
		else if (button == "RIGHT_STICK_LEFT" && state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			return true;
		else if (button == "RIGHT_STICK_RIGHT" && state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			return true;
		else if (button == "LT/L2" && state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			return true;
		else if (button == "RT/R2" && state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
			return true;
	}
	return false;
}
std::string ControllerButtons[] = { "A/CROSS", "B/CIRCLE", "X/SQUARE", "Y/TRIANGLE",
"LB/L1", "RB/R1", "BACK/SELECT", "START",
"LEFT_STICK_PRESS", "RIGHT_STICK_PRESS", "DPAD_UP", "DPAD_DOWN",
"DPAD_LEFT", "DPAD_RIGHT", "LEFT_STICK_UP", "LEFT_STICK_DOWN",
"LEFT_STICK_LEFT", "LEFT_STICK_RIGHT", "RIGHT_STICK_UP", "RIGHT_STICK_DOWN",
"RIGHT_STICK_LEFT", "LT/L2", "RT/R2" };
int StyleNumber = 0;
struct ChangeableStyle {
	std::map<int, std::string> AnimationMapping;
};
std::vector<ChangeableStyle> styles;
std::map<std::string, int> OriginalMapping;
static int AkPlaySound(const char* se, int a2)
{
	return ((int(__cdecl*)(const char* se, int a2))(shared::base + 0xA5E050))(se, a2);
}
void BackupStyle(AnimationMap* pAnimMap) {
	int AnimationsCount = pAnimMap->m_pUnits->size();

	ChangeableStyle style;
	style.AnimationMapping = std::map<int, std::string>();

	for (int i = 0; i < AnimationsCount; i++) {
		auto& unit = pAnimMap->getUnit(i);

		OriginalMapping.insert({ std::string(unit.m_name4), i });
		style.AnimationMapping.insert({ i, std::string(unit.m_name4) });
	}

	styles.push_back(style);
}
ChangeableStyle LoadStyle(std::string mappings) {
	std::istringstream stream(mappings);
	std::string animPair;
	ChangeableStyle style;
	style.AnimationMapping = std::map<int, std::string>();
	while (std::getline(stream, animPair, ',')) {
		auto dst = animPair.substr(0, 4);	// Animation to map onto
		auto replace = animPair.substr(5, 4);	// New animation to map

		if (OriginalMapping.contains(dst)) {
			style.AnimationMapping.insert({ OriginalMapping[dst], replace });
		}
	}
	return style;
}
static void StyleSwitch(AnimationMap* pAnimMap, int Number)
{
	int AnimationsCount = pAnimMap->m_pUnits->size();
	for (auto& animPair : styles[Number].AnimationMapping) {
		auto& unit = pAnimMap->getUnit(animPair.first);
		strncpy(unit.m_name4, animPair.second.c_str(), 4);
	}
}
int ParseKeyCode(const std::string& keyStr) {
	if (keyStr.empty())
		return 0;
	if (keyStr.rfind("0x", 0) == 0 || keyStr.rfind("0X", 0) == 0)
		return std::stoi(keyStr, nullptr, 16);
	return static_cast<unsigned char>(keyStr[0]);
}
bool KeepStyleAfterLoading = true;
int StartingStyle = 0;
int KeyToChangeStyle = 0;
int MenuKey = 0;
bool ResetFlags = false;
bool DisableMenu = false;
CIniReader iniReader("Styles.ini");
std::string ControllerButtonToChangeStyle = "";
std::string ControllerButtonToChangeStyle2 = "";
static void LoadSettings() noexcept
{	
	DisableMenu = iniReader.ReadBoolean("Settings", "DisableMenu", false);
	KeyToChangeStyle = ParseKeyCode(iniReader.ReadString("Settings", "KeyToChangeStyle", "5"));
	MenuKey = ParseKeyCode(iniReader.ReadString("Settings", "MenuKey", "K"));
	ControllerButtonToChangeStyle = iniReader.ReadString("Settings", "ControllerButtonToChangeStyle", "");
	ControllerButtonToChangeStyle2 = iniReader.ReadString("Settings", "ControllerButtonToChangeStyle2", "");
	StartingStyle = iniReader.ReadInteger("Styles", "StartAtStyle", 0);
	if (StartingStyle < 0)
	{
		StyleNumber = 0;
	}
	else
		StyleNumber = StartingStyle + 1;
}
class Plugin {
public:
	static inline void InitGUI() {
		Events::OnDeviceReset.before += gui::OnReset::Before;
		Events::OnDeviceReset.after += gui::OnReset::After;
		Events::OnEndScene += gui::OnEndScene;
	}
	Plugin() {
		InitGUI();
	}
} plugin;
bool antiloop = false;
std::string GetKeyName(int key)
{
	switch (key) {
	case VK_SPACE: return "SPACE";
	case VK_RETURN: return "ENTER";
	case VK_TAB: return "TAB";
	case VK_MENU: return "ALT";
	case VK_SHIFT: return "SHIFT";
	case VK_CONTROL: return "CTRL";
	default:
		return std::string(1, static_cast<char>(key));
	}
}
static bool waitingForKey = false;
bool ButtonPrompt(int id, bool StyleButton, bool Keyboard = true, bool PrimaryControllerButton = true)
{
	static std::unordered_map<int, bool> DotLabelMap;
	bool& DotLabel = DotLabelMap[id];
	bool result = false;
	std::string keyLabel = "";
	if (DotLabel)
		keyLabel = "...";
	else
	{
		if (StyleButton)
		{
			if (Keyboard)
				keyLabel = GetKeyName(KeyToChangeStyle);
			else if (PrimaryControllerButton)
				keyLabel = ControllerButtonToChangeStyle;
			else
				keyLabel = ControllerButtonToChangeStyle2;
		}
		else
			keyLabel = GetKeyName(MenuKey);
	}
	if (ImGui::Button(keyLabel.c_str())) {
		waitingForKey = true;
		DotLabel = true;
	}
	if (ImGui::IsKeyPressed(VK_ESCAPE) || IsButtonPressed("START")) {
		waitingForKey = false;
		DotLabel = false;
	}
	ImGuiIO& io = ImGui::GetIO();
	if (!ImGui::IsKeyReleased(VK_MENU))
		io.KeysDown[VK_MENU] = false;
	if (DotLabel) {
		if (Keyboard) 
		{
			for (int key = 0; key < 512; key++) {
				if (ImGui::IsKeyPressed(key) && key != MenuKey)
				{
					if (StyleButton)
					{
						KeyToChangeStyle = key;
					}
					else
					{
						MenuKey = key;
					}
					waitingForKey = false;
					DotLabel = false;
					result = true;
					break;
				}
			}
		}
		else
		{
			for (int CB = 0; CB < 23; CB++) {
				if (IsButtonPressed(ControllerButtons[CB]))
				{
					if (PrimaryControllerButton)
					{
						if (ControllerButtons[CB] != ControllerButtonToChangeStyle)
						{
							ControllerButtonToChangeStyle = ControllerButtons[CB];
						}
					}
					else if (ControllerButtons[CB] != ControllerButtonToChangeStyle2)
					{
						ControllerButtonToChangeStyle2 = ControllerButtons[CB];
					}
					waitingForKey = false;
					DotLabel = false;
					result = true;
					break;
				}
			}
		}
	}
	return result;
}
bool bShowGUI2 = false;
bool StyleSwitching = false;
CREATE_THISCALL(false, shared::base + 0x008104B0, void, Pl0000_UpdateInput, Pl0000*) {
	oPl0000_UpdateInput(pThis);
	if ((shared::IsKeyPressed(KeyToChangeStyle, false) ||
		(IsButtonPressed(ControllerButtonToChangeStyle) && IsButtonPressed(ControllerButtonToChangeStyle2)))
		&& !bShowGUI2)
	{
		if (!StyleSwitching)
		{
			StyleNumber++;
			if (StyleNumber > styles.size() - 1)
				StyleNumber = 0;
			if (StyleNumber == 0)
			{
				AkPlaySound("core_se_sys_decide_s", 0);
			}
			else
			{
				AkPlaySound("core_se_sys_decide_l", 0);
			}
			StyleSwitch(pThis->m_pAnimationMap, 0); // Restore original style
			StyleSwitch(pThis->m_pAnimationMap, StyleNumber); // Apply new style		
		}
		StyleSwitching = true;
	}
	else
		StyleSwitching = false;
}
bool StartingStyleApplied = false;
CREATE_THISCALL(false, shared::base + 0x00802660, void, Pl0000_startup, Pl0000*) {
	oPl0000_startup(pThis);
	PlayerManagerImplement::ms_Instance->getPlayerEntity();
	OriginalMapping = std::map<std::string, int>();
	styles = std::vector<ChangeableStyle>();
	OriginalMapping.clear();
	styles.clear();
	KeepStyleAfterLoading = iniReader.ReadBoolean("Settings", "KeepStyleAfterLoading", true);
	BackupStyle(pThis->m_pAnimationMap);
	for (int i = 0; true; i++) {
		std::string CurrentReading = iniReader.ReadString("RaidenStyles", std::format("Style{}", i), "");
		if (CurrentReading == "")
		{
			break;
		}
		else
		{
			styles.push_back(LoadStyle(CurrentReading));
		}
	}
	if (KeepStyleAfterLoading || StartingStyle >= 0)
	{
		if (StartingStyle == 0 && !StartingStyleApplied)
		{
			StyleSwitch(pThis->m_pAnimationMap, 1);
			StyleNumber = 1;
			StartingStyleApplied = true;
		}
		else
		{
			StyleSwitch(pThis->m_pAnimationMap, StyleNumber);
		}
	}
	else
		StyleNumber = 0;
}

void gui::RenderWindow()
{
	if (!antiloop)
	{
		LoadSettings();
		antiloop = true;
	}
	Pl0000* player = cGameUIManager::ms_Instance.m_pPlayer;
	if (ImGui::IsKeyPressed(MenuKey, false) && waitingForKey == false)
	{
		if (!DisableMenu)
		{
			bShowGUI2 ^= true;
		}
		else if (bShowGUI2)
		{
			bShowGUI2 = false;
		}
	}
	if (bShowGUI2)
	{
		ImGui::Begin("Menu");
		{
			if (ImGui::BeginTabBar("NOTITLE", ImGuiTabBarFlags_NoTooltip))
			{
				if (ImGui::BeginTabItem("Settings")) {
					ImGui::Value("Current Style", StyleNumber);
					ImGui::Value("Starting Style", StartingStyle);
					ImGui::Text("Style Key:");
					ImGui::SameLine();
					if (ButtonPrompt(1, true, true))
					{
						iniReader.WriteString("Settings", "KeyToChangeStyle", GetKeyName(KeyToChangeStyle));
					}
					ImGui::Text("Style Controller Button:");
					if (ButtonPrompt(2, true, false))
					{
						iniReader.WriteString("Settings", "ControllerButtonToChangeStyle", ControllerButtonToChangeStyle);
					}
					ImGui::Text("Style Controller Button 2:");
					if (ButtonPrompt(3, true, false, false))
					{
						iniReader.WriteString("Settings", "ControllerButtonToChangeStyle2", ControllerButtonToChangeStyle2);
					}
					ImGui::Text("Keep Style After Loading:");
					ImGui::SameLine();
					if (ImGui::Checkbox("##a", &KeepStyleAfterLoading)) 
					{
						iniReader.WriteBoolean("Settings", "KeepStyleAfterLoading", KeepStyleAfterLoading);
					}
					ImGui::Text("Menu Key:");
					ImGui::SameLine();
					if (ButtonPrompt(4, false, true))
					{
						iniReader.WriteString("Settings", "MenuKey", GetKeyName(MenuKey));
					}
					ImGui::Text("Disable this menu (can be re-enabled in Styles.ini):");
					ImGui::SameLine();
					if (ImGui::Checkbox("##b", &DisableMenu))
					{
						iniReader.WriteBoolean("Settings", "DisableMenu", DisableMenu);
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
		ResetFlags = true;
		Trigger::StpFlags.STP_MOUSE_UPDATE = 1;
		Trigger::StpFlags.STP_GAME_UPDATE = 1;
	}
	else
	{	
		waitingForKey = false;
		if (ResetFlags)
		{
			Trigger::StpFlags.STP_MOUSE_UPDATE = 0;
			Trigger::StpFlags.STP_GAME_UPDATE = 0;
			ResetFlags = false;
		}
	}
}
