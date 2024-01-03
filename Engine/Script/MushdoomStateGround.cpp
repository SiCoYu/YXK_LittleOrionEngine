#include "MushdoomStateGround.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"

#include "Main/Application.h"
#include "Main/GameObject.h"

#include "Mushdoom.h"
#include "MushdoomBall.h"
#include "PlayerController.h"

MushdoomStateGround::MushdoomStateGround(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Ground");
}

void MushdoomStateGround::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Ground"))
	{
		enemy->animation->ActiveAnimation("ground");
	}
}

void MushdoomStateGround::OnStateUpdate()
{
}

void MushdoomStateGround::OnStateExit()
{
}
