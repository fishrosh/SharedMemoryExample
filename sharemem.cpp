#include <sstream>
#include <iostream>

#include "sharemem.h"
#include "mem_exports.h"

void throw_std_exception(const char* info)
{
    std::ostringstream oss;
    oss << info << " Error code: " << GetLastError() << std::endl;
    std::cout << oss.str();
    throw std::exception{ oss.str().c_str() };
}

MemorySize adjustMemorySize(MemorySize size)
{
    SYSTEM_INFO systemInfo = {};
    GetSystemInfo(&systemInfo);

    return ((size / systemInfo.dwPageSize) + 1) * systemInfo.dwPageSize;
}

void const* getNextAlignedAddress(void const* ptr, size_t alignment)
{
    return reinterpret_cast<char const*>(ptr) + !!(reinterpret_cast<size_t const>(ptr) % alignment) * alignment;
}

void* getNextAlignedAddress(void* ptr, size_t alignment)
{
    return reinterpret_cast<char*>(ptr) + !!(reinterpret_cast<size_t>(ptr) % alignment) * alignment;
}

size_t nextByAlignment(size_t n, size_t alignment)
{
    return n + !!(n % alignment) * alignment;
}

ipc::Connector::CreateInfo createCreateInfo(ipc::Descriptor const& descriptor)
{
    return ipc::Connector::CreateInfo{ 
        descriptor.bInitialOwner, 
        descriptor.bManualReset, 
        descriptor.bInitialState 
    };
}

ipc::Connector::OpenInfo createOpenInfo(ipc::Descriptor const& descriptor)
{
    return ipc::Connector::OpenInfo{
        descriptor.dwDesiredAccess, 
        descriptor.bInheritHandle
    };
}

void ::ipc::Memory::allocate(vip::batch<Descriptor> descriptors)
{
    connectors.reserve(connectors.size() + descriptors.size);
    for (auto& descriptor : descriptors)
    {
        LPWORD address = alloc.allocato(descriptor.size, descriptor.alignment);

        if (isOwner) connectors.push_back
        (
            Connector{ address, descriptor.lpName, createCreateInfo(descriptor)}
        );
        else connectors.push_back
        (
            Connector{ address, descriptor.lpName, createOpenInfo(descriptor) }
        );
    }
}

ipc::Connector* ipc::Memory::connect(const char* lpName)
{
    return nullptr;
}

ipc::Connector* ::ipc::Memory::connect(size_t nIndex)
{
    return &connectors[nIndex];
}

LPWORD ipc::Allocator::allocato(MemorySize msSize, MemorySize msAlignment)
{
    availableMemoryPtr = reinterpret_cast<LPWORD>(getNextAlignedAddress(availableMemoryPtr, msAlignment));
    auto output = availableMemoryPtr;
    availableMemoryPtr += msSize;

    return output;
}

void MutexHandle::lock()
{
    auto waitResult = WaitForSingleObject(hMutex, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        throw_std_exception("Unable to acquire ownership of a mutex.");
    }
}

void MutexHandle::unlock()
{
    if (!ReleaseMutex(hMutex))
    {
        std::cout << "Unable to release mutex. Error: " << GetLastError() << std::endl;
    }
}

template <typename _Callable_Ty, typename... Args>
Result trycatch_wrapper(_Callable_Ty _callable, Args&&... args)
{
    try {
        _callable(args...);
        return Result::eSuccess;
    }
    catch (std::exception ex) {
        lastKnownError = ex.what();
        return Result::eFailure;
    }
    catch (...) {
        return Result::eUnknownError;
    }
}

namespace exception_policy_nothrow
{
    template <typename _Callable_Ty, typename... Args>
    inline Result wrapper(_Callable_Ty _callable, Args&&... args)
    {
        return ::trycatch_wrapper(_callable, args...);
    }
}

namespace exception_policy_any
{
    template <typename _Callable_Ty, typename... Args>
    Result wrapper(_Callable_Ty _callable, Args&&... args)
    {
        _callable(args...);
        return Result::eSuccess;
    }
}

#ifdef EXCEPTION_POLICY_NOTHROW
#define _policy ::exception_policy_nothrow::
#else
#define _policy ::exception_policy_any::
#endif

