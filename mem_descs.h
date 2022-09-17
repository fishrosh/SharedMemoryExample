#pragma once

using ProtectionFlags = size_t;
using AccessFlags = size_t;
using MemorySize = size_t;

namespace ipc
{
    namespace Section
    {
        enum Access
        {
            eQuery = 0x1,
            eMapWrite = 0x2,
            eMapRead = 0x4,
            eMapExecute = 0x8,
            eExtend = 0x10,
            eMapExecuteExplicit = 0x20,
            eAllAccess = 0xFF
        };
    }

    namespace Page
    {
        enum Access
        {
            eNoAccess = 0x1,
            eReadonly = 0x2,
            eReadWrite = 0x4,
            eWriteCopy = 0x8,
            eExecute = 0x10
        };
    }

    namespace SAT
    {
        enum StandardAccessType
        {
            eDelete = 0x00010000,
            eReadControl = 0x00020000,
            eWriteDAC = 0x00040000,
            eWriteOwner = 0x00080000,
            eSynchronize = 0x00100000
        };
    }

    struct Descriptor
    {
        const char* lpName;
        MemorySize size;
        MemorySize alignment;
        AccessFlags accessFlags;
        uint32_t dwDesiredAccess;
        bool bInheritHandle;
        bool bInitialOwner;
        bool bManualReset;
        bool bInitialState;
    };

    struct IMemory
    {
        struct CommonInfo
        {
            const char* lpMemoryName;
            AccessFlags accessFlags;
            MemorySize msMemorySize;
        };

        struct OpenInfo : public CommonInfo
        {
            uint32_t dwDesiredAccess;
            bool bInheritHandle;
        };

        struct CreateInfo : public CommonInfo
        {
            void* hFile;
            void* lpFileMappingAttributes;
            ProtectionFlags flProtect;
        };
    };
}