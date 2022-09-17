#pragma once

#include "mem_descs.h"
#include "External/Utilities/utils.h"

#if SharedMemory_EXPORTS
#define API_CALL __declspec(dllexport)
#else
#define API_CALL __declspec(dllimport)
#endif

// mutex handle iface
API_CALL void lock(void* pMutexHandle);
API_CALL void unlock(void* pMutexHandle);

// connector iface
API_CALL void* getHandle(void* pConnector);
API_CALL void* getMutex(void* pConnector);

// memory iface
API_CALL void* createMemoryServer(ipc::IMemory::CreateInfo const& createInfo);
API_CALL void* openMemoryServer(ipc::IMemory::OpenInfo const& openInfo);
API_CALL void destroyMemoryServer(void* pMemoryServer);
API_CALL void allocate(void* pMemoryServer, vip::batch<ipc::Descriptor> descriptors);
API_CALL void clear(void* pMemoryServer);
API_CALL void* connect(void* pMemoryServer, const char* lpName);
API_CALL void* connectIDX(void* pMemoryServer, size_t nIndex);

// misc
API_CALL size_t adjustMemorySizeWrapper(size_t in);
