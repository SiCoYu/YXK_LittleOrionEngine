#ifndef  __UIMANAGER_H__
#define  __UIMANAGER_H__

#include "Script.h"
#include "ComboCounter.h"

class ComboCounterIndicator;
class DamageIndicatorSpawner;
class InputManager;
class PauseMenuController;
class ProgressBar;
class SpecialProgressBar;
class ComponentImage;
class UILives;

class UIManager : public Script
{
public:
	UIManager();
	~UIManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;



	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;

	void SetSecondPlayerUI();

	void SetPlayer1Health(float percentage);
	void SetPlayer2Health(float percentage);

	void SetPlayer1SpecialPoints(float percentage);
	void SetPlayer2SpecialPoints(float percentage);

	void SetComboCounterProgress(float combo_counter_number_progress);
	void SetComboCounterLevel(ComboCounter::ComboCounterLevel  combo_counter_level);
	void HideComboCounter();

	void SpawnDamageIndicator(int damage, const float3& receiver_position, const float3& agressor_position);

	void ShowReviveButton(const float3& position);
	GameObject * SelectReviveButton();

	void IncreaseLives();
	void DecreaseLives();

	void HideReviveButton();

	void SetWinScreen();
	void SetLoseScreen();

	void SetPauseScreen(bool open);
	void ShowMenu(GameObject* menu);
	void ShakePlayerHud(GameObject* hud, const float2 starting_position);
	void ShowDamageOverlay(float percentage);
	float RandomRange(float a, float b);

	void FillLightballUI(unsigned int player, float progress) const;
	void EmptyLightballUI(bool multiplayer) const;
	void ShowPlayersLightballUI(bool multiplayer) const;
	void HideLightballUI(bool multiplayer) const;

	void UpdateShieldlUI(unsigned int player, float durability) const;

private:
	InputManager* input_manager = nullptr;

	GameObject* second_player_hud = nullptr;

	GameObject* win_screen = nullptr;
	GameObject* lose_screen = nullptr;

	GameObject* pause_menu_controller_go = nullptr;
	PauseMenuController* pause_menu_controller = nullptr;

	GameObject* revive_button_keyboard = nullptr;
	GameObject* revive_button_controller = nullptr;
	GameObject* selected_revive_button = nullptr;

	GameObject* player1_health_progress_bar_game_object = nullptr;
	ProgressBar* player1_health_progress_bar = nullptr;

	GameObject* player1_special_progress_bar_game_object = nullptr;
	SpecialProgressBar* player1_special_progress_bar = nullptr;

	GameObject* player2_health_progress_bar_game_object = nullptr;
	ProgressBar* player2_health_progress_bar = nullptr;

	GameObject* player2_special_progress_bar_game_object = nullptr;
	SpecialProgressBar* player2_special_progress_bar = nullptr;

	GameObject* player1_hud = nullptr;
	GameObject* player2_hud = nullptr;

	GameObject* damage_overlay = nullptr;

	float2 player1_hud_position = float2::zero;
	float2 player2_hud_position = float2::zero;

	GameObject* damage_indicator_spawner_game_object = nullptr;
	DamageIndicatorSpawner* damage_indicator_spawner = nullptr;

	GameObject* player1_light_ball_ui_game_object = nullptr;
	ProgressBar* player1_light_ball_ui = nullptr;

	GameObject* player2_light_ball_ui_game_object = nullptr;
	ProgressBar* player2_light_ball_ui = nullptr;

	GameObject* player1_shield_ui_game_object = nullptr;
	ProgressBar* player1_shield_ui = nullptr;

	GameObject* player2_shield_ui_game_object = nullptr;
	ProgressBar* player2_shield_ui = nullptr;

	ComboCounterIndicator* combo_counter_indicator = nullptr;

	UILives* lives_ui = nullptr;

	bool transitioning = false;
};
extern "C" SCRIPT_API UIManager* UIManagerDLL(); //This is how we are going to load the script
#endif
