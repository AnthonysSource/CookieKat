#include "Game.h"

#include "CookieKat/Core/Random/Random.h"
#include "CookieKat/Engine/Entities/Components/CameraComponent.h"
#include "CookieKat/Engine/Entities/Components/MeshComponent.h"
#include "CookieKat/Engine/Entities/Components/PointLightComponent.h"
#include "CookieKat/Engine/Resources/Resources/RenderMaterialResource.h"
#include "CookieKat/Engine/Resources/Resources/MeshResource.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"

#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "Systems/CubeMoverSystem.h"
#include "Systems/FlyCameraSystem.h"
#include "Systems/PendulumAnimationSystem.h"

using namespace CKE;

inline TResourceID<MeshResource>           s_CerberusMesh;
inline TResourceID<MeshResource>           s_SphereMesh;
inline TResourceID<MeshResource>           s_IcosphereMesh;
inline TResourceID<MeshResource>           s_CubeMesh;
inline TResourceID<RenderMaterialResource> s_CerberusMat;

inline TResourceID<MeshResource> s_MonkeyMesh;
inline TResourceID<MeshResource> s_PlatformMesh;
inline TResourceID<MeshResource> s_GridMesh;
inline TResourceID<MeshResource> s_LampBaseMesh;
inline TResourceID<MeshResource> s_LampLightMesh;

inline u64 GetNextMeshObjIdx() {
	static u64 lastIdx = 0;
	lastIdx++;
	return lastIdx;
}

void Game::LoadWorldResources(ResourceSystem& res) {
	s_CerberusMesh = res.LoadResource<MeshResource>("Models/Cerberus.fbx");
	s_SphereMesh = res.LoadResource<MeshResource>("Models/Sphere.fbx");
	s_CubeMesh = res.LoadResource<MeshResource>("Models/Cube.fbx");
	s_IcosphereMesh = res.LoadResource<MeshResource>("Models/Icosphere.fbx");
	s_PlatformMesh = res.LoadResource<MeshResource>("Models/Platform.fbx");
	s_LampBaseMesh = res.LoadResource<MeshResource>("Models/Lamp_Base.fbx");
	s_LampLightMesh = res.LoadResource<MeshResource>("Models/Lamp_Light.fbx");
	s_GridMesh = res.LoadResource<MeshResource>("Models/Grid.fbx");
	s_MonkeyMesh = res.LoadResource<MeshResource>("Models/Monkey.fbx");
	s_CerberusMat = res.LoadResourceAsync<RenderMaterialResource>("Materials/Cerberus.mat");
}

void Game::PopulateWorld(EntityDatabase& db, EntitySystem* system) {
	World_Everything(db, system);
}

void Game::RegisterAllComponents(CKE::EntityDatabase& db) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();
}

void Game::World_StressTest(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();
	system->AddSystem<CubeMoverSystem>();

	EntityID camera = db.CreateEntity();
	db.AddComponent<CameraComponent>(camera);


	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light, PointLightComponent{Vec3{5, 5, 5}, Vec3{125.0f}});

	constexpr i32 CUBE_SIZE = 30;
	constexpr f32           CUBE_SEPARATION = 100.0f;

	MeshComponent meshComp{};
	meshComp.m_MeshID = s_CubeMesh;

	for (i32 x = 0; x < CUBE_SIZE; ++x) {
		for (i32 y = 0; y < CUBE_SIZE; ++y) {
			for (i32 z = 0; z < CUBE_SIZE; ++z) {
				meshComp.m_MaterialModifiers = PBRTextureModifiers
				{
					{
						Random::F32(0.0f, 1.0f),
						Random::F32(0.0f, 1.0f),
						Random::F32(0.0f, 1.0f)
					},
					Random::F32(0.0f, 1.0f),
					Random::F32(0.0f, 1.0f)
				};
				meshComp.m_ObjectIdx = GetNextMeshObjIdx();

				EntityID e = db.CreateEntity();
				Mat4     model = glm::translate(Mat4(1.0f),
				                                Vec3(CUBE_SEPARATION * (-CUBE_SIZE / 2.0f + x),
				                                     CUBE_SEPARATION * (-CUBE_SIZE / 2.0f + y),
				                                     CUBE_SEPARATION * (-CUBE_SIZE / 2.0f + z)));
				db.AddComponent<LocalToWorldComponent>(e, model);
				db.AddComponent<MeshComponent>(e, meshComp);
				db.AddComponent<VelocityComponent>(e, Vec3{0.0f, 0.0f, 0.0f});
			}
		}
	}

	db.PrintAdminState();
}

