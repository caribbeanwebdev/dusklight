// Web (Emscripten) bridge for the on-screen touch controls.
//
// The page-side overlay (platforms/web/shell.html) tracks touches on a virtual
// stick and buttons and injects the combined state into aurora's virtual pad
// for port 1. PADRead() merges it with any physical/keyboard input. These are
// called from the browser's main thread via Module.ccall while the game loop
// runs on the proxied main pthread; PADStatus is plain shared memory and the
// occasional torn read of an input frame is harmless.
#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>

#include <dolphin/pad.h>

extern "C" {

EMSCRIPTEN_KEEPALIVE void dusk_web_virtual_pad(unsigned int buttons, int stickX, int stickY,
                                               int substickX, int substickY, int triggerLeft,
                                               int triggerRight) {
    PADStatus status{};
    status.button = static_cast<u16>(buttons);
    status.stickX = static_cast<s8>(stickX);
    status.stickY = static_cast<s8>(stickY);
    status.substickX = static_cast<s8>(substickX);
    status.substickY = static_cast<s8>(substickY);
    status.triggerLeft = static_cast<u8>(triggerLeft);
    status.triggerRight = static_cast<u8>(triggerRight);
    status.err = PAD_ERR_NONE;
    PADSetVirtualStatus(0, &status);
}

EMSCRIPTEN_KEEPALIVE void dusk_web_virtual_pad_clear(void) { PADClearVirtualStatus(0); }

} // extern "C"

#endif // __EMSCRIPTEN__
