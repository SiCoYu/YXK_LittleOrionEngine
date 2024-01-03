#include "MushdoomStateHit.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentScript.h"

#include "Main/GameObject.h"

#include "Mushdoom.h"
#include "MushdoomBall.h"
#include "PlayerAttack.h"
#include "PlayerController.h"

MushdoomStateHit::MushdoomStateHit(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Hit");
}

void MushdoomStateHit::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Hit"))
	{
		enemy->animation->ActiveAnimation("hity");
	}
}

void MushdoomStateHit::OnStateUpdate()
{
	enemy->KnockBack();

	if (enemy->animation->GetCurrentClipPercentatge() >= total_percentatge_hitted)
	{
		Exit(enemy->pursue_state);
	}
}

void MushdoomStateHit::OnStateExit()
{
}
