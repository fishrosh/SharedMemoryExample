
#include <sstream>
#include <array>
#include <thread>
#include <chrono>

#include "sharemem.h"
#include "tests.h"

// LPCSTR testName = "Global\\Test";
LPCSTR gExampleMappingName = ".exmap";

void throw_std_exception(const char* info)
{
    std::ostringstream oss;
    oss << info << " Error code: " << GetLastError() << std::endl;
    std::cout << oss.str();
    throw std::exception{ oss.str().c_str() };
}

struct Credentials
{
    char lpName[64];
    char lpPassword[64];
};

std::ostream& operator<<(std::ostream& os, Credentials const& cred)
{
    os << "Name: " << cred.lpName << std::endl;
    os << "Password: " << cred.lpPassword << std::endl;

    return os;
}

struct Parameters
{
    double A;
    float B;
    uint32_t C;
    int D;
};

std::ostream& operator<<(std::ostream& os, Parameters const& params)
{
    os << "Value of A is: " << params.A << std::endl;
    os << "Value of B is: " << params.B << std::endl;
    os << "Value of C is: " << params.C << std::endl;
    os << "Value of D is: " << params.D << std::endl;

    return os;
}

template <typename _Ty>
constexpr ipc::Descriptor createDescriptor(char const* lpName)
{
    return ipc::Descriptor{
        lpName,
        sizeof(_Ty),
        alignof(_Ty),
        SECTION_MAP_READ | SECTION_MAP_WRITE,
        SYNCHRONIZE,
        false,
        false,
        false,
        false
    };
}

std::shared_ptr<ipc::Memory> createSharedMemory()
{
    ipc::Memory::CreateInfo createInfo = {};
    createInfo.lpMemoryName = gExampleMappingName;
    createInfo.hFile = INVALID_HANDLE_VALUE;
    createInfo.lpFileMappingAttributes = nullptr;
    createInfo.msMemorySize = adjustMemorySize(sizeof(Credentials));
    createInfo.flProtect = PAGE_READWRITE;
    createInfo.accessFlags = SECTION_MAP_READ | SECTION_MAP_WRITE;

    return std::make_shared<ipc::Memory>(createInfo);
}

std::shared_ptr<ipc::Memory> openSharedMemory()
{
    ipc::Memory::OpenInfo openInfo = {};
    openInfo.bInheritHandle = false;
    openInfo.dwDesiredAccess = SECTION_MAP_READ | SECTION_MAP_WRITE;
    openInfo.lpMemoryName = gExampleMappingName;
    openInfo.msMemorySize = adjustMemorySize(sizeof(Credentials));
    openInfo.accessFlags = SECTION_MAP_READ | SECTION_MAP_WRITE;

    return std::make_shared<ipc::Memory>(openInfo);
}

struct SingleStaticTest
{
    static constexpr ipc::Descriptor credentialsExampleDescriptor{
        "Credentials",
        sizeof(Credentials),
        alignof(Credentials),
        SECTION_MAP_READ | SECTION_MAP_WRITE,
        SYNCHRONIZE,
        false,
        false,
        false,
        false
    };

    static constexpr std::array<ipc::Descriptor, 1ull> descriptors{
        credentialsExampleDescriptor
    };

    static void run()
    {
        auto memory = createSharedMemory();
        memory->allocate(descriptors);

        std::thread clientThread{ open };
        clientThread.join();

        for (auto i = 0ull; i < 1; ++i)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);

            auto connection = memory->acquire<Credentials>(0);

            std::cout << **connection;
        }
    }

    static void open()
    {
        auto memory = openSharedMemory();
        memory->allocate(descriptors);

        auto name = "Credentials test";
        auto password = "Test password";

        auto connection = memory->acquire<Credentials>(0);

        memcpy(connection->lpName, name, strlen(name) + 1);
        memcpy(connection->lpPassword, password, strlen(password) + 1);
    }
};

void ::tests::singleStatic()
{
    SingleStaticTest::run();
}

struct MultiStaticTest
{
    static constexpr std::array<ipc::Descriptor, 4ull> descriptors{
        createDescriptor<Credentials>("MainCreds"),
        createDescriptor<Parameters>("MainParams"),
        createDescriptor<Credentials>("HelperCreds"),
        createDescriptor<Credentials>("HelperParams")
    };

    static void run()
    {
        auto memory = createSharedMemory();
        memory->allocate(descriptors);

        std::thread clientThread{ open };
        clientThread.join();

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);

        {
            auto connection = memory->acquire<Credentials>(0);
            std::cout << **connection;
        }
        {
            auto connection = memory->acquire<Parameters>(1);
            std::cout << **connection;
        }
        {
            auto connection = memory->acquire<Credentials>(2);
            std::cout << **connection;
        }
        {
            auto connection = memory->acquire<Parameters>(3);
            std::cout << **connection;
        }
    }

    static void open()
    {
        auto memory = openSharedMemory();
        memory->allocate(descriptors);

        {
            auto name = "Janek Tarski";
            auto password = "abc111";

            auto connection = memory->acquire<Credentials>(0);

            memcpy(connection->lpName, name, strlen(name) + 1);
            memcpy(connection->lpPassword, password, strlen(password) + 1);
        }
        {
            auto params = memory->acquire<Parameters>(1);
            params->A = 11.11;
            params->B = 20.20f;
            params->C = 11111ul;
            params->D = -192;
        }

        {
            auto name = "Jean Maionaisse";
            auto password = "ukulele11";

            auto connection = memory->acquire<Credentials>(2);

            memcpy(connection->lpName, name, strlen(name) + 1);
            memcpy(connection->lpPassword, password, strlen(password) + 1);
        }
        {
            auto params = memory->acquire<Parameters>(3);
            params->A = 22.33;
            params->B = 77.44f;
            params->C = 1ul;
            params->D = 0;
        }
    }

};

void tests::multiStatic()
{
    MultiStaticTest::run();
}