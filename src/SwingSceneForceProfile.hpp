// SPDX-License-Identifier: MIT
#pragma once
#include "QuartzProfile.hpp"

namespace gbb::profile {
// Reader fingerprints and argument bytes from 54yq/GakuMotion 7dacef7.
// The output filter follows effea26 under the GakuBurstBridge MIT license.
inline constexpr unsigned SwingPropertiesSize = 104;
inline constexpr unsigned SwingSceneForceOffset = 32;
inline constexpr unsigned SwingSceneForceSize = 17 * sizeof(float);
static_assert(SwingSceneForceOffset + SwingSceneForceSize + sizeof(int32_t) == SwingPropertiesSize);

struct SwingPropertyReaderVariant {
    uint32_t swing, reader, end, callReturn;
    uint64_t fingerprint;
    unsigned char callArguments[16];
};
inline constexpr SwingPropertyReaderVariant SwingPropertyReaders[]{
    {0x280b0,0x100f0,0x11369,0x282ca,0x6fe5efa5a3a6e6deull,
     {0x4d,0xb8,0x48,0x8d,0x95,0x8c,0,0,0,0x4c,0x8d,0x85,0xe0,1,0,0}},
    {0xcc8e0,0xade00,0xaeddb,0xcca83,0x7c92f037b1f1b135ull,
     {0x8d,0x95,0x8c,0,0,0,0x4c,0x8d,0x85,0xf8,1,0,0,0xc5,0xf8,0x77}}
};
inline bool MatchesSwingPropertyReader(uintptr_t base, uint32_t size, const SwingPropertyReaderVariant& v) {
    __try {
        if (!base || v.reader >= v.end || v.end > size || v.callReturn > size ||
            v.callReturn < 5 + sizeof(v.callArguments)) return false;
        const auto* bytes = reinterpret_cast<const unsigned char*>(base + v.reader);
        if (!Executable(bytes) || !ExactFunction(base, v.reader, v.end) ||
            !RelativeCall(base, v.callReturn, v.reader) ||
            std::memcmp(reinterpret_cast<void*>(base + v.callReturn - 5 - sizeof(v.callArguments)),
                        v.callArguments, sizeof(v.callArguments))) return false;
        uint64_t hash = 14695981039346656037ull;
        for (uint32_t i = 0; i < v.end - v.reader; ++i) hash = (hash ^ bytes[i]) * 1099511628211ull;
        return hash == v.fingerprint;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
inline const SwingPropertyReaderVariant* VerifySwingPropertyReader(HMODULE module, uintptr_t swing) {
    const auto base = reinterpret_cast<uintptr_t>(module);
    const auto size = ImageSize(module);
    for (const auto& v : SwingPropertyReaders)
        if (swing == base + v.swing && MatchesSwingPropertyReader(base, size, v)) return &v;
    return nullptr;
}
inline bool ClearSwingSceneForces(void* output) {
    if (!output) return false;
    __try {
        // Leave the eight motion/solver weights and final prewarm count intact.
        std::memset(static_cast<unsigned char*>(output) + SwingSceneForceOffset, 0, SwingSceneForceSize);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}
} // namespace gbb::profile
