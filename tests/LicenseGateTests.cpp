#include "Licensing/LicenseGate.h"
#include <iostream>
#include <stdexcept>

static void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

int main() {
    using namespace pas::licensing;
    try {
        const std::string machine(64, 'A');
        check(validateRecord("AMU_PIANO_AUTO_SUSTAIN|LC-TEST-KEY|" + machine, machine),
              "valid product, key, and machine must pass");
        check(!validateRecord("AMU_MUSIC_TRANSLATOR|LC-TEST-KEY|" + machine, machine),
              "another product must fail");
        check(!validateRecord("AMU_PIANO_AUTO_SUSTAIN|LC-TEST-KEY|" + std::string(64, 'B'), machine),
              "another machine must fail");
        check(!validateRecord("AMU_PIANO_AUTO_SUSTAIN||" + machine, machine),
              "empty key must fail");
        check(!validateRecord("broken", machine), "corrupt record must fail");
        check(!hasValidLocalLicenseAt(L"Z:\\definitely-missing\\license.dat", machine),
              "missing local record must fail closed");
        std::cout << "License gate validation tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
