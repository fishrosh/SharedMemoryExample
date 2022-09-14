#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <memory>

#include <Windows.h>

#include <utils.h>

void throw_std_exception(const char* info);

using ProtectionFlags = DWORD;
using AccessFlags = DWORD;
using MemorySize = size_t;

class EventIPC
{
    HANDLE hEvent;

public:

    EventIPC(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
    {
        hEvent = OpenEventA(dwDesiredAccess, bInheritHandle, lpName);
        if (!hEvent)
        {
            throw_std_exception("Could not open interprocess event.");
        }
    }

    EventIPC(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName)
    {
        hEvent = CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName);
        if (!hEvent)
        {
            throw_std_exception("Could not create interprocess event.");
        }
    }

    EventIPC(const EventIPC&) = delete;

    EventIPC(EventIPC&& other)
        : hEvent{ nullptr }
    {
        vip::swapperator(hEvent, other.hEvent);
    }

    ~EventIPC()
    {
        CloseHandle(hEvent);
    }

    EventIPC& operator=(const EventIPC&) = delete;

    HANDLE get() { return hEvent; }
};

class EventHandle
{
    HANDLE hEvent;

public:

    EventHandle() : EventHandle{ NULL } {}

    EventHandle(HANDLE hEvent)
        : hEvent{ hEvent }
    {}

    HANDLE get() { return hEvent; }
    HANDLE operator*() { return hEvent; }
};

class MutexHandle
{
    HANDLE hMutex;

public:

    MutexHandle() : MutexHandle{ NULL } {}

    MutexHandle(HANDLE hMutex) : hMutex{ hMutex } {}

    void lock()
    {
        auto waitResult = WaitForSingleObject(hMutex, INFINITE);
        if (waitResult != WAIT_OBJECT_0)
        {
            throw_std_exception("Unable to acquire ownership of a mutex.");
        }
    }

    void unlock()
    {
        if (!ReleaseMutex(hMutex))
        {
            std::cout << "Unable to release mutex. Error: " << GetLastError() << std::endl;
        }
    }

    HANDLE get() { return hMutex; }
};

class MutexIPC
{
    HANDLE hMutex;

public:

    MutexIPC(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
    {
        hMutex = OpenMutexA(dwDesiredAccess, bInheritHandle, lpName);
        if (!hMutex)
        {
            throw_std_exception("Could not create interprocess mutex.");
        }
    }

    MutexIPC(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName)
    {
        hMutex = CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
        if (!hMutex)
        {
            throw_std_exception("Could not create interprocess mutex.");
        }
    }

    MutexIPC(const MutexIPC&) = delete;

    MutexIPC(MutexIPC&& other)
        : hMutex{ nullptr }
    {
        vip::swapperator(hMutex, other.hMutex);
    }

    ~MutexIPC()
    {
        if (hMutex) CloseHandle(hMutex);
    }

    MutexIPC& operator=(const MutexIPC&) = delete;

    HANDLE get() const { return hMutex; }

    MutexHandle getHandle() const { return MutexHandle{ hMutex }; }
};

class FileIPC
{
    HANDLE hFile;

public:

    FileIPC(LPCSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile)
    {
        hFile = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        if (!hFile)
        {
            throw_std_exception("Could not create mapping file.");
        }
    }

    FileIPC(const FileIPC&) = delete;

    ~FileIPC()
    {
        CloseHandle(hFile);
    }

    FileIPC& operator=(const FileIPC&) = delete;

    HANDLE get() { return hFile; }
};

class FileMappingHandle
{
    HANDLE hFileMapping;

public:

    FileMappingHandle() : FileMappingHandle{ NULL } {}

    FileMappingHandle(HANDLE hFileMapping)
        : hFileMapping{ hFileMapping }
    {}

    operator bool() { return hFileMapping; }

    HANDLE operator*() { return hFileMapping; }
    HANDLE get() { return hFileMapping; }
};

class FileMappingIPC
{
    HANDLE hFileMapping;

public:

