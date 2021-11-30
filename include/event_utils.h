#ifndef __EVENT_UTILS_H__
#define __EVENT_UTILS_H__
#include <functional>
#include <unordered_map>
#include <memory>
#include <any>
#include <atomic>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <regex>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <random>
#include <cstdarg>
#ifdef H_OS_WINDOWS
#include <iphlpapi.h>
#include <tlhelp32.h>
#elif H_OS_LINUX
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

namespace Evpp
{
    static constexpr char charhexset [] = "0123456789ABCDEFabcdef";
    static constexpr char randhexset [] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    class LeaveStack
    {
    public:
        explicit LeaveStack(const Handler& callback) : leave_callback(callback) {};
        virtual ~LeaveStack()
        {
            if (nullptr != leave_callback)
            {
                leave_callback();
            }
        }
    private:
        Handler                                                     leave_callback;
    };

    NOFORCEINLINE static std::uintmax_t GetFileSize(const std::filesystem::path& fs)
    {
        if (std::filesystem::exists(fs))
        {
            return std::filesystem::file_size(fs);
        }
        return 0;
    }

    NOFORCEINLINE static bool GetFileSize(const double file_size, std::string& buffer)
    {
        constexpr static const char* format [] = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };

        if (buffer.size())
        {
            return false;
        }

        for (auto [i, j] = std::make_tuple(u96(0), static_cast<double>(1024ULL << 0x32)); i < std::size(format); ++i, j /= 1024.0F)
        {
            if (file_size < j)
            {
                continue;
            }

            if (buffer.empty())
            {
                buffer.append(std::to_string(static_cast<double>(file_size / j)));
                buffer.append(" ");
                buffer.append(format[i]);
            }
            break;
        }