void Game::World_CerberusPBR(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();

	EntityID camera = db.CreateEntity();
	db.AddComponent<CameraComponent>(camera);

	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light, PointLightComponent{Vec3{5, 5, 5}, Vec3{125.0f}});

	EntityID light2 = db.CreateEntity();
	db.AddComponent<
		PointLightComponent>(light2, PointLightComponent{Vec3{-7, 5, -7}, Vec3{325.0f}});

	EntityID light3 = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light3, PointLightComponent{Vec3{-2, 5, 0}, Vec3{225.0f}});

	for (i32 i = 0; i < 2; ++i) {
		EntityID gun = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(gun,
		                                       GetTransformL2W({0.5f * i, 0.0f, -0.5f},
		                                                       {-90.0f, 0.0f, 0.0f}, Vec3{0.01f}));
		db.AddComponent<MeshComponent>(gun,
		                               MeshComponent{
			                               s_CerberusMesh,
			                               s_CerberusMat,
			                               GetNextMeshObjIdx()
		                               });
	}
}

void Game::World_SpheresPBR(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();

	EntityID camera = db.CreateEntity();
	db.AddComponent<CameraComponent>(camera);

	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light, PointLightComponent{
		                                     Vec3{5, 5, 5}, Vec3{125.0f, 125.0f, 125.0f}
	                                     });
	//EntityID light2 = db.CreateEntity();
	//db.AddComponent<PointLightComponent>(light2, PointLightComponent{Vec3{-7, 5, -7}, Vec3{125.0f, 125.0f, 125.0f}});

	int meshObjIdx = 0;
	for (int x = -3; x < 4; ++x) {
		for (int y = -3; y < 4; ++y) {
			auto sphere = db.CreateEntity();
			Mat4 model = glm::translate(Mat4(1.0f), Vec3(0.0f));
			model = glm::translate(model, {x * 0.5f, y * 0.5f, -0.5f});
			model = glm::rotate(model, glm::radians(-90.0f), Vec3{1.0f, 0.0f, 0.0f});
			model = glm::scale(model, Vec3(0.002f));
			float xInterp = std::max(0.05f, (3.0f + static_cast<float>(x)) / 6.0f);
			float yInterp = std::max(0.05f, (3.0f + static_cast<float>(y)) / 6.0f);
			db.AddComponent<LocalToWorldComponent>(sphere, model);
			MeshComponent meshComp{};
			meshComp.m_MeshID = s_SphereMesh;
			meshComp.m_MaterialID = TResourceID<RenderMaterialResource>{};
			meshComp.m_MaterialModifiers = PBRTextureModifiers{
				{1.0f, 1.0f, 1.0f},
				xInterp,
				yInterp,
			};
			meshObjIdx++;
			meshComp.m_ObjectIdx = meshObjIdx;
			db.AddComponent<MeshComponent>(sphere, meshComp);
		}
	}
}

Mat4 GetSimpleTransformFBX(Vec3 pos, float scale) {
	Mat4 model = glm::translate(Mat4(1.0f), Vec3(0.0f));
	model = glm::translate(model, pos);
	model = glm::scale(model, Vec3(scale * 0.01f));
	return model;
}

MeshComponent GetSphereMeshComponent(Vec3 color, f32 roughness, f32 metallic) {
	MeshComponent meshComp{};
	meshComp.m_MeshID = s_SphereMesh;
	meshComp.m_MaterialID = TResourceID<RenderMaterialResource>{};
	meshComp.m_MaterialModifiers = PBRTextureModifiers{
		color, roughness, metallic,
	};
	meshComp.m_ObjectIdx = GetNextMeshObjIdx();
	return meshComp;
}

