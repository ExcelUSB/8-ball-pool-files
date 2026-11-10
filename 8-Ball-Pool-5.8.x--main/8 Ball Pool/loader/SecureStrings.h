#pragma once
#include <type_traits>
#include <cstdarg>

template<class Class>
using RawType = typename std::remove_const_t<std::remove_reference_t<Class>>;

template <unsigned long StringSize, int Key, typename Type>
class SecureString
{
public:
    constexpr SecureString(Type* Data)
    {
        XORonCompile(Data);
    }

    constexpr Type* Decrypt()
    {
        Type TempBuffer[StringSize]{};
        for (int i = 0; i < StringSize; i++)
        {
            TempBuffer[i] = DataBuffer[i] ^ (Key + i);
        }
        return TempBuffer;
    }

private:
    constexpr void XORonCompile(Type* Data)
    {
        for (int i = 0; i < StringSize; i++)
        {
            DataBuffer[i] = Data[i] ^ (Key + i);
        }
    }

    // Here is the Encrypted String saved
    Type DataBuffer[StringSize]{};
};


#define SString_Key(String, Key) []() {\
                    constexpr static auto x = SecureString<sizeof(String) / sizeof(String[0]), Key, RawType<decltype(String[0])>>((RawType<decltype(String[0])>*)String);\
                    return x;}()
#define SString(String) SString_Key(String,__TIME__[0])