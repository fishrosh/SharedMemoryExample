#pragma once

#include "mem_descs.h"
#include "External/Utilities/utils.h"

#if SharedMemory_EXPORTS
#define API_CALL __declspec(dllexport)
#else
#define API_CALL __declspec(dllimport)
#endif

enum class Result : uint16_t
{
	eSuccess,
	eFailure,
	eUnknownError = 0xFFFF
};

// mutex handle iface
API_CALL Result lock(void* pMutexHandle);
API_CALL Result unlock(void* pMutexHandle);

// connector iface
API_CALL Result getHandle(void* pConnector, void** outMemPtr);
API_CALL Result getMutex(void* pConnector, void** outMutex);

// memory iface
API_CALL Result createSharedMemory(ipc::IMemory::CreateInfo const& createInfo, void** outSharedMemory);
API_CALL Result openSharedMemory(ipc::IMemory::OpenInfo const& openInfo, void** outSharedMemory);
API_CALL void destroyMemoryServer(void* pMemoryServer);
API_CALL Result allocate(void* pMemoryServer, vip::batch<ipc::Descriptor> descriptors);
API_CALL Result clear(void* pMemoryServer);
API_CALL Result signal(void* pMemoryServer);
API_CALL Result wait(void* pMemoryServer);
API_CALL Result connect(void* pMemoryServer, const char* lpName, void** outConnector);
API_CALL Result connectIDX(void* pMemoryServer, size_t nIndex, void** outConnector);

// misc
API_CALL Result adjustMemorySizeWrapper(size_t inSize, size_t* outSize);
