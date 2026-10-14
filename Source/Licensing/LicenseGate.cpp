#include "LicenseGate.h"
#include <windows.h>
#include <bcrypt.h>
#include <dpapi.h>
#include <shlobj.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iterator>
#include <vector>

namespace pas::licensing {
namespace {

std::vector<unsigned char> decodeBase64(std::string text) {
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xef &&
        static_cast<unsigned char>(text[1]) == 0xbb && static_cast<unsigned char>(text[2]) == 0xbf)
        text.erase(0, 3);
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char c) { return std::isspace(c) != 0; }), text.end());
    DWORD size = 0;
    if (text.empty() || !CryptStringToBinaryA(text.c_str(), static_cast<DWORD>(text.size()),
            CRYPT_STRING_BASE64, nullptr, &size, nullptr, nullptr)) return {};
    std::vector<unsigned char> result(size);
    if (!CryptStringToBinaryA(text.c_str(), static_cast<DWORD>(text.size()), CRYPT_STRING_BASE64,
            result.data(), &size, nullptr, nullptr)) return {};
    result.resize(size);
    return result;
}

std::string decryptRecord(const std::vector<unsigned char>& encrypted) {
    if (encrypted.empty()) return {};
    DATA_BLOB input{static_cast<DWORD>(encrypted.size()), const_cast<BYTE*>(encrypted.data())};
    DATA_BLOB output{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output)) return {};
    std::string result(reinterpret_cast<char*>(output.pbData), output.cbData);
    LocalFree(output.pbData);
    return result;
}

std::string sha256(std::string_view value) {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objectSize = 0, bytes = 0;
    std::vector<unsigned char> object;
    std::array<unsigned char, 32> digest{};
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) return {};
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectSize),
            sizeof(objectSize), &bytes, 0) < 0) { BCryptCloseAlgorithmProvider(algorithm, 0); return {}; }
    object.resize(objectSize);
    if (BCryptCreateHash(algorithm, &hash, object.data(), objectSize, nullptr, 0, 0) < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0); return {};
    }
    PUCHAR data = reinterpret_cast<PUCHAR>(const_cast<char*>(value.data()));
    const auto ok = BCryptHashData(hash, data, static_cast<ULONG>(value.size()), 0) >= 0 &&
                    BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) >= 0;
    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (!ok) return {};
    static constexpr char hex[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(digest.size() * 2);
    for (auto byte : digest) { result.push_back(hex[byte >> 4]); result.push_back(hex[byte & 15]); }
    return result;
}

}

bool validateRecord(std::string_view record, std::string_view expectedMachineHash) {
    const auto first = record.find('|');
    const auto second = first == std::string_view::npos ? first : record.find('|', first + 1);
    if (first == std::string_view::npos || second == std::string_view::npos || record.find('|', second + 1) != std::string_view::npos)
        return false;
    const auto product = record.substr(0, first);
    const auto key = record.substr(first + 1, second - first - 1);
    const auto machine = record.substr(second + 1);
    return product == productId && !key.empty() && machine.size() == 64 && machine == expectedMachineHash;
}

std::string machineHash() {
    wchar_t value[256]{};
    DWORD bytes = sizeof(value);
    const auto status = RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid",
                                     RRF_RT_REG_SZ, nullptr, value, &bytes);
    if (status != ERROR_SUCCESS || value[0] == 0) return {};
    const int utf8Size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 1) return {};
    std::string utf8(static_cast<std::size_t>(utf8Size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, -1, utf8.data(), utf8Size, nullptr, nullptr);
    utf8.pop_back();
    return sha256(std::string(productId) + "|" + utf8);
}

std::wstring licenseFilePath() {
    PWSTR local = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &local))) return {};
    std::wstring path(local);
    CoTaskMemFree(local);
    return path + L"\\AmuPianoAutoSustain\\license.dat";
}

bool hasValidLocalLicenseAt(const std::wstring& path, std::string_view expectedMachineHash) {
    if (path.empty() || expectedMachineHash.empty()) return false;
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    std::string encoded((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    return validateRecord(decryptRecord(decodeBase64(std::move(encoded))), expectedMachineHash);
}

bool hasValidLocalLicense() {
    const auto machine = machineHash();
    return hasValidLocalLicenseAt(licenseFilePath(), machine);
}

}
