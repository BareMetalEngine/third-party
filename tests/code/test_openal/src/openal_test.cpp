/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include <algorithm>
#include <vector>
#include <thread>
#include <math.h>

#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>

//--

#if 0

TEST(OpenAL, InitDevice)
{
	ALCdevice* device = alcOpenDevice(NULL);
	ASSERT_NE(nullptr, device);

	alcCloseDevice(device);
}

//--

TEST(OpenAL, CheckEnumerationExtension)
{
	ALCdevice* device = alcOpenDevice(NULL);
	ASSERT_NE(nullptr, device);

	auto enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	EXPECT_TRUE(enumeration);

	alcCloseDevice(device);
}

//--

TEST(OpenAL, ListAudioDevices)
{
	ALCdevice* device = alcOpenDevice(NULL);
	ASSERT_NE(nullptr, device);

	if (auto enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT"))
	{
		const char* devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		while (devices && *devices)
		{
			auto len = strlen(devices);
			fprintf(stdout, "%s\n", devices);
			devices += (len + 1);
		}
	}	

	alcCloseDevice(device);
}

//--

class WrappedContext
{
public:
	ALCdevice* m_device = nullptr;
	ALCcontext* m_context = nullptr;

	WrappedContext()
	{
		m_device = alcOpenDevice(NULL);
		EXPECT_NE(nullptr, m_device);

		m_context = alcCreateContext(m_device, NULL);
		EXPECT_NE(nullptr, m_context);

		auto ret = alcMakeContextCurrent(m_context);
		EXPECT_TRUE(ret);

		ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

		alListener3f(AL_POSITION, 0, 0, 1.0f);
		alListener3f(AL_VELOCITY, 0, 0, 0);
		alListenerfv(AL_ORIENTATION, listenerOri);
	}

	~WrappedContext()
	{
		alcMakeContextCurrent(nullptr);

		if (m_context)
			alcDestroyContext(m_context);

		if (m_device)
			alcCloseDevice(m_device);
	}
};

TEST(OpenAL, CreateContext)
{
	WrappedContext ctx;
	EXPECT_TRUE(ctx.m_context);
}

struct WrappedSource
{
	ALuint source = 0;

	WrappedSource()
	{
		alGenSources((ALuint)1, &source);
		EXPECT_NE(0, (int)source);

		alSourcef(source, AL_PITCH, 1);
		alSourcef(source, AL_GAIN, 1);
		alSource3f(source, AL_POSITION, 0, 0, 0);
		alSource3f(source, AL_VELOCITY, 0, 0, 0);
		alSourcei(source, AL_LOOPING, AL_FALSE);
	}

	~WrappedSource()
	{
		if (source)
		{
			alDeleteSources(1, &source);
			source = 0;
		}
	}
};

struct WrappedBuffer
{
	ALuint buffer = 0;

	WrappedBuffer()
	{
		alGenBuffers((ALuint)1, &buffer);
		EXPECT_NE(0, (int)buffer);
	}

	~WrappedBuffer()
	{
		if (buffer)
		{
			alDeleteBuffers(1, &buffer);
			buffer = 0;
		}
	}
};

struct SineBuffer : public WrappedBuffer
{
	SineBuffer(uint32_t sampleRate, float length, float freq)
	{
		const auto numSamples = std::max<int>(256, (int)(sampleRate * length));

		std::vector<short> samples;
		samples.resize(numSamples);

		for (int i=0; i<numSamples; ++i)
		{
			float f = i / (float)sampleRate;
			//float v = 0.1f * sin(f * freq) + 0.5f;
			//int val = std::clamp<int>(v * 65536.0f, 0, 65535);
			float v = 0.5f * sin(f * freq);
			int val = std::clamp<int>((int)(v * 32767.0f), std::numeric_limits<short>::min(), std::numeric_limits<short>::max());
			samples[i] = (short)val;
		}

		alBufferData(buffer, AL_FORMAT_MONO16, samples.data(), (uint32_t)(samples.size() * sizeof(short)), sampleRate);
	}
};

TEST(OpenAL, CreateSource)
{
	WrappedContext ctx;
	EXPECT_TRUE(ctx.m_context);

	WrappedSource s;
	EXPECT_NE(0, (int)s.source);
}

TEST(OpenAL, CreateBuffer)
{
	WrappedContext ctx;
	EXPECT_TRUE(ctx.m_context);

	WrappedBuffer b;
	EXPECT_NE(0, (int)b.buffer);
}

TEST(OpenAL, CreateBufferWithData)
{
	WrappedContext ctx;
	EXPECT_TRUE(ctx.m_context);

	SineBuffer b(44100, 0.5f, 1000.0f);
	EXPECT_NE(0, (int)b.buffer);
}

TEST(OpenAL, PlayTestSound)
{
	WrappedContext ctx;
	EXPECT_TRUE(ctx.m_context);

	SineBuffer b(44100, 0.5f, 1000.0f);
	EXPECT_NE(0, (int)b.buffer);

	WrappedSource s;
	EXPECT_NE(0, (int)s.source);

	alSourcei(s.source, AL_BUFFER, b.buffer);
	alSourcePlay(s.source);

	int source_state = 0;
	alGetSourcei(s.source, AL_SOURCE_STATE, &source_state);
	while (source_state == AL_PLAYING) {
		std::this_thread::sleep_for(std::chrono::duration<double>(1.0));
		alGetSourcei(s.source, AL_SOURCE_STATE, &source_state);
	}
}

#endif