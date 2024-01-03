#ifndef  __DIALOGUEMANAGER_H__
#define  __DIALOGUEMANAGER_H__

#include "Script.h"

#include "Dialogues.h"

#include <queue>
#include <functional>

class ComponentImage;
class ComponentText;
class InputManager;
class WorldManager;
struct Player;

class DialogueManager : public Script
{
public:
	typedef std::function<void(DialogueFeeling)> NextDialogueCallback;
	DialogueManager() = default;
	~DialogueManager();

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void StartDialogue(DialogueID,const NextDialogueCallback& callback = nullptr);

	bool IsOnDialogue() const;

	GameObject* ShowPopup(const std::string popup_to_show);
	void DisablePopup(GameObject* popup_to_disable);
	bool IsPopupAvailable();

private:
	bool ValidPopup(const std::string popup_to_show);
	bool IsDialogue(DialogueID);
	
	void DisplayNextSentence();
	void UpdateText();

	void GetDialogueList();
	void InitDialogueList();
	void InitPopupList();

	void EnsurePlayersTouchingTheGround() const;

	float2 GetCameraCoordinates(const float3 & position);
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;
	
private:

	WorldManager* world_manager = nullptr;
	InputManager* input_manager = nullptr;
	//Player's references
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;

	ComponentText* dialogue_text = nullptr;
	std::queue<Sentence> sentences;
	std::vector<Dialogue*> dialogue_list;
	Dialogue* current_dialogue = nullptr;
	bool on_dialogue = false;
	bool end_of_sentence = false;
	Sentence sentence_to_show;
	std::string::size_type string_position = 0;
	float cooldown_spawn = 30.f;
	float spawn_time = 100.f;

	//PopUps
	std::vector<GameObject*> pop_up_list;
	GameObject* selected_pop_up = nullptr;
	bool showing_popup = false;

	//Listener
	NextDialogueCallback next_dialogue_callback;

};

extern "C" SCRIPT_API DialogueManager* DialogueManagerDLL(); //This is how we are going to load the script

#endif