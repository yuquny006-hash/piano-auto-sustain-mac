#include "MacLicense.h"
#include <juce_cryptography/juce_cryptography.h>
#include <sodium.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>

#ifndef PAS_MAC_PUBLIC_KEY_HEX
#error Mac licensed builds require the server public key
#endif

namespace pas::licensing {
namespace {
bool asciiToken(const juce::String& s, const char* alphabet, int min, int max) {
    return s.length()>=min && s.length()<=max && s.containsOnly(alphabet);
}
constexpr const char* product="AMU_PIANO_AUTO_SUSTAIN";
constexpr const char* endpoint="https://license.amumusic.com/api/activate";
}

juce::String MacLicense::machineHash() {
    const auto service=IOServiceGetMatchingService(kIOMasterPortDefault,IOServiceMatching("IOPlatformExpertDevice"));
    if(!service) return {};
    auto value=IORegistryEntryCreateCFProperty(service,CFSTR("IOPlatformUUID"),kCFAllocatorDefault,0);
    IOObjectRelease(service);
    char buffer[128]{};
    const bool ok=value && CFGetTypeID(value)==CFStringGetTypeID()
      && CFStringGetCString(static_cast<CFStringRef>(value),buffer,sizeof buffer,kCFStringEncodingUTF8);
    if(value) CFRelease(value);
    if(!ok) return {};
    auto id=juce::String(buffer).trim().toUpperCase();
    if(id.isEmpty()) return {};
    return juce::SHA256(id.toRawUTF8(),id.getNumBytesAsUTF8()).toHexString().toUpperCase();
}
juce::File MacLicense::licenseFile() {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
      .getChildFile("Application Support/AmuPianoAutoSustain/license.dat");
}
MacLicense::MacLicense(std::atomic<bool>& state):Thread("PAS license worker"),authorized(state) {startThread();}
MacLicense::~MacLicense(){
    signalThreadShouldExit();notify();
    {std::lock_guard<std::mutex> lock(mutex);if(network) network->cancel();}
    stopThread(-1);
}
void MacLicense::setStatus(juce::String s){std::lock_guard<std::mutex> lock(mutex);message=std::move(s);}
juce::String MacLicense::status() const {std::lock_guard<std::mutex> lock(mutex);return message;}
void MacLicense::activate(juce::String code) {
    std::lock_guard<std::mutex> lock(mutex);
    pending=code.trim().toUpperCase();message=juce::String::fromUTF8("正在激活 / Activating…");notify();
}
void MacLicense::run() {
    const auto machine=machineHash(); const auto file=licenseFile();
    if(machine.isEmpty()){setStatus("无法读取机器码 / Machine ID unavailable");return;}
    auto readLocal=[&]{return file.existsAsFile() && file.getSize()<=16384
        && verify(juce::JSON::parse(file.loadFileAsString()),machine,PAS_MAC_PUBLIC_KEY_HEX);};
    authorized.store(readLocal(),std::memory_order_release);
    setStatus(authorized.load()?"已授权 / Licensed":"未激活，点击输入授权码 / Activate");
    while(!threadShouldExit()) {
        juce::String code;
        {std::lock_guard<std::mutex> lock(mutex);code=std::move(pending);pending.clear();}
        if(code.isNotEmpty()) {
            if(!asciiToken(code,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-",1,128)) {setStatus("授权码格式错误 / Invalid code format");continue;}
            auto* body=new juce::DynamicObject();
            body->setProperty("licenseKey",code);body->setProperty("productId",product);
            body->setProperty("machineHash",machine);body->setProperty("platform","macOS");body->setProperty("appVersion","1.0");
            const juce::var request(body);
            {
                std::lock_guard<std::mutex> lock(mutex);
                if(threadShouldExit()) break;
                network=std::make_unique<juce::WebInputStream>(juce::URL(endpoint).withPOSTData(juce::JSON::toString(request)),true);
                network->withExtraHeaders("Content-Type: application/json\r\n").withConnectionTimeout(15000).withNumRedirectsToFollow(0);
            }
            auto* stream=network.get();
            const bool connected=stream->connect(nullptr);
            if(threadShouldExit()) break;
            if(!connected){setStatus("连接失败 D1，请收集日志 / Connection failed D1");continue;}
            const int http=stream->getStatusCode();
            juce::MemoryOutputStream bytes;
            const auto read=bytes.writeFromInputStream(*stream,16385);
            if(threadShouldExit()) break;
            auto response=read<=16384?juce::JSON::parse(bytes.toUTF8()):juce::var();
            if(http!=200 || !response["valid"].isBool() || !bool(response["valid"])) {
                const auto reason=response["code"].toString();
                if(reason=="BOUND_TO_OTHER_MACHINE") setStatus("已绑定其他电脑，请申请换机 / Bound to another machine");
                else if(reason=="SIGNING_UNAVAILABLE") setStatus("Mac 授权服务未就绪 / Service not ready");
                else if(reason=="NOT_FOUND") setStatus("授权码不存在 / Code not found");
                else if(reason=="DISABLED" || reason=="EXPIRED") setStatus("授权码已停用或过期 / Code inactive");
                else setStatus(juce::String::fromUTF8("激活失败 / Activation failed HTTP ")+juce::String(http));
                continue;
            }
            auto record=response["license"];
            if(record["licenseKey"].toString()!=code || !verify(record,machine,PAS_MAC_PUBLIC_KEY_HEX)) {
                setStatus("授权验签失败 / License signature invalid");continue;
            }
            if(file.getParentDirectory().createDirectory().failed()) {setStatus("授权目录不可写 / Cannot create license folder");continue;}
            juce::TemporaryFile temp(file);
            if(!temp.getFile().replaceWithText(juce::JSON::toString(record))
               || chmod(temp.getFile().getFullPathName().toRawUTF8(),0600)!=0
               || !temp.overwriteTargetFileWithTemporary()) {setStatus("授权保存失败 / Cannot save license");continue;}
            authorized.store(true,std::memory_order_release);setStatus("已授权，可离线使用 / Licensed for offline use");
        }
        // Let AU/VST3 instances discover activation by another instance, offline.
        if(!authorized.load() && readLocal()) {authorized.store(true,std::memory_order_release);setStatus("已授权 / Licensed");}
        wait(2000);
    }
}
}
