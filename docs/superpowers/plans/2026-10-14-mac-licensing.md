# Mac licensing implementation plan

Goal: licensed Universal VST3/AU with offline Ed25519 activation, preserving Windows.
Architecture: server signs fixed-schema canonical JSON; plugin worker verifies and atomically saves; audio reads atomic authorization. Source and customer packages remain separate.
Stack: Python/SQLite, cryptography, JUCE 8.0.12, static libsodium 1.0.20, CMake/Xcode.
Spec: E:/WorkBuddy/work/2026-09-22-10-46-02/mac_licensing/Mac授权版_完整实现规格_合并版.md

- [x] Inspect actual Windows DPAPI and server response contract (takes precedence over inaccurate spec examples).
- [x] Add mac_signing.py and atomic binding; test Windows response, signatures, bad credentials, signing failure, concurrent binding.
- [x] Add Source/Licensing/MacLicense.*, processor worker, in-editor activation without changing main layout.
- [x] Gate Windows-only tests; require production public key for Mac CMake.
- [x] Add Universal libsodium build, signature fixtures and Mac verification tests.
- [x] Add staged installer with both-format rollback; package with required hashes, no private material.
- [x] Add in-app and customer Logic/VST3/activation instructions and friend acceptance checklist.
- [x] Verify Windows regression, script syntax, source archive contents.
- [ ] Deploy to real server after access is available; obtain only public key.
- [ ] Friend builds on Mac and executes AU validation, native/Rosetta host and licensing matrix; then approve customer distribution.

Constraints: version 1.0; no trial; unsigned license never enables engine; unlicensed MIDI unchanged; no production signing key on builder; no recurring activation checks; no Mac build or host-test claims from Windows. Never change issued Windows licenses.
Review focus: file corruption, editor closing mid-request, multi-instance license sharing, failed install preserving prior bundles, duplicate MIDI recording and stuck pedal. Production server access currently unavailable with saved SSH key; no deployment performed.
