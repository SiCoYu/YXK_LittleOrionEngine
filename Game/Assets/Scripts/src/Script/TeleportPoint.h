#ifndef  __TELEPORTPOINT_H__
#define  __TELEPORTPOINT_H__

#include "Script.h"
class TeleportPoint : public Script
{
public:

	TeleportPoint();
	~TeleportPoint() = default;

	void OnInspector(ImGuiContext*) override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;
	int zone_to_spawn = 0;

};

extern "C" SCRIPT_API TeleportPoint* TeleportPointDLL(); //This is how we are going to load the script

#endif