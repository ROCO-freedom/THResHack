﻿#pragma once
#include "Hook.h"
#include <memory>
#include <functional>
#include <vector>


class ResHackBase
{
protected:
	typedef void* (__cdecl* MallocType)(size_t size);
	MallocType m_thMalloc = NULL;
	InlineHook m_readResHook;


public:
	ResHackBase(void* thReadRes, void* myReadRes, MallocType thMalloc);
	ResHackBase(const ResHackBase&) = delete;
	virtual ~ResHackBase() = default;

protected:
	void* MyReadRes(const char* fileName, DWORD* bytesRead, BOOL isFile);
	virtual void* CallOriginalReadRes(const char* fileName, DWORD* bytesRead, BOOL isFile) = 0;
};

class ResHackFactory final
{
private:
	ResHackFactory() = default;
	ResHackFactory(const ResHackFactory&) = delete;

	struct Generator
	{
		const void* const signatureAddr = NULL;
		const std::vector<BYTE>* signature = NULL;
		std::function<std::unique_ptr<ResHackBase>()> generator;

		Generator(const void* _signatureAddr, decltype(signature) _signature, decltype(generator) _generator) :
			signatureAddr(_signatureAddr),
			signature(_signature),
			generator(_generator)
		{
		}
	};
	std::vector<Generator> m_generators;

	std::unique_ptr<ResHackBase> m_resHackInstance;

public:
	static ResHackFactory& GetInstance()
	{
		static ResHackFactory s_instance;
		return s_instance;
	}

	ResHackBase& GetResHackInstance();
	
	template<class T>
	class AddGenerator
	{
	public:
		template<class... Args>
		AddGenerator(uintptr_t signatureAddr, const std::vector<BYTE>& signature, Args... args)
		{
			auto& generators = ResHackFactory::GetInstance().m_generators;
			generators.emplace_back((void*)signatureAddr, &signature, [=] { return std::make_unique<T>(args...); });
		}
	};
};

#define REGISTER_RES_HACK(signatureAddr, signature, T, ...) \
	static ResHackFactory::AddGenerator<T> __RegisterResHack_##signature(signatureAddr, signature, ##__VA_ARGS__)
