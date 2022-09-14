#include "sharemem.h"

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

void ::ipc::Memory::allocate(vip::batch<Descriptor> descriptors)
{
    connectors.reserve(connectors.size() + descriptors.size);
    for (auto& descriptor : descriptors)
    {
        LPWORD address = alloc.allocato(descriptor.size, descriptor.alignment);

        if (isOwner) connectors.push_back
        (
            Connector{ address, descriptor.lpName, static_cast<Connector::CreateInfo>(descriptor)}
        );
        else connectors.push_back
        (
            Connector{ address, descriptor.lpName, static_cast<Connector::OpenInfo>(descriptor) }
        );
    }
}

LPWORD ipc::Allocator::allocato(MemorySize msSize, MemorySize msAlignment)
{
    availableMemoryPtr = reinterpret_cast<LPWORD>(getNextAlignedAddress(availableMemoryPtr, msAlignment));
    auto output = availableMemoryPtr;
    availableMemoryPtr += msSize;

    return output;
}
