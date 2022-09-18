#include <algorithm>
#include <vector>

#include "mem_exports.h"
#include "imemory.h"

size_t ipc::adjustMemorySize(size_t nSize)
{
    size_t output;
    ::adjustMemorySizeWrapper(nSize, &output);
    return output;
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
    if (mutex) unlock(mutex);
}

ipc::SharedMemory::SharedMemory(OpenInfo const& openInfo)
    : server{ nullptr }
{
    openSharedMemory(openInfo, &server);
}

ipc::SharedMemory::SharedMemory(CreateInfo const& createInfo)
    : server{ nullptr }
{
    createSharedMemory(createInfo, &server);
}

ipc::SharedMemory::~SharedMemory()
{
    destroyMemoryServer(server);
}

void ipc::SharedMemory::allocate(vip::batch<Descriptor> descriptors)
{
    ::allocate(server, descriptors);
}

void ipc::SharedMemory::clear()
{
    ::clear(server);
}

void ipc::SharedMemory::signal()
{
    ::signal(server);
}

void ipc::SharedMemory::wait()
{
    ::wait(server);
}

ipc::ExclusiveHandle ipc::SharedMemory::acquire(const char* lpName)
{
    void* connector, * memory, * mutex;
    connect(server, lpName, &connector);
    getHandle(connector, &memory);
    getMutex(connector, &mutex);

    return ExclusiveHandle(memory, mutex);
}

ipc::ExclusiveHandle ipc::SharedMemory::acquire(size_t nIndex)
{
    void* connector, * memory, * mutex;
    connectIDX(server, nIndex, &connector);
    getHandle(connector, &memory);
    getMutex(connector, &mutex);

    return ExclusiveHandle(memory, mutex);
}
