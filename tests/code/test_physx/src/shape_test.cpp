/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#define PX_PHYSX_STATIC_LIB
#include <PxPhysicsAPI.h>
#include <PxShape.h>
#include <extensions/PxDefaultAllocator.h>
#include <cooking/PxCooking.h>
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

extern PxDefaultAllocator		GPhysXAllocator;
extern PxDefaultErrorCallback	GPhysXErrorCallback;

TEST(Cooking, CreateCooking)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PxTolerancesScale sc;
	PxCookingParams params(sc);

	PhysxPointer<PxCooking> cooking(PxCreateCooking(PX_PHYSICS_VERSION, *foundation.ptr, params));
	ASSERT_NE(nullptr, cooking.ptr);
}

struct StdVectorOutputStream : public PxOutputStream
{
	std::vector<uint8_t> data;

	virtual uint32_t write(const void* src, uint32_t count) override
	{
		auto pos = data.size();
		data.resize(pos + count);
		memcpy(data.data() + pos, src, count);
		return count;
	}
};

const char* HexTable =
"00\00001\00002\00003\00004\00005\00006\00007\00008\00009\0000a\0000b\0000c\0000d\0000e\0000f\000"
"10\00011\00012\00013\00014\00015\00016\00017\00018\00019\0001a\0001b\0001c\0001d\0001e\0001f\000"
"20\00021\00022\00023\00024\00025\00026\00027\00028\00029\0002a\0002b\0002c\0002d\0002e\0002f\000"
"30\00031\00032\00033\00034\00035\00036\00037\00038\00039\0003a\0003b\0003c\0003d\0003e\0003f\000"
"40\00041\00042\00043\00044\00045\00046\00047\00048\00049\0004a\0004b\0004c\0004d\0004e\0004f\000"
"50\00051\00052\00053\00054\00055\00056\00057\00058\00059\0005a\0005b\0005c\0005d\0005e\0005f\000"
"60\00061\00062\00063\00064\00065\00066\00067\00068\00069\0006a\0006b\0006c\0006d\0006e\0006f\000"
"70\00071\00072\00073\00074\00075\00076\00077\00078\00079\0007a\0007b\0007c\0007d\0007e\0007f\000"
"80\00081\00082\00083\00084\00085\00086\00087\00088\00089\0008a\0008b\0008c\0008d\0008e\0008f\000"
"90\00091\00092\00093\00094\00095\00096\00097\00098\00099\0009a\0009b\0009c\0009d\0009e\0009f\000"
"a0\000a1\000a2\000a3\000a4\000a5\000a6\000a7\000a8\000a9\000aa\000ab\000ac\000ad\000ae\000af\000"
"b0\000b1\000b2\000b3\000b4\000b5\000b6\000b7\000b8\000b9\000ba\000bb\000bc\000bd\000be\000bf\000"
"c0\000c1\000c2\000c3\000c4\000c5\000c6\000c7\000c8\000c9\000ca\000cb\000cc\000cd\000ce\000cf\000"
"d0\000d1\000d2\000d3\000d4\000d5\000d6\000d7\000d8\000d9\000da\000db\000dc\000dd\000de\000df\000"
"e0\000e1\000e2\000e3\000e4\000e5\000e6\000e7\000e8\000e9\000ea\000eb\000ec\000ed\000ee\000ef\000"
"f0\000f1\000f2\000f3\000f4\000f5\000f6\000f7\000f8\000f9\000fa\000fb\000fc\000fd\000fe\000ff\000";

void BytesToHexString(std::stringstream& str, const uint8_t* data, uint32_t length)
{
	const auto* end = data + length;
	while (data < end)
	{
		const auto byte = *data++;
		const char* txt = HexTable + (3 * byte);
		str << txt;
	}
}

std::string VectorToHex(const std::vector<uint8_t>& data)
{
	std::stringstream str;
	BytesToHexString(str, data.data(), data.size());
	return str.str();
}

