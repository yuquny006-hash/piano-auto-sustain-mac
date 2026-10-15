"""Instrument an isolated JUCE 8.0.12 checkout; never log request/response bodies."""
from pathlib import Path
import sys

MARKER = "PAS_AUTH_NETWORK_DIAGNOSTICS_V1"
ANCHOR = "    void didComplete (NSError*) override\n    {"
REPLACEMENT = r'''    void didComplete (NSError* pasError) override
    {
        // PAS_AUTH_NETWORK_DIAGNOSTICS_V1: this endpoint only, numeric error metadata only.
        // Preserve JUCE transport, TLS verification, callbacks and request body unchanged.
        if ([[[request.get() URL] host] isEqualToString: @"license.amumusic.com"]
            && [[[request.get() URL] path] isEqualToString: @"/api/activate"])
        {
            NSError* underlying = [pasError.userInfo objectForKey: NSUnderlyingErrorKey];
            if (![underlying isKindOfClass: [NSError class]]) underlying = nil;
            NSLog (@"PAS_AUTH_NETWORK_V1 domain=%@ code=%ld underlyingDomain=%@ underlyingCode=%ld",
                   pasError == nil ? @"none" : [pasError domain], (long) [pasError code],
                   underlying == nil ? @"none" : [underlying domain], (long) [underlying code]);
        }'''

def patch(path):
    text = path.read_text(encoding="utf-8")
    if MARKER in text:
        if text.count(REPLACEMENT) != 1:
            raise ValueError("Existing diagnostic patch differs; refusing to overwrite")
        return
    if text.count(ANCHOR) != 1:
        raise ValueError("Unsupported JUCE network source; expected one exact callback")
    path.write_text(text.replace(ANCHOR, REPLACEMENT), encoding="utf-8", newline="\n")

if __name__ == "__main__":
    patch(Path(sys.argv[1]) / "modules/juce_core/native/juce_Network_mac.mm")
