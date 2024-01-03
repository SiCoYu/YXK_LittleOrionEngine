#ifndef  __ATTACKS_H__
#define  __ATTACKS_H__

#include <string>
#include <map>
#include <vector>

enum class Move
{
	LIGHT,
	DOWN_KICK,
	HEAVY
};

enum class NodeID
{
	NO_COMBO,
	LIGHT_ONE,
	LIGHT_TWO,
	LIGHT_THREE,
	HEAVY_ONE,
	HEAVY_TWO,
	HEAVY_THREE,
	DOWN_SMASH
	
};

struct Node
{
	NodeID id;
	std::map<Move, NodeID> transitions;
	virtual void Init() = 0;
	int damage;
	float min_transition_animation_time = 0.0f;
	float max_transition_animation_time = 1.f;
	float cooldown_time_if_fail = 0.0f;
	float percentatge_of_collision;
	float percentatge_of_sfx = 0.0f;
	float attack_range_distance = 4.f;
	float attack_range_angle = 0.6f;
	std::string animation;
	std::string animation_state = "";
	//Velocity depending where to move (forward or backward)
	float distance;
	float speed;
	float knockback_power = 0.f; //Distance that the enemies will cover that will be multiplied by a divider on take dmg depending on the enemy
	bool is_aerial = false;
	std::string sfx_attack = "play_attack_player";
	std::vector<size_t> count_of_emit_vfx;
	std::vector<int> vfx_attack_indexes;
};

struct NoCombo : Node
{
	void Init() override
	{
		id = NodeID::NO_COMBO;
		transitions.insert(std::pair<Move, NodeID>(Move::LIGHT, NodeID::LIGHT_ONE));
		transitions.insert(std::pair<Move, NodeID>(Move::HEAVY, NodeID::HEAVY_ONE));
		transitions.insert(std::pair<Move, NodeID>(Move::DOWN_KICK, NodeID::DOWN_SMASH));
		damage = 0;
		animation = "idle";
		animation_state = "Idle";
		percentatge_of_collision = 0.99f;
		distance = 0;

		//SFX
		sfx_attack = "play_attack_player";
	}
};

struct LightOne : Node
{
	void Init() override
	{
		id = NodeID::LIGHT_ONE;
		transitions.insert(std::pair<Move, NodeID>(Move::LIGHT, NodeID::LIGHT_TWO));
		transitions.insert(std::pair< Move, NodeID>(Move::HEAVY, NodeID::HEAVY_ONE));
		damage = 10;
		min_transition_animation_time = 0.65f;
		max_transition_animation_time = 0.8f;
		animation = "light_one";
		animation_state = "LightOne";
		percentatge_of_collision = 0.10f;
		percentatge_of_sfx = 0.10f;
		attack_range_angle = 0.6f;
		attack_range_distance = 4.f;
		distance = 0;
		speed = 0;
		knockback_power = 5.f;

		//SFX
		sfx_attack = "play_attack_player";

		//VFX
		vfx_attack_indexes.push_back(3);
		vfx_attack_indexes.push_back(4);
	}
};

struct LightTwo : Node
{
	void Init() override
	{
		id = NodeID::LIGHT_TWO;
		transitions.insert(std::pair<Move, NodeID>(Move::LIGHT, NodeID::LIGHT_THREE));
		transitions.insert(std::pair<Move, NodeID>(Move::HEAVY, NodeID::HEAVY_ONE));
		damage = 10;
		min_transition_animation_time = 0.5f;
		max_transition_animation_time = 0.85f;
		animation = "light_two";
		animation_state = "LightTwo";
		percentatge_of_collision = 0.30f;
		percentatge_of_sfx = 0.10f;
		attack_range_angle = 0.6f;
		attack_range_distance = 3.f;
		knockback_power = 5.5f;

		//SFX
		distance = 2;
		speed = 10;

		//SFX
		sfx_attack = "play_attack_player";

		//VFX
		vfx_attack_indexes.push_back(3);
		vfx_attack_indexes.push_back(4);
		count_of_emit_vfx.push_back(12);
	}
};

