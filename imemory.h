#pragma once

#include "mem_descs.h"
#include "External/Utilities/utils.h"

namespace ipc
{
    size_t adjustMemorySize(size_t nSize);

    class ExclusiveHandle
    {
        void* memory = nullptr;
        void* mutex = nullptr;

    public:

        ExclusiveHandle(void* memory, void* mutex);
        ExclusiveHandle(ExclusiveHandle const&) = delete;
        ExclusiveHandle(ExclusiveHandle&& other);

        ExclusiveHandle& operator=(ExclusiveHandle const&) = delete;
        ExclusiveHandle& operator=(ExclusiveHandle&& other);

        ~ExclusiveHandle();

        template <typename _Ty>
        _Ty& as() { return *reinterpret_cast<_Ty*>(memory); }

        template <typename _Ty>
        _Ty const& as() const { return *reinterpret_cast<_Ty*>(memory); }
    };

    template <typename _Ty>
    struct xhandle : ExclusiveHandle
    {
        // using ExclusiveHandle::ExclusiveHandle;
        // using ExclusiveHandle::operator=;

        xhandle(ExclusiveHandle&& eH) : ExclusiveHandle{ std::move(eH) } {}

        _Ty* operator->() { return &this->as<_Ty>(); }
        _Ty* operator*() { return &this->as<_Ty>(); }

        _Ty const* operator->() const { return &this->as<_Ty>(); }
        _Ty const* operator*() const { return &this->as<_Ty>(); }
    };

	class SharedMemory : public ipc::IMemory 
	{
		void* server;

	public:

        SharedMemory(OpenInfo const& openInfo);
        SharedMemory(CreateInfo const& createInfo);
        ~SharedMemory();

		void allocate(vip::batch<Descriptor> descriptors);
		void clear();
        void signal();
        void wait();

        ExclusiveHandle acquire(const char* lpName);
        ExclusiveHandle acquire(size_t nIndex);

        template <typename _Ty>
        xhandle<_Ty> acquire_t(const char* lpName) { return xhandle<_Ty>{ std::move(acquire(lpName)) }; }

        template <typename _Ty>
        xhandle<_Ty> acquire_t(size_t nIndex) { return xhandle<_Ty>{ std::move(acquire(nIndex)) }; }
	};
}