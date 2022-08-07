/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#define PX_PHYSX_STATIC_LIB
#include <PxPhysicsAPI.h>
#include <PxFoundation.h>
#include <PxSceneDesc.h>
#include <PxPhysics.h>
#include <extensions/PxDefaultAllocator.h>
#include <common/PxTolerancesScale.h>

using namespace physx;

//--

template< typename T >
struct PhysxPointer
{
	T* ptr = nullptr;

	PhysxPointer() {};
	PhysxPointer(T* ptr_) : ptr(ptr_) {};
	~PhysxPointer()
	{
		if (ptr)
			ptr->release();
	}

	inline T* operator->() const { return ptr; };
};

//--

PxDefaultAllocator		GPhysXAllocator;
PxDefaultErrorCallback	GPhysXErrorCallback;

TEST(PhysX, CreateFoundataion)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);
}

TEST(PhysX, CreatePhysics)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);
}

TEST(PhysX, CreateScene)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	auto dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	PhysxPointer<PxScene> scene(physics->createScene(sceneDesc));
	ASSERT_NE(nullptr, scene.ptr);
}

TEST(PhysX, CreateSceneWithStaticActors)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	auto dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	PhysxPointer<PxScene> scene(physics->createScene(sceneDesc));
	ASSERT_NE(nullptr, scene.ptr);

	PhysxPointer<PxMaterial> material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*physics.ptr, PxPlane(0, 1, 0, 0), *material.ptr);
	scene.ptr->addActor(*groundPlane);
}

TEST(PhysX, CreateSceneWithDynamicActor)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, 0.0f, -9.81f);

	auto dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	PhysxPointer<PxScene> scene(physics->createScene(sceneDesc));
	ASSERT_NE(nullptr, scene.ptr);

	PhysxPointer<PxMaterial> material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	{
		PxRigidStatic* groundPlane = PxCreatePlane(*physics.ptr, PxPlane(0, 1, 0, 0), *material.ptr);
		scene.ptr->addActor(*groundPlane);
	}

	{
		PxTransform transform(PxVec3(0, 0, 2.0f));

		PxRigidDynamic* dynamic = PxCreateDynamic(*physics.ptr, transform, PxSphereGeometry(1.0f), *material.ptr, 10.0f);
		dynamic->setAngularDamping(0.5f);
		dynamic->setLinearVelocity(PxVec3(0, 0, 0));
		scene->addActor(*dynamic);
	}
}

TEST(PhysX, SimulateSceneWithDynamicActor)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, 0.0f, -9.81f);

	auto dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	PhysxPointer<PxScene> scene(physics->createScene(sceneDesc));
	ASSERT_NE(nullptr, scene.ptr);

	PhysxPointer<PxMaterial> material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	{
		PxRigidStatic* groundPlane = PxCreatePlane(*physics.ptr, PxPlane(0, 0, 1, 0), *material.ptr);
		scene.ptr->addActor(*groundPlane);
	}

	PxRigidDynamic* dynamic = nullptr;
	{
		PxTransform transform(PxVec3(0, 0, 2.0f));

		dynamic = PxCreateDynamic(*physics.ptr, transform, PxSphereGeometry(1.0f), *material.ptr, 10.0f);
		dynamic->setAngularDamping(0.5f);
		dynamic->setLinearVelocity(PxVec3(0, 0, 0));
		scene->addActor(*dynamic);
	}

	//float lastZ = dynamic->getGlobalPose().p.z;

	for (int i = 0; i < 100; ++i)
	{
		scene->simulate(1.0f / 100.0f);
		scene->fetchResults(true);

		const auto pos = dynamic->getGlobalPose();
		EXPECT_LT(1.0f, pos.p.z);

		//fprintf(stdout, "F[%d]: %f\n", i, pos.p.z);

		// make sure it falls down
		if (i == 30)
			EXPECT_LT(1.5f, pos.p.z);
	}
}

//--