    FileMappingIPC(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
    {
        hFileMapping = OpenFileMappingA(dwDesiredAccess, bInheritHandle, lpName);
        if (!hFileMapping)
        {
            throw_std_exception("Could not open file mapping object.");
        }
    }

    FileMappingIPC(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, ProtectionFlags flProtect, size_t size, LPCSTR lpName)
    {
        DWORD sizeH = HIWORD(size);
        DWORD sizeL = LOWORD(size);

        hFileMapping = CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, sizeH, sizeL, lpName);
        if (!hFileMapping)
        {
            throw_std_exception("Could not create file mapping object.");
        }
    }

    FileMappingIPC(const FileMappingIPC&) = delete;

    ~FileMappingIPC()
    {
        CloseHandle(hFileMapping);
    }

    FileMappingIPC& operator=(const FileMappingIPC&) = delete;

    HANDLE get() { return hFileMapping; }
    HANDLE operator*() { return hFileMapping; }
};

class MapViewHandle
{
    LPVOID mapping;

public:

    MapViewHandle() : MapViewHandle{ NULL } {}

    MapViewHandle(LPVOID mapping)
        : mapping{ mapping }
    {}

    operator bool() { return mapping; }

    LPVOID operator*() { return mapping; }
    LPVOID get() { return mapping; }
};

class MapViewIPC
{
    LPVOID mapping;

public:

    MapViewIPC(HANDLE hFileMapping, AccessFlags viewAccess, size_t offset, size_t size)
    {
        DWORD offsetH = HIWORD(offset);
        DWORD offsetL = LOWORD(offset);

        mapping = MapViewOfFile(hFileMapping, viewAccess, offsetH, offsetL, size);
        if (!mapping)
        {
            throw_std_exception("Could not create file view object.");
        }
    }

    MapViewIPC(const MapViewIPC&) = delete;

    MapViewIPC(MapViewIPC&& other)
        : MapViewIPC{}
    {
        std::swap(this->mapping, other.mapping);
    }

    ~MapViewIPC()
    {
        UnmapViewOfFile(mapping);
    }

    MapViewIPC& operator=(const MapViewIPC&) = delete;

    MapViewIPC& operator=(MapViewIPC&& other)
    {
        std::swap(this->mapping, other.mapping);
    }

    LPVOID get() { return mapping; }

private:

    MapViewIPC() : mapping{ nullptr } {}
};

template <typename _Ty>
class exclusive_ptr
{
    _Ty* memory;
    MutexHandle mutex;

public:

    exclusive_ptr(MutexHandle mutex, _Ty* memory)
        : mutex{ mutex }
        , memory{ memory }
    {
        mutex.lock();
    }

    exclusive_ptr(MutexHandle mutex, LPVOID memory)
        : exclusive_ptr{ mutex, reinterpret_cast<_Ty*>(memory) }
    {}

    exclusive_ptr(const exclusive_ptr&) = delete;

    exclusive_ptr& operator=(const exclusive_ptr&) = delete;

    ~exclusive_ptr()
    {
        mutex.unlock();
    }

    _Ty* operator*() { return memory; }
    _Ty* get() { return memory; }

    _Ty* const operator*() const { return memory; }
    _Ty* const get() const { return memory; }

    _Ty* operator->() { return memory; }
};

// hidden 
MemorySize adjustMemorySize(MemorySize size);

namespace ipc
{
    class Connector
    {
        MutexIPC viewMutex;
        LPVOID memory;

        MutexHandle hMutex;

    public:

        struct OpenInfo 
        {
            DWORD dwDesiredAccess;
            BOOL bInheritHandle;
        };

        Connector(LPVOID memory, LPCSTR lpName, OpenInfo const& openInfo)
            : viewMutex{ openInfo.dwDesiredAccess,
                openInfo.bInheritHandle,
                getMutexName(lpName).c_str() }
            , memory{ memory }
        {
            hMutex = viewMutex.get();
        }

        struct CreateInfo
        {
            BOOL bInitialOwner;
            BOOL bManualReset;
            BOOL bInitialState;
        };

