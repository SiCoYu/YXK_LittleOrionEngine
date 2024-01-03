#ifndef  __ENEMYSTATE_H__
#define  __ENEMYSTATE_H__

template <typename T>
class EnemyState
{
public:
	EnemyState() {}
	EnemyState(T* enemy) : enemy(enemy) {}
	virtual ~EnemyState() = default;

	virtual void OnStateEnter() {}
	virtual void OnStateUpdate() {}
	virtual void OnStateExit() {}

	void Exit(EnemyState* state)
	{
		enemy->animation->Stop();
		enemy->previous_state = enemy->current_state;
		enemy->current_state->OnStateExit();
		enemy->current_state = state;
		enemy->current_state->OnStateEnter();
		enemy->animation->Play();
	}

public:
	char state_name[16] = "None";

protected:
	T* const enemy = nullptr;

	float animation_end = 0.90f;
};

#endif