#include "MacLicense.h"
#include <sodium.h>
namespace pas::licensing {
namespace {
bool asciiToken(const juce::String& s, const char* alphabet, int min, int max) {
    return s.length()>=min && s.length()<=max && s.containsOnly(alphabet);
}
constexpr const char* product="AMU_PIANO_AUTO_SUSTAIN";
}
bool MacLicense::verify(const juce::var& record, const juce::String& machine, const juce::String& publicKey) {
    auto* obj=record.getDynamicObject();
    if(!obj || obj->getProperties().size()!=6 || !asciiToken(machine,"0123456789ABCDEF",64,64)) return false;
    const auto schema=record["schema"], expiry=record["expiresAt"];
    const auto key=record["licenseKey"], prod=record["productId"], hash=record["machineHash"], signature=record["signature"];
    if(!schema.isInt() || int(schema)!=1 || !obj->hasProperty("expiresAt") || !expiry.isVoid()
       || !key.isString() || !prod.isString() || !hash.isString() || !signature.isString()
       || prod.toString()!=product || hash.toString()!=machine
       || !asciiToken(key.toString(),"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-",1,128)
       || !asciiToken(publicKey,"0123456789ABCDEF",64,64)) return false;
    // Sorted fixed ASCII fields; identical bytes to mac_signing.py.
    const juce::String canonical="{\"expiresAt\":null,\"licenseKey\":\""+key.toString()
      +"\",\"machineHash\":\""+machine+"\",\"productId\":\""+product+"\",\"schema\":1}";
    juce::MemoryOutputStream decoded;
    if(!juce::Base64::convertFromBase64(decoded,signature.toString()) || decoded.getDataSize()!=64) return false;
    unsigned char pk[32]; size_t count=0;
    if(sodium_init()<0 || sodium_hex2bin(pk,sizeof pk,publicKey.toRawUTF8(),64,nullptr,&count,nullptr)!=0 || count!=32) return false;
    return crypto_sign_verify_detached(static_cast<const unsigned char*>(decoded.getData()),
        reinterpret_cast<const unsigned char*>(canonical.toRawUTF8()),canonical.getNumBytesAsUTF8(),pk)==0;
}

}