static const char* COMPILED_CONVEX_DATA = "4e5853014356584d0d0000000000000049434501434c484c090000000500000008000000050000001000000000000000000000000000803f000000000000803f00000000000080bf00000000000000000000803f00000000000000000000000000000000000080bf00000000000080bf0000000000000080000004013acd13bf3acd133f3acd133f3acd13bf040003033acd133f3acd133f3acd133f3acd13bf070003023acd133f3acd133f3acd13bf3acd13bf0a0003003acd13bf3acd133f3acd13bf3acd13bf0d000300040300020001020003010401030402010003000200010004010201040203030400010201020300010400020300030400000000000080bf00000000000080bf0000803f0000803f0000803fabaa2a3f8988083e0000000000000000000000008988083e0000000000000000000000008988083e000000000000803e00000000000080bf0000803ec732ec3e3acd133e3acd133e";

TEST(Cooking, CompileConvex)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);

	static const PxVec3 convexVerts[] = { PxVec3(0,1,0),PxVec3(1,0,0),PxVec3(-1,0,0),PxVec3(0,0,1),PxVec3(0,0,-1) };

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = 5;
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = convexVerts;
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PxTolerancesScale sc;
	PxCookingParams params(sc);

	PhysxPointer<PxCooking> cooking(PxCreateCooking(PX_PHYSICS_VERSION, *foundation.ptr, params));
	ASSERT_NE(nullptr, cooking.ptr);

	StdVectorOutputStream output;
	PxConvexMeshCookingResult::Enum condition = PxConvexMeshCookingResult::eSUCCESS;
	ASSERT_TRUE(cooking->cookConvexMesh(convexDesc, output, &condition));
	ASSERT_EQ(PxConvexMeshCookingResult::eSUCCESS, condition);

	EXPECT_EQ(351, output.data.size());

	const auto hexData = VectorToHex(output.data);
	EXPECT_STREQ(COMPILED_CONVEX_DATA, hexData.c_str());
}

//--

static const char* COMPILED_MESH_DATA = "4e5853014d4553480f00000000000000060000000300000001000000000000000000803f000000000000803f0000000000000000000080bf000000000000000000010200000000005254524502000000621080bf6f1203ba6f1203ba000000006210803f6210803f6f12033a000000000000803f0000803f0000803f0000803fe210003844218037f212833200000000040000000100000001000000040000000100000000000000621080bfffff7f7fffff7f7fffff7f7f6f1203baffff7f7fffff7f7fffff7f7f6f1203baffff7f7fffff7f7fffff7f7f6210803fffff7fffffff7fffffff7fff6210803fffff7fffffff7fffffff7fff6f12033affff7fffffff7fffffff7fff010000001d0000001d0000001d00000000008034000080bf00000000000000000000803f0000803f000000000100000038";

TEST(Cooking, CompileTriangleMesh)
{
	PhysxPointer<PxFoundation> foundation(PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback));
	ASSERT_NE(nullptr, foundation.ptr);

	PhysxPointer<PxPhysics> physics(PxCreatePhysics(PX_PHYSICS_VERSION, *foundation.ptr, PxTolerancesScale(), true, nullptr));
	ASSERT_NE(nullptr, physics.ptr);

	static const PxVec3 meshVertices[] = { PxVec3(0,1,0),PxVec3(1,0,0),PxVec3(-1,0,0) };
	static const uint16_t meshIndices[] = { 0, 1, 2 };

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = 3;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = meshVertices;
	meshDesc.triangles.count = 1;
	meshDesc.triangles.data = meshIndices;
	meshDesc.triangles.stride = 3 * sizeof(uint16_t);
	meshDesc.flags = PxMeshFlag::e16_BIT_INDICES;

	PxTolerancesScale sc;
	PxCookingParams params(sc);

	PhysxPointer<PxCooking> cooking(PxCreateCooking(PX_PHYSICS_VERSION, *foundation.ptr, params));
	ASSERT_NE(nullptr, cooking.ptr);

	StdVectorOutputStream output;
	PxTriangleMeshCookingResult::Enum condition = PxTriangleMeshCookingResult::eSUCCESS;
	ASSERT_TRUE(cooking->cookTriangleMesh(meshDesc, output, &condition));
	ASSERT_EQ(PxTriangleMeshCookingResult::eSUCCESS, condition);

	EXPECT_EQ(313, output.data.size());

	const auto hexData = VectorToHex(output.data);
	EXPECT_STREQ(COMPILED_MESH_DATA, hexData.c_str());
}

//--