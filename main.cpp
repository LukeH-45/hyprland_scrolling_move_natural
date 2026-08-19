#define WLR_USE_UNSTABLE

#include <unistd.h>
#include <any>
#include <cmath>
#include <string>

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/values/ConfigValues.hpp>
#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/managers/input/trackpad/gestures/ITrackpadGesture.hpp>

inline HANDLE PHANDLE;
static SP<Config::Values::CBoolValue> g_pNatural;

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

inline CFunctionHook* g_pUpdateHook = nullptr;
typedef void (*origUpdate)(void*, const ITrackpadGesture::STrackpadGestureUpdate&);

void hkUpdate(void* thisptr, const ITrackpadGesture::STrackpadGestureUpdate& e) {
    if (!g_pNatural->value()) {
        (*(origUpdate)g_pUpdateHook->m_original)(thisptr, e);
        return;
    }

    IPointer::SSwipeUpdateEvent modifiedSwipe;
    if (e.swipe) {
        modifiedSwipe      = *e.swipe;
        modifiedSwipe.delta = e.swipe->delta * -1.F;
    }

    ITrackpadGesture::STrackpadGestureUpdate modifiedE = e;
    modifiedE.swipe = e.swipe ? &modifiedSwipe : nullptr;

    (*(origUpdate)g_pUpdateHook->m_original)(thisptr, modifiedE);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH        = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[scroll-move-natural] Failure in initialization: Version mismatch (headers ver is not equal to running hyprland ver)",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[scroll-move-natural] Version mismatch");
    }

    g_pNatural = Config::Values::makeConfigValue<Config::Values::CBoolValue>("plugin:scroll_move_natural:natural", "Inverts scroll_move gesture direction for natural scrolling", false);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pNatural);

    HyprlandAPI::reloadConfig();

    static const auto METHODS = HyprlandAPI::findFunctionsByName(PHANDLE, "update");
    if (METHODS.empty()) {
        HyprlandAPI::addNotification(PHANDLE, "[scroll-move-natural] Failed to find CScrollMoveTrackpadGesture::update",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[scroll-move-natural] Failed to find update function");
    }

    const void* targetAddr = nullptr;
    for (const auto& m : METHODS) {
        if (m.demangled.contains("ScrollMoveTrackpadGesture")) {
            targetAddr = m.address;
            break;
        }
    }

    if (!targetAddr) {
        HyprlandAPI::addNotification(PHANDLE, "[scroll-move-natural] Failed to find CScrollMoveTrackpadGesture::update among candidates",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[scroll-move-natural] Failed to find update function");
    }

    g_pUpdateHook = HyprlandAPI::createFunctionHook(handle, targetAddr, (void*)&hkUpdate);
    g_pUpdateHook->hook();

    HyprlandAPI::addNotification(PHANDLE, "[scroll-move-natural] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);

    return {"scroll-move-natural", "Inverts scroll_move gesture direction for natural scrolling", "plugin", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (g_pUpdateHook)
        g_pUpdateHook->unhook();
}