        return true;
    }

    NOFORCEINLINE static bool GetFileSize(const std::string& file_name, std::string& buffer)
    {
        if (file_name.empty())
        {
            return false;
        }

        return GetFileSize(GetFileSize(std::filesystem::absolute(std::filesystem::path(file_name))), buffer);
    }

    NOFORCEINLINE static bool ExistsFile(const std::filesystem::path& fs)
    {
        return std::filesystem::exists(fs);
    }

    NOFORCEINLINE static bool ExistsDirectory(const std::string& path)
    {
        if (path.empty())
        {
            return false;
        }
        return std::filesystem::exists(std::filesystem::absolute(std::filesystem::path(path)));
    }


    NOFORCEINLINE static bool ExistsSubDirectory(const std::filesystem::path& absolute/* "dir/subdir/subdir"; */, const std::filesystem::path& relative /* subdir/subdir*/)
    {
        return std::search(absolute.begin(), absolute.end(), relative.begin(), relative.end()) != absolute.end();
    }

    NOFORCEINLINE static bool GetFileName(const std::string& path, std::string& buffer)
    {
        if (path.empty())
        {
            return false;
        }
        else
        {
            buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::filesystem::path(path).filename().string())))));
        }
        return true;
    }

    NOFORCEINLINE static bool GetFileName(const std::string& path, std::wstring& buffer)
    {
        if (path.empty())
        {
            return false;
        }
        else
        {
            buffer.swap(*const_cast<std::wstring*>(std::addressof(static_cast<const std::wstring&>(std::move(std::filesystem::path(path).filename().wstring())))));
        }
        return true;
    }

    NOFORCEINLINE static bool LoadSystemFile(const std::string& file, std::string& buffer, const std::filesystem::path& fs)
    {
        if (ExistsFile(fs))
        {
            if (buffer.size())
            {
                buffer.clear();
                buffer.shrink_to_fit();
            }

            try
            {
                buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string((std::istreambuf_iterator<char>(*const_cast<std::ifstream*>(std::addressof(static_cast<const std::ifstream&>(std::move(std::ifstream(file, std::ifstream::in | std::ifstream::binary))))))),
                                                                                                                           std::istreambuf_iterator<char>())
                )))));

                return true;
            }
            catch (...)
            {
                std::throw_with_nested(std::runtime_error(std::string().append("Can't resize to ").append(std::to_string(GetFileSize(fs)).append(" bytes"))));
            }
        }
        return false;
    }

    NOFORCEINLINE static bool LoadSystemFile(const std::string& file, std::string& buffer)
    {
        return LoadSystemFile(file, buffer, std::filesystem::absolute(std::filesystem::path(file)));
    }

    NOFORCEINLINE static bool EnumFilesBasic(const std::filesystem::directory_entry& entry, const std::filesystem::directory_iterator& list, const std::regex& express, const InterfaceFiles& function)
    {
        if (entry.status().type() != std::filesystem::file_type::directory)
        {
            return false;
        }

        for (const std::filesystem::directory_entry& file : list)
        {
            if (nullptr != function)
            {
                try
                {
                    if (std::regex_match(file.path().string(), express))
                    {
                        if (function(file.path().string(),
                                     GetFileSize(file.path()),
                                     std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(file.path().string()).time_since_epoch()) - std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::file_time_type::clock::now().time_since_epoch() - std::chrono::system_clock::now().time_since_epoch())))
                        {
                            continue;
                        }
                        break;
                    }
                }
                catch (const std::regex_error& ex)
                {
                    EVENT_ERROR("regex_error: %3d\t%s", ex.code(), ex.what());
                }
                catch (...)
                {
                    break;
                }
            }
        }

        return true;
    }

    NOFORCEINLINE static bool EnumFilesBasic(const std::filesystem::path& path, const std::regex& express, const InterfaceFiles& function)
    {
        try
        {
            return EnumFilesBasic(std::filesystem::directory_entry(path), std::filesystem::directory_iterator(path), express, function);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            EVENT_ERROR("%s %d %s %s", e.what(), e.code().value(), e.path1().string().c_str(), e.path2().string().c_str());
        }
        catch (...)
        {

        }
        return false;
    }

    NOFORCEINLINE static bool EnumFiles(const std::string& path, const std::regex& express/* .*\.\d+ or (.*)\.(\d+) */, const InterfaceFiles& function)
    {
        if (path.empty())
        {
            return false;
        }

        return EnumFilesBasic(std::filesystem::absolute(std::filesystem::path(path)), express, function);
    }

    NOFORCEINLINE static bool DeleteFiles(const std::filesystem::path& file)
    {
        if (file.empty())
        {
            return false;
        }

        if (ExistsFile(file))
        {
            return std::filesystem::remove(file);
        }

        return false;
    }

    NOFORCEINLINE static bool DeleteFiles(const std::string& file)
    {
        if (file.empty())
        {
            return false;
        }

        return DeleteFiles(std::filesystem::absolute(std::filesystem::path(file)));
    }

    NOFORCEINLINE static bool DeleteFiles(const std::string& file, const u64 file_date, const u64 date)
    {
        if (file.empty())
        {
            return false;
        }

        if (static_cast<u64>(file_date + date) <= ComplexUnixTimer())
        {
            return DeleteFiles(file);
        }

        return false;
    }

    NOFORCEINLINE static bool CreateDirectory(const std::string& path)
    {
        if (path.empty())
        {
            return false;
        }
        return std::filesystem::create_directory(std::filesystem::absolute(std::filesystem::path(path)));
    }

    NOFORCEINLINE static bool CreateDirectories(const std::string& path)
    {
        if (path.empty())
        {
            return false;
        }

        if (ExistsDirectory(path))
        {
            return true;
        }

        return std::filesystem::create_directories(std::filesystem::absolute(std::filesystem::path(path)));
    }

    NOFORCEINLINE static bool StringReplaceRegex(const std::string& src, std::string& buffer, const std::string& mask = "^\\s*|\\s*$", const std::string& fmt = std::string())
    {
        if (src.empty())
        {
            return false;
        }

        if (buffer.size())
        {
            buffer.clear();
            buffer.shrink_to_fit();
        }

        try
        {
            std::regex_replace(std::back_inserter(buffer), src.begin(), src.end(), std::regex(mask), fmt, std::regex_constants::match_default);
        }
        catch (const std::regex_error& ex)
        {
            EVENT_ERROR("%s", ex.what()); return false;
        }

        return true;
    }

    NOFORCEINLINE static bool StringReplace(std::string& source, const std::string& delimiters, const std::string& dispose = "", const std::size_t offset = 1)
    {
        if (source.empty())
        {
            return false;
        }

        if(delimiters.empty())
        {
            return true;
        }

        for (std::string::size_type pos = source.find(delimiters); pos != std::string::npos; pos = source.find(delimiters))
        {
            if (source.replace(pos, offset, dispose).size())
            {
                continue;
            }
            return false;
        }

        return true;
    }

    NOFORCEINLINE static bool StringSplit(std::string& source, std::vector<std::string>& array, const std::string& delimiters = "")
    {
        if (StringReplace(source, delimiters, " ", 1))
        {
            array.swap(*const_cast<std::vector<std::string>*>(std::addressof(static_cast<const std::vector<std::string>&>(std::move(std::vector<std::string>
            {
                std::istream_iterator<std::string>(*const_cast<std::istringstream*>(std::addressof(static_cast<const std::istringstream&>(std::move(std::istringstream(source)))))),
                    std::istream_iterator<std::string>()
            })))));
            return true;
        }

        return false;
    }

    NOFORCEINLINE static bool BinaryToString(const std::string& data, std::string& buffer)
    {
        if (data.empty())
        {
            return false;
        }

        if (buffer.size())
        {
            buffer.clear();
            buffer.shrink_to_fit();
        }

        for (const unsigned char byte : data)
        {
            buffer += charhexset[byte >> 0x4];
            buffer += charhexset[byte & 0x0F];
        }

        return true;
    }

    NOFORCEINLINE static bool StringToBinaryImpl(const std::string& data, std::string& buffer, const std::size_t offset)
    {
        if (data.empty())
        {
            return false;
        }

        for (std::size_t i = 0; i < data.size(); i += offset)
        {
            buffer += static_cast<char>(std::stoul(data.substr(i, offset), nullptr, 16));
        }

        return true;
    }

    NOFORCEINLINE static bool StringToBinary(std::string& data, std::string& buffer, const u32 transform = 0)
    {
        if (data.empty())
        {
            return false;
        }

        // 去掉 0x 去掉 空格
        if (StringReplace(data, "0x") && StringReplace(data, " "))
        {
            // 大小写转换
            if (data.end() == (1 == transform ? std::transform(data.begin(), data.end(), data.begin(), [](unsigned char ch) { return toupper(ch); }) : std::transform(data.begin(), data.end(), data.begin(), [](unsigned char ch) { return tolower(ch); })))
            {
                // 检查是否是完整的十六进制数
                if (std::string::npos != data.find_first_not_of(charhexset))
                {
                    return false;
                }

                return StringToBinaryImpl(data, buffer, 2);
            }
        }
        return false;
    }

    NOFORCEINLINE static bool RandomString(std::string& data, const u96 min = 5, const u96 max = 50)
    {
        if (data.size())
        {
            data.clear();
            data.shrink_to_fit();
        }

        char buffer[256] = {};
        {
            if (static_cast<char*>(buffer) != std::generate_n(buffer, std::uniform_int_distribution<u96>(min, max)(*const_cast<std::mt19937*>(std::addressof(static_cast<const std::mt19937&>(std::move(std::mt19937(std::random_device{}())))))), [&] { return randhexset[
                std::uniform_int_distribution<u96>(0, static_cast<u96>(std::size(randhexset) - 1))
                    (*const_cast<std::mt19937*>(std::addressof(static_cast<const std::mt19937&>(std::move(std::mt19937(std::random_device{}()))))))
            ]; }))
            {
                return StringReplaceRegex(buffer, data);
            }
        }

        return false;
    }

    NOFORCEINLINE static bool GetGetAdaptersMacAddress(const std::string& name, std::string& buffer, u96 count = 256)
    {
#ifdef H_OS_WINDOWS
        IP_ADAPTER_ADDRESSES* adapter_address = nullptr; unsigned long adapter_size = count * sizeof(IP_ADAPTER_ADDRESSES);
        {
            if (nullptr == adapter_address)
            {
                adapter_address = new IP_ADAPTER_ADDRESSES[adapter_size];
            }

            LeaveStack leave([&] { if (nullptr != adapter_address) { delete [] adapter_address; adapter_address = nullptr; } });
            {
                switch (GetAdaptersAddresses(AF_UNSPEC,
                                             GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS,
                                             nullptr,
                                             adapter_address,
                                             &adapter_size))
                {
                    case NO_ERROR:
                    {
                        for (IP_ADAPTER_ADDRESSES* adapter = adapter_address; nullptr != adapter; adapter = adapter->Next)
                        {
                            std::string adapter_name, adapter_description;
                            {
                                if (W2A(adapter->FriendlyName, adapter_name) && W2A(adapter->Description, adapter_description))
                                {
                                    if (name == adapter_name)
                                    {
                                        if (0 != adapter->PhysicalAddressLength)
                                        {
                                            char adapter_bytes_buffer[32] = {};
                                            {
                                                sprintf(adapter_bytes_buffer, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
                                                        adapter->PhysicalAddress[0],
                                                        adapter->PhysicalAddress[1],
                                                        adapter->PhysicalAddress[2],
                                                        adapter->PhysicalAddress[3],
                                                        adapter->PhysicalAddress[4],
                                                        adapter->PhysicalAddress[5]
                                                );
                                                buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(adapter_bytes_buffer, std::size(adapter_bytes_buffer)))))));
                                            }

                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case ERROR_ADDRESS_NOT_ASSOCIATED:  EVENT_ERROR("An address has not yet been associated with the network endpoint. DHCP lease information was available"); break;
                    case ERROR_BUFFER_OVERFLOW:         EVENT_ERROR("The buffer size indicated by the SizePointer parameter is too small to hold the adapter information or the AdapterAddresses parameter is NULL. The SizePointer parameter returned points to the required size of the buffer to hold the adapter information"); break;
                    case ERROR_INVALID_PARAMETER:       EVENT_ERROR("One of the parameters is invalid. This error is returned for any of the following conditions: the SizePointer parameter is NULL, the Address parameter is not AF_INET, AF_INET6, or AF_UNSPEC, or the address information for the parameters requested is greater than ULONG_MAX"); break;
                    case ERROR_NOT_ENOUGH_MEMORY:       EVENT_ERROR("Insufficient memory resources are available to complete the operation."); break;
                    case ERROR_NO_DATA:                 EVENT_ERROR("No addresses were found for the requested parameters."); break;
                    default:                            EVENT_ERROR("Use FormatMessage to obtain the message string for the returned error."); break;
                }
            }
        }
#elif H_OS_LINUX
        struct ifreq request = {};

        int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

        if (strcpy(request.ifr_name, name.c_str()))
        {
            LeaveStack leave([&] { if (-1 != s)(close(s)); });
            {
                if (0 == ioctl(s, SIOCGIFHWADDR, &request))
                {
                    char adapter_bytes_buffer[32] = {};
                    {
                        sprintf(adapter_bytes_buffer, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
                                request.ifr_hwaddr.sa_data[0],
                                request.ifr_hwaddr.sa_data[1],
                                request.ifr_hwaddr.sa_data[2],
                                request.ifr_hwaddr.sa_data[3],
                                request.ifr_hwaddr.sa_data[4],
                                request.ifr_hwaddr.sa_data[5]
                        );

                    }
                    buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::move(std::string(reinterpret_cast<const char*>(adapter_bytes_buffer), std::size(adapter_bytes_buffer)))))));
                    return true;
                }
            }
        }