struct LightThree : Node
{
	void Init() override
	{
		id = NodeID::LIGHT_THREE;
		transitions.insert(std::pair<Move, NodeID>(Move::HEAVY, NodeID::HEAVY_ONE));
		transitions.insert(std::pair<Move, NodeID>(Move::LIGHT, NodeID::LIGHT_ONE));
	
		damage = 20;
		min_transition_animation_time = 0.75f;
		percentatge_of_collision = 0.40f;
		percentatge_of_sfx = 0.20f;
		animation = "light_three";
		animation_state = "LightThree";
		attack_range_angle = 0.6f;
		attack_range_distance = 4.f;
		distance = 3;
		speed = 15;
		knockback_power = 9.f;

		//SFX
		sfx_attack = "play_backflip_player";

		//VFX
		vfx_attack_indexes.push_back(0);
		vfx_attack_indexes.push_back(1);
		count_of_emit_vfx.push_back(32);
	}
};

struct HeavyOne : Node
{
	void Init() override
	{
		id = NodeID::HEAVY_ONE;
		transitions.insert(std::pair<Move, NodeID>(Move::HEAVY, NodeID::HEAVY_TWO));
		damage = 40;
		min_transition_animation_time = 0.75f;
		animation = "heavy_one";
		animation_state = "HeavyOne";
		attack_range_angle = 0.5f;
		attack_range_distance = 3.5f;
		percentatge_of_collision = 0.60f;
		percentatge_of_sfx = 0.0f;
		distance = 0;
		speed = 0;
		knockback_power = 6.f;

		//SFX
		sfx_attack = "Play_H_Attack_1";

		//VFX
		vfx_attack_indexes.push_back(0);
		vfx_attack_indexes.push_back(1);
		vfx_attack_indexes.push_back(2);
		count_of_emit_vfx.push_back(22);
	}
};


struct HeavyTwo : Node
{
	void Init() override
	{
		id = NodeID::HEAVY_TWO;
		transitions.insert(std::pair<Move, NodeID>(Move::HEAVY, NodeID::HEAVY_THREE));
		damage = 50;
		min_transition_animation_time = 0.75f;
		percentatge_of_collision = 0.50f;
		percentatge_of_sfx = 0.50f;
		animation = "heavy_two";
		animation_state = "HeavyTwo";
		attack_range_angle = 0.6f;
		attack_range_distance = 3.5f;

		distance = 2;
		speed = 10;
		knockback_power = 7.f;
		//SFX
		sfx_attack = "Play_H_Attack_2";

		//VFX
		vfx_attack_indexes.push_back(0);
		vfx_attack_indexes.push_back(1);
		count_of_emit_vfx.push_back(22);
	}
};

struct HeavyThree : Node
{
	void Init() override
	{
		id = NodeID::HEAVY_THREE;
		damage = 65;
		min_transition_animation_time = 0.95f;
		percentatge_of_collision = 0.50f;
		percentatge_of_sfx = 0.45f;
		animation = "heavy_three";
		animation_state = "HeavyThree";
		attack_range_angle = 0.6f;
		attack_range_distance = 4.f;
		distance = 2.f;
		speed = 10.f;

		knockback_power = 15.f;

		//SFX
		sfx_attack = "Play_H_Attack_3";

		//VFX
		vfx_attack_indexes.push_back(7);
		vfx_attack_indexes.push_back(8);
		//count_of_emit_vfx.push_back(42);
	}
};

struct DownSmash : Node
{
	void Init() override
	{
		id = NodeID::DOWN_SMASH;
		damage = 40;
		min_transition_animation_time = 0.75f;
		percentatge_of_collision = 0.60f;
		percentatge_of_sfx = 0.30f;
		animation = "down_tail_smash";
		animation_state = "DownTailSmash";
		distance = 0;
		speed = -35.f;
		is_aerial = true;
		attack_range_distance = 2.f;

		//Sound
		sfx_attack = "play_backflip_player";

		//VFX
		vfx_attack_indexes.push_back(5);
		vfx_attack_indexes.push_back(6);
		count_of_emit_vfx.push_back(22);
	}
};

#endif