        Connector(LPVOID memory, LPCSTR lpName, CreateInfo const& createInfo)
            : viewMutex{ nullptr,
                createInfo.bInitialOwner,
                getMutexName(lpName).c_str() }
            , memory{ memory }
        {
            hMutex = viewMutex.get();
        }

        Connector(Connector&& other)
            : viewMutex{ std::move(other.viewMutex) }
            , memory{ other.memory }
        {
            hMutex = viewMutex.get();
        }

        template <typename _Ty>
        exclusive_ptr<_Ty> acquire() { return exclusive_ptr<_Ty>{ hMutex, memory }; }

    private:

        static std::string getMutexName(LPCSTR lpName) { return std::string{ lpName } + "::Mutex"; }
    };

    struct Descriptor
    {
        LPCSTR lpName;
        MemorySize size;
        MemorySize alignment;
        AccessFlags accessFlags;
        DWORD dwDesiredAccess;
        BOOL bInheritHandle;
        BOOL bInitialOwner;
        BOOL bManualReset;
        BOOL bInitialState;

        operator Connector::OpenInfo() const
        {
            return Connector::OpenInfo{ dwDesiredAccess, bInheritHandle };
        }

        operator Connector::CreateInfo() const
        {
            return Connector::CreateInfo{ bInitialOwner, bManualReset, bInitialState };
        };
    };

    class Allocator
    {
        MapViewIPC mapView;
        LPWORD availableMemoryPtr;
         
    public:

        struct CreateInfo
        {
            AccessFlags accessFlags = SECTION_QUERY;
            MemorySize msMemorySize = 0ull;
            MemorySize msOffset = 0ull;
        };

        Allocator(FileMappingHandle fileMapping, CreateInfo const& createInfo)
            : mapView{ *fileMapping,
                createInfo.accessFlags,
                createInfo.msOffset,
                createInfo.msMemorySize }
        {
            availableMemoryPtr = reinterpret_cast<LPWORD>(mapView.get());
        }

        template <typename _Ty>
        _Ty* allocato()
        {
            return reinterpret_cast<_Ty*>(allocato(sizeof(_Ty), alignof(_Ty)));
        }

        LPWORD allocato(MemorySize msSize, MemorySize msAlignment);
    };

    class Memory : FileMappingIPC
    {
        Allocator alloc;

        std::vector<Connector> connectors;

        bool isOwner;

    public:

        struct CommonInfo
        {
            LPCSTR lpMemoryName = "";
            AccessFlags accessFlags = SECTION_QUERY;
            MemorySize msMemorySize = 0ull;

            operator Allocator::CreateInfo() const
            {
                return Allocator::CreateInfo{ accessFlags, msMemorySize, 0ull };
            }
        };

        struct OpenInfo : public CommonInfo
        {
            DWORD dwDesiredAccess;
            BOOL bInheritHandle;
        };

        Memory(OpenInfo const& openInfo)
            : FileMappingIPC{ openInfo.dwDesiredAccess,
                openInfo.bInheritHandle, 
                openInfo.lpMemoryName }
            , alloc{ this->get(), openInfo }
            , isOwner{ false }
        {}

        struct CreateInfo : public CommonInfo
        {
            HANDLE hFile = INVALID_HANDLE_VALUE;
            LPSECURITY_ATTRIBUTES lpFileMappingAttributes = NULL;
            ProtectionFlags flProtect = PAGE_READWRITE;
        };

        Memory(CreateInfo const& createInfo)
            : FileMappingIPC{ createInfo.hFile,
                createInfo.lpFileMappingAttributes,
                createInfo.flProtect,
                createInfo.msMemorySize,
                createInfo.lpMemoryName }
            , alloc{ this->get(), createInfo }
            , isOwner{ true }
        {}

        void allocate(vip::batch<Descriptor> descriptors);

        inline void clear() { connectors.clear(); }

        template <typename _Ty>
        exclusive_ptr<_Ty> acquire(size_t index) { return connectors[index].acquire<_Ty>(); }
    };
}