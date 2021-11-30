#ifndef __EVENT_CONVERY_H__
#define __EVENT_CONVERY_H__

#include <codecvt>
#include <locale>

namespace std
{
    class codecvt_chs : public std::codecvt_byname<wchar_t, char, std::mbstate_t>
    {
    public:
        explicit codecvt_chs() : codecvt_byname("chs") { }
    };
}

namespace Evpp
{
    NOFORCEINLINE static bool U2W(const std::string& source, std::wstring& buffer)
    {
        try
        {
            buffer.swap(*const_cast<std::wstring*>(std::addressof(static_cast<const std::wstring&>(std::wstring(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(source))))));
            return true;
        }
        catch (const std::range_error& ex)
        {
            EVENT_ERROR("%s", ex.what());
        }

        return false;
    }

    NOFORCEINLINE static bool W2U(const std::wstring& source, std::string& buffer)
    {
        try
        {
            buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::string(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(source))))));
            return true;
        }
        catch (const std::range_error& ex)
        {
            EVENT_ERROR("%s", ex.what());
        }

        return false;
    }

    NOFORCEINLINE static bool A2W(const std::string& source, std::wstring& buffer)
    {
        try
        {
            buffer.swap(*const_cast<std::wstring*>(std::addressof(static_cast<const std::wstring&>(std::wstring(std::wstring_convert<std::codecvt_chs>().from_bytes(source))))));
            return true;
        }
        catch (const std::range_error& ex)
        {
            EVENT_ERROR("%s", ex.what());
        }
        return false;
    }

    NOFORCEINLINE static bool W2A(const std::wstring& source, std::string& buffer)
    {
        try
        {
            buffer.swap(*const_cast<std::string*>(std::addressof(static_cast<const std::string&>(std::string(std::wstring_convert<std::codecvt_chs>().to_bytes(source))))));
            return true;
        }
        catch (const std::range_error& ex)
        {
            EVENT_ERROR("%s", ex.what());
        }

        return false;
    }

    NOFORCEINLINE static bool U2A(const std::string& source, std::string& buffer)
    {
        std::wstring temp;
        {
            if (U2W(source, temp) && W2A(temp, buffer))
            {
                return true;
            }
        }

        return false;
    }

    NOFORCEINLINE static bool A2U(const std::string& source, std::string& buffer)
    {
        std::wstring temp;
        {
            if (A2W(source, temp) && W2U(temp, buffer))
            {
                return true;
            }
        }
        return false;
    }

}
#endif