void Game::World_RenderingTests(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();

	//-----------------------------------------------------------------------------

	Vec3 colorArray[4];
	colorArray[0] = Vec3(1.0f, 1.0f, 1.0f);
	colorArray[1] = Vec3(1.0f, 0.0f, 0.0f);
	colorArray[2] = Vec3(0.0f, 1.0f, 0.0f);
	colorArray[3] = Vec3(0.0f, 0.0f, 1.0f);

	//-----------------------------------------------------------------------------

	enum class TestCase
	{
		Sphere_Simple,
		Sphere_Roughness,
		Sphere_Metallic,
		Sphere_Grid,
		Cerberus
	};
	TestCase testCase = TestCase::Cerberus;

	//-----------------------------------------------------------------------------

	EntityID        camera = db.CreateEntity();
	CameraComponent cameraComponent{};
	cameraComponent.m_CamPos = Vec3{0, 0, 5.0f};
	db.AddComponent<CameraComponent>(camera, cameraComponent);

	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light, PointLightComponent{Vec3{-3, 3, 3}, Vec3{160.0f}});

	//-----------------------------------------------------------------------------

	switch (testCase) {
	case TestCase::Sphere_Simple: {
		db.GetComponent<CameraComponent>(camera)->m_CamPos = Vec3{0, 0, 2.2f};

		EntityID sphere = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(sphere, GetSimpleTransformFBX(Vec3{0.0f}, 1.0f));
		db.AddComponent<MeshComponent>(sphere, GetSphereMeshComponent(Vec3{1.0f}, 0.5f, 0.0f));
	}
	break;

	case TestCase::Sphere_Roughness: {
		db.GetComponent<CameraComponent>(camera)->m_CamPos = Vec3{0, 0, 5.0f};

		for (i32 colIdx = 0; colIdx < 4; ++colIdx) {
			for (i32 x = 0; x < 7; ++x) {
				EntityID sphere = db.CreateEntity();

				db.AddComponent<LocalToWorldComponent>(sphere, GetSimpleTransformFBX(Vec3{x - 3.0f, colIdx - 1.5f, 0.0f}, 0.4f));
				f32 roughness = static_cast<f32>(x) / 6.0f;
				db.AddComponent<MeshComponent>(sphere, GetSphereMeshComponent(colorArray[colIdx], roughness, 0.0f));
			}
		}
	}
	break;

	case TestCase::Sphere_Metallic: {
		db.GetComponent<CameraComponent>(camera)->m_CamPos = Vec3{0, 0, 5.0f};

		for (i32 colIdx = 0; colIdx < 4; ++colIdx) {
			for (i32 x = 0; x < 7; ++x) {
				EntityID sphere = db.CreateEntity();

				db.AddComponent<LocalToWorldComponent>(sphere, GetSimpleTransformFBX(Vec3{x - 3.0f, colIdx - 1.5f, 0.0f}, 0.4f));
				f32 metallic = static_cast<f32>(x) / 6.0f;
				db.AddComponent<MeshComponent>(sphere, GetSphereMeshComponent(colorArray[colIdx], 0.5f, metallic));
			}
		}
	}
	break;

	case TestCase::Sphere_Grid: {
		PointLightComponent* pLight = db.GetComponent<PointLightComponent>(light);
		pLight->m_Radiance *= Vec3{2.0f};

		for (int x = 0; x < 7; ++x) {
			for (int y = 0; y < 7; ++y) {
				EntityID sphere = db.CreateEntity();
				db.AddComponent<LocalToWorldComponent>(sphere, GetSimpleTransformFBX(Vec3{x - 3.0f, y - 3.0f, -1.5f}, 0.4f));
				f32 roughness = static_cast<f32>(x) / 6.0f;
				f32 metallic = static_cast<f32>(y) / 6.0f;
				db.AddComponent<MeshComponent>(sphere, GetSphereMeshComponent(Vec3{1.0f, 0.0f, 0.0f}, roughness, metallic));
			}
		}
	}

	break;

	case TestCase::Cerberus: {
		PointLightComponent* pLight = db.GetComponent<PointLightComponent>(light);
		pLight->m_Position = Vec3{1.0f, 1.0f, 2.0f};

		EntityID gun = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(
			gun, GetTransformL2W({2.7f, 0.0f, 0.0f},
			                     {-90.0f, -90.0f, 0.0f},
			                     Vec3{0.05f}));
		db.AddComponent<MeshComponent>(
			gun, MeshComponent{
				s_CerberusMesh,
				s_CerberusMat,
				GetNextMeshObjIdx()
			});
	}
	break;
	default: ;
	}
}

