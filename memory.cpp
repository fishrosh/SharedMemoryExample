#include <algorithm>
#include <vector>

#include "mem_exports.h"
#include "imemory.h"

size_t ipc::adjustMemorySize(size_t nSize)
{
    return ::adjustMemorySizeWrapper(nSize);
}

ipc::ExclusiveHandle::ExclusiveHandle(void* memory, void* mutex)
	: memory{ memory }
	, mutex{ mutex }
{
    lock(mutex);
}

ipc::ExclusiveHandle::ExclusiveHandle(ExclusiveHandle&& other)
{
    std::swap(memory, other.memory);
    std::swap(mutex, other.mutex);
}

ipc::ExclusiveHandle& ipc::ExclusiveHandle::operator=(ExclusiveHandle&& other)
{
    std::swap(*this, other); return *this;
}

ipc::ExclusiveHandle::~ExclusiveHandle()
{
    unlock(mutex);
}

ipc::MemoryServer::MemoryServer(OpenInfo const& openInfo)
    : server{ openMemoryServer(openInfo) }
{}

ipc::MemoryServer::MemoryServer(CreateInfo const& createInfo)
    : server{ createMemoryServer(createInfo) }
{}

ipc::MemoryServer::~MemoryServer()
{
    destroyMemoryServer(server);
}

void ipc::MemoryServer::allocate(vip::batch<Descriptor> descriptors)
{
    ::allocate(server, descriptors);
}

void ipc::MemoryServer::clear()
{
    ::clear(server);
}

ipc::ExclusiveHandle ipc::MemoryServer::acquire(const char* lpName)
{
    auto connector = connect(server, lpName);
    auto memory = getHandle(connector);
    auto mutex = getMutex(connector);

    return ExclusiveHandle(memory, mutex);
}

ipc::ExclusiveHandle ipc::MemoryServer::acquire(size_t nIndex)
{
    auto connector = connectIDX(server, nIndex);
    auto memory = getHandle(connector);
    auto mutex = getMutex(connector);

    return ExclusiveHandle(memory, mutex);
}
