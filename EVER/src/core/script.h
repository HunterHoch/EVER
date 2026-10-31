#pragma once

#define NOMINMAX

#include <d3d11.h>
#include <dxgi.h>
#include <future>
#include <queue>
#include <wrl/client.h>

using namespace Microsoft::WRL;
    template <class T> void SafeRelease(T** ppT) {
        if (*ppT) {
            (*ppT)->Release();
            *ppT = NULL;
        }
    }

    template <class B, class A> B ForceCast(A a) {
        union {
            A a;
            B b;
        } x;
        x.a = a;
        return x.b;
    }

    namespace ever {
    void initialize();
    void ScriptMain();
    void OnPresent(IDXGISwapChain* p_swap_chain);
    void finalize();
    bool isExportActive();
}
