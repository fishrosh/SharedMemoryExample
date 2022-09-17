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

API_CALL void lock(void* handle)
{
    if (auto mutex = reinterpret_cast<MutexHandle*>(handle))
    {
        mutex->lock();
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void unlock(void* handle)
{
    if (auto mutex = reinterpret_cast<MutexHandle*>(handle))
    {
        mutex->unlock();
    }
}

API_CALL void* getHandle(void* handle)
{
    if (auto connector = reinterpret_cast<ipc::Connector*>(handle))
    {
        return connector->getHandle();
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void* getMutex(void* handle)
{
    if (auto connector = reinterpret_cast<ipc::Connector*>(handle))
    {
        return &(connector->getMutex());
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void* createMemoryServer(ipc::IMemory::CreateInfo const& createInfo)
{
    return new ipc::Memory(createInfo);
}

API_CALL void* openMemoryServer(ipc::IMemory::OpenInfo const& openInfo)
{
    return new ipc::Memory(openInfo);
}

API_CALL void destroyMemoryServer(void* pMemoryServer)
{
    delete reinterpret_cast<ipc::Memory*>(pMemoryServer);
}

API_CALL void allocate(void* handle, vip::batch<ipc::Descriptor> descriptors)
{
    if (auto memory = reinterpret_cast<ipc::Memory*>(handle))
    {
        memory->allocate(descriptors);
    }
    else throw_std_exception( "Provided handle is not a proper handle.");
}

API_CALL void clear(void* pMemoryServer)
{
    if (auto memory = reinterpret_cast<ipc::Memory*>(pMemoryServer))
    {
        memory->clear();
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void signal(void* pMemoryServer)
{
    if (auto memory = reinterpret_cast<ipc::Memory*>(pMemoryServer))
    {
        memory->signal();
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void wait(void* pMemoryServer)
{
    if (auto memory = reinterpret_cast<ipc::Memory*>(pMemoryServer))
    {
        memory->wait();
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void* connect(void* handle, const char* lpName)
{
    if (auto memory = reinterpret_cast<ipc::Memory*>(handle))
    {
        return memory->connect(lpName);
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL void* connectIDX(void* pMemoryServer, size_t nIndex)
{
    if (auto memory = reinterpret_cast<ipc::Memory*>(pMemoryServer))
    {
        return memory->connect(nIndex);
    }
    else throw_std_exception("Provided handle is not a proper handle.");
}

API_CALL size_t adjustMemorySizeWrapper(size_t in)
{
    return ::adjustMemorySize(in);
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