void Game::World_SSAO_Test(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();

	EntityID camera = db.CreateEntity();
	db.AddComponent<CameraComponent>(camera);

	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light, PointLightComponent{
		                                     Vec3{5, 5, 5}, Vec3{125.0f, 125.0f, 125.0f}
	                                     });

	EntityID light2 = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light2, PointLightComponent{
		                                     Vec3{-7, 5, -7}, Vec3{125.0f, 125.0f, 125.0f}
	                                     });

	int meshObjIdx = 0;
	for (int x = -3; x < 4; ++x) {
		for (int y = -3; y < 4; ++y) {
			auto sphere = db.CreateEntity();
			Mat4 model = glm::translate(Mat4(1.0f), Vec3(0.0f));
			model = glm::translate(model, {x * 0.5f, y * 0.5f, -0.5f});
			model = glm::rotate(model, glm::radians(-90.0f), Vec3{1.0f, 0.0f, 0.0f});
			model = glm::scale(model, Vec3(0.2f));
			float xInterp = std::max(0.05f, (3.0f + static_cast<float>(x)) / 6.0f);
			float yInterp = std::max(0.05f, (3.0f + static_cast<float>(y)) / 6.0f);
			db.AddComponent<LocalToWorldComponent>(sphere, model);
			MeshComponent meshComp{};
			meshComp.m_MeshID = s_IcosphereMesh;
			meshComp.m_MaterialID = TResourceID<RenderMaterialResource>{};
			meshComp.m_MaterialModifiers = PBRTextureModifiers{
				{1.0f, 0.0f, 0.0f},
				xInterp,
				yInterp,
			};
			meshObjIdx++;
			meshComp.m_ObjectIdx = meshObjIdx;
			db.AddComponent<MeshComponent>(sphere, meshComp);
		}
	}
}

void Game::World_Pendulum(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PendulumComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();
	system->AddSystem<PendulumAnimationSystem>();

	EntityID camera = db.CreateEntity();
	db.AddComponent<CameraComponent>(camera);

	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(light, PointLightComponent{Vec3{0, 3, 3}, Vec3{0.6f, 0.2f, 0.2f}});

	auto sphere = db.CreateEntity();
	Mat4 model = glm::translate(Mat4(1.0f), Vec3(0.0f));
	model = glm::translate(model, {0.0f, 0.0f, -1.5f});
	model = glm::rotate(model, glm::radians(-90.0f), Vec3{1.0f, 0.0f, 0.0f});
	model = glm::scale(model, Vec3(0.002f));

	db.AddComponent<LocalToWorldComponent>(sphere, model);
	db.AddComponent<PendulumComponent>(sphere);
	MeshComponent meshComp{};
	meshComp.m_MeshID = s_SphereMesh;
	meshComp.m_MaterialID = TResourceID<RenderMaterialResource>{};
	meshComp.m_MaterialModifiers = PBRTextureModifiers{
		{1.0f, 0.0f, 0.5f},
		0.5f,
		0.0f
	};
	meshComp.m_ObjectIdx = 1;
	db.AddComponent<MeshComponent>(sphere, meshComp);
}

