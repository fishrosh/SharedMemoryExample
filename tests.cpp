
#include <array>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <array>
#include <vector>

#include "imemory.h"
#include "tests.h"

auto gExampleMappingName = ".exmap";

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
        ipc::Section::eMapRead | ipc::Section::eMapWrite,
        ipc::SAT::eSynchronize,
        false,
        false,
        false,
        false
    };
}

std::shared_ptr<ipc::MemoryServer> createSharedMemory()
{
    ipc::MemoryServer::CreateInfo createInfo = {};
    createInfo.lpMemoryName = gExampleMappingName;
    createInfo.hFile = nullptr;
    createInfo.lpFileMappingAttributes = nullptr;
    createInfo.msMemorySize = ipc::adjustMemorySize(sizeof(Credentials));
    createInfo.flProtect = ipc::Page::eReadWrite;
    createInfo.accessFlags = ipc::Section::eMapRead | ipc::Section::eMapWrite;

    return std::make_shared<ipc::MemoryServer>(createInfo);
}

std::shared_ptr<ipc::MemoryServer> openSharedMemory()
{
    ipc::MemoryServer::OpenInfo openInfo = {};
    openInfo.bInheritHandle = false;
    openInfo.dwDesiredAccess = ipc::Section::eMapRead | ipc::Section::eMapWrite;
    openInfo.lpMemoryName = gExampleMappingName;
    openInfo.msMemorySize = ipc::adjustMemorySize(sizeof(Credentials));
    openInfo.accessFlags = ipc::Section::eMapRead | ipc::Section::eMapWrite;

    return std::make_shared<ipc::MemoryServer>(openInfo);
}

struct SingleStaticTest
{
    static constexpr ipc::Descriptor credentialsExampleDescriptor{
        "Credentials",
        sizeof(Credentials),
        alignof(Credentials),
        ipc::Section::eMapRead | ipc::Section::eMapWrite,
        ipc::SAT::eSynchronize,
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

            auto connection = memory->acquire_t<Credentials>(0ull);

            std::cout << **connection;
        }
    }

    static void open()
    {
        auto memory = openSharedMemory();
        memory->allocate(descriptors);

        auto name = "Credentials test";
        auto password = "Test password";

        auto connection = memory->acquire_t<Credentials>(0ull);

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
            auto connection = memory->acquire_t<Credentials>(0ull);
            std::cout << **connection;
        }
        {
            auto connection = memory->acquire_t<Parameters>(1ull);
            std::cout << **connection;
        }
        {
            auto connection = memory->acquire_t<Credentials>(2ull);
            std::cout << **connection;
        }
        {
            auto connection = memory->acquire_t<Parameters>(3ull);
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

            auto connection = memory->acquire_t<Credentials>(0ull);

            memcpy(connection->lpName, name, strlen(name) + 1);
            memcpy(connection->lpPassword, password, strlen(password) + 1);
        }
        {
            auto params = memory->acquire_t<Parameters>(1);
            params->A = 11.11;
            params->B = 20.20f;
            params->C = 11111ul;
            params->D = -192;
        }
        {
            auto name = "Jean Maionaisse";
            auto password = "ukulele11";

            auto connection = memory->acquire_t<Credentials>(2ull);

            memcpy(connection->lpName, name, strlen(name) + 1);
            memcpy(connection->lpPassword, password, strlen(password) + 1);
        }
        {
            auto params = memory->acquire_t<Parameters>(3ull);
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

struct Example
{
    static constexpr std::array<ipc::Descriptor, 1ull> descriptors{
           createDescriptor<Credentials>("Creds")
    };

    static void run()
    {
        auto memory = createSharedMemory();
        memory->allocate(descriptors);
        memory->wait();

        auto connection = memory->acquire_t<Credentials>(0ull);
        std::cout << **connection;
    }

    static void open()
    {
        auto memory = openSharedMemory();
        memory->allocate(descriptors);

        auto name = "Crossprocess test";
        auto password = "YeSYesYes";

        {
            auto connection = memory->acquire_t<Credentials>(0ull);

            memcpy(connection->lpName, name, strlen(name) + 1);
            memcpy(connection->lpPassword, password, strlen(password) + 1);

            memory->signal();
        }
    }
};

void tests::example(bool isHost)
{
    if (isHost) Example::run();
    else Example::open();
}