const char* lastKnownError = nullptr;

// mutex
void _lock(void* handle) { reinterpret_cast<MutexHandle*>(handle)->lock(); }
void _unlock(void* handle) { reinterpret_cast<MutexHandle*>(handle)->unlock(); }

// connector
void _get_memory(void* pConnector, void** outMemPtr) 
{ 
    *outMemPtr = reinterpret_cast<ipc::Connector*>(pConnector)->getHandle(); 
}

void _get_mutex(void* pConnector, void** outMutex)
{
    *outMutex = &reinterpret_cast<ipc::Connector*>(pConnector)->getMutex();
}

// memory
void _create_memory(ipc::IMemory::CreateInfo const& createInfo, void** outSharedMemory)
{
    *outSharedMemory = new ipc::Memory(createInfo);
}

void _open_memory(ipc::IMemory::OpenInfo const& openInfo, void** outSharedMemory)
{
    *outSharedMemory = new ipc::Memory(openInfo);
}

void _allocate(void* handle, vip::batch<ipc::Descriptor> descriptors)
{
    reinterpret_cast<ipc::Memory*>(handle)->allocate(descriptors);
}

void _clear(void* pMemory) { reinterpret_cast<ipc::Memory*>(pMemory)->clear(); }
void _signal(void* pMemory) { reinterpret_cast<ipc::Memory*>(pMemory)->signal(); }
void _wait(void* pMemory) { reinterpret_cast<ipc::Memory*>(pMemory)->wait(); }

void _connect(void* pMemoryServer, const char* lpName, void** outConnector)
{
    *outConnector = reinterpret_cast<ipc::Memory*>(pMemoryServer)->connect(lpName);
}

void _connectIDX(void* pMemoryServer, size_t nIndex, void** outConnector)
{
    *outConnector = reinterpret_cast<ipc::Memory*>(pMemoryServer)->connect(nIndex);
}

// misc
void _adjust_mem_size(size_t inSize, size_t* outSize)
{
    *outSize = adjustMemorySize(inSize);
}

// mutex
API_CALL Result lock(void* handle)
{
    return _policy wrapper(_lock, handle);
}

API_CALL Result unlock(void* handle)
{
    return _policy wrapper(_unlock, handle);
}

// connector
API_CALL Result getHandle(void* pConnector, void** outMemPtr)
{
    return _policy wrapper(_get_memory, pConnector, outMemPtr);
}

API_CALL Result getMutex(void* pConnector, void** outMutex)
{
    return _policy wrapper(_get_mutex, pConnector, outMutex);
}

// memory
API_CALL Result createSharedMemory(ipc::IMemory::CreateInfo const& createInfo, void** outSharedMemory)
{
    return _policy wrapper(_create_memory, createInfo, outSharedMemory);
}

API_CALL Result openSharedMemory(ipc::IMemory::OpenInfo const& openInfo, void** outSharedMemory)
{
    return _policy wrapper(_open_memory, openInfo, outSharedMemory);
}

API_CALL void destroyMemoryServer(void* pMemoryServer)
{
    delete reinterpret_cast<ipc::Memory*>(pMemoryServer);
}

API_CALL Result allocate(void* handle, vip::batch<ipc::Descriptor> descriptors)
{
    return _policy wrapper(_allocate, handle, descriptors);
}

API_CALL Result clear(void* pMemoryServer)
{
    return _policy wrapper(_clear, pMemoryServer);
}

API_CALL Result signal(void* pMemoryServer)
{
    return _policy wrapper(_signal, pMemoryServer);
}

API_CALL Result wait(void* pMemoryServer)
{
    return _policy wrapper(_wait, pMemoryServer);
}

API_CALL Result connect(void* pMemoryServer, const char* lpName, void** outConnector)
{
    return _policy wrapper(_connect, pMemoryServer, lpName, outConnector);
}

API_CALL Result connectIDX(void* pMemoryServer, size_t nIndex, void** outConnector)
{
    return _policy wrapper(_connectIDX, pMemoryServer, nIndex, outConnector);
}

// misc
API_CALL Result adjustMemorySizeWrapper(size_t inSize, size_t* outSize)
{
    return _policy wrapper(_adjust_mem_size, inSize, outSize);
}
