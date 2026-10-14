#pragma once
#include <string>
#include <string_view>

namespace pas::licensing {

inline constexpr std::string_view productId = "AMU_PIANO_AUTO_SUSTAIN";

bool validateRecord(std::string_view record, std::string_view expectedMachineHash);
std::string machineHash();
std::wstring licenseFilePath();
bool hasValidLocalLicenseAt(const std::wstring& path, std::string_view expectedMachineHash);
bool hasValidLocalLicense();

}
