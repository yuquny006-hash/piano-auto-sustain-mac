#include "Licensing/MacLicense.h"
#include <iostream>
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    auto fixture=juce::JSON::parse(juce::File(argv[1]));
    auto record=fixture["license"];
    auto pk=fixture["publicKey"].toString();
    auto machine=record["machineHash"].toString();
    auto check=[&](bool ok,const char* name){if(!ok){std::cerr<<name<<'\n';std::exit(1);}};
    using pas::licensing::MacLicense;
    check(MacLicense::verify(record,machine,pk),"Python to libsodium signature interoperability");
    check(!MacLicense::verify(record,juce::String::repeatedString("B",64),pk),"wrong machine");
    check(!MacLicense::verify(record,machine,juce::String::repeatedString("0",64)),"wrong public key");
    for(const auto* field:{"licenseKey","productId","machineHash","signature"}) {
        auto changed=record.clone();changed.getDynamicObject()->setProperty(field,"TAMPERED");
        check(!MacLicense::verify(changed,machine,pk),field);
    }
    auto changed=record.clone();changed.getDynamicObject()->removeProperty("expiresAt");
    check(!MacLicense::verify(changed,machine,pk),"missing expiry");
    changed=record.clone();changed.getDynamicObject()->setProperty("expiresAt","2030-01-01");
    check(!MacLicense::verify(changed,machine,pk),"nonperpetual");
    changed=record.clone();changed.getDynamicObject()->setProperty("schema",2);
    check(!MacLicense::verify(changed,machine,pk),"schema");
    check(!MacLicense::verify(juce::var(),machine,pk),"corrupt input");
#if JUCE_MAC
    check(MacLicense::machineHash().length()==64,"IOPlatformUUID fingerprint");
    check(MacLicense::licenseFile().getFullPathName().endsWith("/Library/Application Support/AmuPianoAutoSustain/license.dat"),"shared path");
#endif
    std::cout<<"Mac license checks passed\n";
}
