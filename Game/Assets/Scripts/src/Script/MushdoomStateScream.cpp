#include "MushdoomStateScream.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentParticleSystem.h"

#include "Main/GameObject.h"

#include "Mushdoom.h"
#include "MushdoomBall.h"
#include "PlayerController.h"
#include "PlayerMovement.h"

MushdoomStateScream::MushdoomStateScream(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy(state_name, "Scream");
}

void MushdoomStateScream::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Scream"))
	{
		enemy->animation->ActiveAnimation("scream");
	}

	if (enemy->current_target != nullptr)
	{
		enemy->RotateTowardsPlayer();

		if (enemy->throw_effect)
		{
			enemy->throw_effect->active = true;
			enemy->throw_effect->Play();

		}
	}
	enemy->audio_source->PlayEvent("play_cry");
}

void MushdoomStateScream::OnStateUpdate()
{
	enemy->RotateTowardsPlayer();
	if (enemy->animation->GetCurrentClipPercentatge() >= animation_end)
	{
		Exit(enemy->throw_state);
	}
}

void MushdoomStateScream::OnStateExit()
{
}
