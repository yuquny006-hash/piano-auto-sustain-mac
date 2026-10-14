#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <mutex>

namespace pas::licensing {
// Owns all network, filesystem and crypto work. Audio only reads authorized.
class MacLicense final : private juce::Thread {
public:
    explicit MacLicense(std::atomic<bool>& authorized);
    ~MacLicense() override;
    void activate(juce::String code);
    juce::String status() const;
    static juce::String machineHash();
    static juce::File licenseFile();
    static bool verify(const juce::var&, const juce::String& machine, const juce::String& publicKey);
private:
    void run() override;
    void setStatus(juce::String);
    std::atomic<bool>& authorized;
    mutable std::mutex mutex;
    std::unique_ptr<juce::WebInputStream> network;
    juce::String pending, message{"正在检查授权 / Checking license"};
};
}
