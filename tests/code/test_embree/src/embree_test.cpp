/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#define EMBREE_STATIC_LIB
#include <embree3/rtcore.h>

//--

TEST(EmbreeBasic, DeviceInitTeardown)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	rtcReleaseDevice(device);
}

TEST(EmbreeBasic, GeometryInitTeardown)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	ASSERT_NE(nullptr, geom);

	rtcReleaseGeometry(geom);
	rtcReleaseDevice(device);
}

struct HelperVertex
{
	float x=0, y = 0, z = 0;

	inline HelperVertex()
	{}

	inline HelperVertex(float x_, float y_, float z_)
		:x(x_), y(y_), z(z_)
	{}
};

class HelperGeometry
{
public:
	HelperGeometry(RTCDevice device, uint32_t vertexCount, uint32_t triangleCount)
		: m_device(device)
	{
		m_geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
		EXPECT_NE(nullptr, m_geometry);

		m_vertices = (HelperVertex*)rtcSetNewGeometryBuffer(m_geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(HelperVertex), vertexCount);
		EXPECT_NE(nullptr, m_vertices);

		m_indices = (uint32_t*)rtcSetNewGeometryBuffer(m_geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), triangleCount);
		EXPECT_NE(nullptr, m_indices);
	}

	void commit()
	{
		rtcCommitGeometry(m_geometry);
	}

	virtual ~HelperGeometry()
	{
		rtcReleaseGeometry(m_geometry);
		m_geometry = nullptr;
	}

	RTCGeometry geometery() const
	{
		return m_geometry;
	}

protected:
	RTCDevice m_device = nullptr;
	RTCGeometry m_geometry = nullptr;
	HelperVertex* m_vertices = nullptr;
	uint32_t* m_indices = nullptr;
};

class HelperGeometryTriangle : public HelperGeometry
{
public:
	HelperGeometryTriangle(RTCDevice device)
		: HelperGeometry(device, 3, 1)
	{
		m_vertices[0] = HelperVertex(0, 0, 1);
		m_vertices[1] = HelperVertex(1, 0, -1);
		m_vertices[2] = HelperVertex(-1, 0, -1);

		m_indices[0] = 0;
		m_indices[1] = 1;
		m_indices[2] = 2;

		commit();
	}
};

TEST(EmbreeBasic, GeometryCommit)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperGeometryTriangle triangle(device);
	}

	rtcReleaseDevice(device);
}

class HelperScene
{
public:
	HelperScene(RTCDevice device)
		: m_device(device)
	{
		m_scene = rtcNewScene(device);
		EXPECT_NE(nullptr, m_scene);
	}

	~HelperScene()
	{
		rtcReleaseScene(m_scene);
	}

	operator RTCScene() const
	{
		return m_scene;
	}

	void addGeometry(HelperGeometry& geometry)
	{
		rtcAttachGeometry(m_scene, geometry.geometery());
		rtcCommitScene(m_scene);
	}

private:
	RTCDevice m_device;
	RTCScene m_scene;
};

TEST(EmbreeBasic, SceneInitTeardown)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperScene scene(device);
	}

	rtcReleaseDevice(device);
}


TEST(EmbreeBasic, GeometryAttachToScene)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperGeometryTriangle triangle(device);
		HelperScene scene(device);
		scene.addGeometry(triangle);
	}

	rtcReleaseDevice(device);
}

TEST(EmbreeBasic, IntersectScene1)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperGeometryTriangle triangle(device);
		HelperScene scene(device);
		scene.addGeometry(triangle);

		RTCIntersectContext context;
		rtcInitIntersectContext(&context);

		RTCRayHit rayhit;
		memset(&rayhit, 0, sizeof(rayhit));
		rayhit.ray.org_y = -2.0f;
		rayhit.ray.dir_y = 1.0f;
		rayhit.ray.tfar = 4.0f;
		rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

		rtcIntersect1(scene, &context, &rayhit);

		EXPECT_NE(RTC_INVALID_GEOMETRY_ID, rayhit.hit.geomID);
	}

	rtcReleaseDevice(device);
}

TEST(EmbreeBasic, IntersectScene4)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperGeometryTriangle triangle(device);
		HelperScene scene(device);
		scene.addGeometry(triangle);

		RTCIntersectContext context;
		rtcInitIntersectContext(&context);

		RTCRayHit4 rayhit;
		memset(&rayhit, 0, sizeof(rayhit));
		for (int i = 0; i < 4; ++i)
		{
			rayhit.ray.org_y[i] = -2.0f;
			rayhit.ray.dir_y[i] = 1.0f;
			rayhit.ray.tfar[i] = 4.0f;
			rayhit.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
		}

		int valid[4];
		memset(&valid, 0xFF, sizeof(valid));
		rtcIntersect4(valid, scene, &context, &rayhit);

		for (int i = 0; i < 4; ++i)
		{
			EXPECT_NE(RTC_INVALID_GEOMETRY_ID, rayhit.hit.geomID[i]);
		}
	}

	rtcReleaseDevice(device);
}

TEST(EmbreeBasic, IntersectScene8)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperGeometryTriangle triangle(device);
		HelperScene scene(device);
		scene.addGeometry(triangle);

		RTCIntersectContext context;
		rtcInitIntersectContext(&context);

		RTCRayHit8 rayhit;
		memset(&rayhit, 0, sizeof(rayhit));
		for (int i = 0; i < 8; ++i)
		{
			rayhit.ray.org_y[i] = -2.0f;
			rayhit.ray.dir_y[i] = 1.0f;
			rayhit.ray.tfar[i] = 4.0f;
			rayhit.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
		}

		int valid[8];
		memset(&valid, 0xFF, sizeof(valid));
		rtcIntersect8(valid, scene, &context, &rayhit);

		for (int i = 0; i < 8; ++i)
		{
			EXPECT_NE(RTC_INVALID_GEOMETRY_ID, rayhit.hit.geomID[i]);
		}
	}

	rtcReleaseDevice(device);
}

TEST(EmbreeBasic, IntersectScene16)
{
	RTCDevice device = rtcNewDevice("");
	ASSERT_NE(nullptr, device);

	{
		HelperGeometryTriangle triangle(device);
		HelperScene scene(device);
		scene.addGeometry(triangle);

		RTCIntersectContext context;
		rtcInitIntersectContext(&context);

		RTCRayHit16 rayhit;
		memset(&rayhit, 0, sizeof(rayhit));
		for (int i = 0; i < 16; ++i)
		{
			rayhit.ray.org_y[i] = -2.0f;
			rayhit.ray.dir_y[i] = 1.0f;
			rayhit.ray.tfar[i] = 4.0f;
			rayhit.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
		}

		int valid[16];
		memset(&valid, 0xFF, sizeof(valid));
		rtcIntersect16(valid, scene, &context, &rayhit);

		for (int i = 0; i < 16; ++i)
		{
			EXPECT_NE(RTC_INVALID_GEOMETRY_ID, rayhit.hit.geomID[i]);
		}
	}

	rtcReleaseDevice(device);
}

//--