#endif
        return false;
    }

    NOFORCEINLINE static i32 GetProcessId(const std::string& name)
    {
#ifdef H_OS_WINDOWS
        PROCESS_INFORMATION pi = {};
        PROCESSENTRY32 ps = {};
        ps.dwSize = sizeof(PROCESSENTRY32);

        if (HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); INVALID_HANDLE_VALUE != hSnapshot)
        {
            LeaveStack leave([&] { CloseHandle(hSnapshot); });
            {
                if (!Process32First(hSnapshot, &ps))
                {
                    return 0;
                }

                do
                {
#ifdef UNICODE
                    std::string buffer;

                    if (W2A(ps.szExeFile, buffer))
                    {
                        EVENT_INFO("%s", buffer.c_str());
                    }
#else
                    if (name == ps.szExeFile)
                    {
                        return ps.th32ProcessID;
                    }
#endif
                } while (Process32Next(hSnapshot, &ps));
            }
        }

        return 0;
#elif H_OS_LINUX
        char filepath[384]; char process_name[256]; char buf[1024];
        {
            if (DIR *dir_stream = opendir("/proc"); nullptr != dir_stream)
            {
                LeaveStack leave([&] { closedir(dir_stream); });
                {
                    for (struct dirent *dir_ent = readdir(dir_stream); nullptr != dir_ent; dir_ent = readdir(dir_stream))
                    {
                        if (DT_DIR != dir_ent->d_type || !std::isdigit(dir_ent->d_name[0]) || (strcmp(dir_ent->d_name, ".") == 0) || (strcmp(dir_ent->d_name, "..") == 0))
                        {
                            continue;
                        }

                        if (sprintf(filepath, "/proc/%s/status", dir_ent->d_name))
                        {
                            try
                            {
                                std::ifstream infile(filepath, std::ios::in | std::ios::binary);
                                {
                                    if (infile.is_open() && infile.getline(buf, std::size(buf)).good() && sscanf(buf, "%*s%s", process_name))
                                    {
                                        std::string buffer;
                                        {
                                            if (StringReplaceRegex(process_name, buffer))
                                            {
                                                if (name == buffer)
                                                {
                                                    return std::atoi(dir_ent->d_name);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            catch (...)
                            {
                                std::throw_with_nested(std::runtime_error(std::string().append("Can't resize to ").append(std::to_string(GetFileSize(fs)).append(" bytes"))));
                            }
                        }
                }
            }
        }
    }
        return 0;
#endif
}
}
#endif // __EVENT_UTILS_H__