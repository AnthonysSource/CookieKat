#pragma once

#include <CookieKat/Engine/Entities/IWorldDefinition.h>

class Game : public CKE::IWorldDefinition
{
public:
	void LoadWorldResources(ResourceSystem& res) override;
	void PopulateWorld(CKE::EntityDatabase& admin, CKE::EntitySystem* system) override;

private:
	void RegisterAllComponents(CKE::EntityDatabase& db);

	void World_StressTest(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
	void World_CerberusPBR(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
	void World_SpheresPBR(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
	void World_RenderingTests(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
	void World_SSAO_Test(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
	void World_Pendulum(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
	void World_Everything(CKE::EntityDatabase& admin, CKE::EntitySystem* system);
};