void Game::World_Everything(CKE::EntityDatabase& db, CKE::EntitySystem* system) {
	db.RegisterComponent<LocalToWorldComponent>();
	db.RegisterComponent<MeshComponent>();
	db.RegisterComponent<CameraComponent>();
	db.RegisterComponent<VelocityComponent>();
	db.RegisterComponent<PointLightComponent>();

	system->AddSystem<FlyCameraSystem>();

	EntityID        camera = db.CreateEntity();
	CameraComponent c{};
	c.m_MovSpeed = 8.0f;
	c.m_CamPos = Vec3{0.0f, 2.0f, 0.0f};
	c.m_View = glm::translate(Mat4(1.0f), c.m_CamPos);
	db.AddComponent<CameraComponent>(camera, c);

	{
		EntityID platform = db.CreateEntity();
		Mat4     model = glm::translate(Mat4(1.0f), Vec3(0.0f));
		db.AddComponent<LocalToWorldComponent>(platform, model);
		db.AddComponent<MeshComponent>(
			platform,
			MeshComponent{
				s_PlatformMesh,
				PBRTextureModifiers{
					{1.0f, 1.0f, 1.0f},
					0.95f, 0.0f,
				},
				GetNextMeshObjIdx()
			}
		);

		EntityID grid = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(grid, model);
		db.AddComponent<MeshComponent>(
			grid, MeshComponent{
				s_GridMesh, PBRTextureModifiers{
					{1.0f, 1.0f, 1.0f},
					0.95f, 0.0f,
				},
				GetNextMeshObjIdx()
			});
	}

	{
		Mat4 model = glm::translate(Mat4(1.0f), Vec3(0.0f));

		EntityID lampBase = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(lampBase, model);
		db.AddComponent<MeshComponent>(
			lampBase,
			MeshComponent{
				s_LampBaseMesh,
				PBRTextureModifiers{
					{0.03f, 0.0f, 0.0f},
					0.25f, 0.0f
				},
				GetNextMeshObjIdx()
			});

		EntityID lampLight = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(lampLight, model);
		db.AddComponent<MeshComponent>(
			lampLight,
			MeshComponent{
				s_LampLightMesh,
				PBRTextureModifiers{
					{20.0f, 20.0f, 50.0f},
					0.95f, 0.0f
				},
				GetNextMeshObjIdx()
			});
	}

	{
		EntityID monkey = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(
			monkey, glm::translate(Mat4(1.0f), Vec3(0.0f, 3.5f, 0.0f)));
		db.AddComponent<MeshComponent>(
			monkey, MeshComponent{
				s_MonkeyMesh,
				PBRTextureModifiers{
					{1.f, 0.3f, 0.1f},
					0.5f, 0.0f,
				},
				GetNextMeshObjIdx()
			});
	}

	EntityID light = db.CreateEntity();
	db.AddComponent<PointLightComponent>(
		light, PointLightComponent{
			Vec3{12, 5, 5}, Vec3{125.0f, 125.0f, 125.0f}
		});
	EntityID light2 = db.CreateEntity();
	db.AddComponent<PointLightComponent>(
		light2, PointLightComponent{Vec3{-10, 5, -7}, Vec3{125.0f, 125.0f, 125.0f}});

	for (i32 x = -3; x < 4; ++x) {
		for (i32 y = -3; y < 4; ++y) {
			EntityID sphere = db.CreateEntity();
			db.AddComponent<LocalToWorldComponent>(
				sphere, GetTransformL2W(
					{x * 1.2f, y * 1.2f + 4.2f, -6.5f},
					{-90.0f, 0.0f, 0.0f}, Vec3{0.005f}));
			db.AddComponent<MeshComponent>(
				sphere,
				MeshComponent{
					s_SphereMesh,
					PBRTextureModifiers{
						{1.0f, 1.0f, 1.0f},
						std::max(0.05f, (3.0f + static_cast<float>(x)) / 6.0f),
						std::max(0.05f, (3.0f + static_cast<float>(y)) / 6.0f),
					},
					GetNextMeshObjIdx()
				});
		}
	}

	for (i32 x = -3; x < 4; ++x) {
		for (i32 y = -3; y < 4; ++y) {
			EntityID sphere = db.CreateEntity();
			db.AddComponent<LocalToWorldComponent>(
				sphere,
				GetTransformL2W({x * 1.5f, y * 1.5f + 5.0f, 6.5f},
				                {-90.0f, 0.0f, 0.0f},
				                Vec3{0.6f}));
			db.AddComponent<MeshComponent>(
				sphere,
				MeshComponent{
					s_IcosphereMesh,
					PBRTextureModifiers{
						{1.0f, 0.0f, 0.0f},
						std::max(0.05f, (3.0f + static_cast<float>(x)) / 6.0f),
						std::max(0.05f, (3.0f + static_cast<float>(y)) / 6.0f),
					},
					GetNextMeshObjIdx()
				});
		}
	}

	{
		EntityID light = db.CreateEntity();
		db.AddComponent<PointLightComponent>(
			light, PointLightComponent{Vec3{2, 20, 3}, Vec3{125.0f, 125.0f, 125.0f}});

		EntityID light2 = db.CreateEntity();
		db.AddComponent<PointLightComponent>(
			light2, PointLightComponent{Vec3{7, 16, 2}, Vec3{125.0f, 125.0f, 125.0f}});

		EntityID light3 = db.CreateEntity();
		db.AddComponent<PointLightComponent>(
			light3, PointLightComponent{Vec3{-7, 16, -2}, Vec3{125.0f, 125.0f, 125.0f}});

		EntityID gun = db.CreateEntity();
		db.AddComponent<LocalToWorldComponent>(
			gun, GetTransformL2W({3.0f, 15.0f, 0.5f},
			                     {-90.0f, -90.0f, 0.0f}, Vec3{0.1f}));
		db.AddComponent<MeshComponent>(
			gun, MeshComponent{
				s_CerberusMesh,
				s_CerberusMat,
				GetNextMeshObjIdx()
			});
	}
}
