#include <d3d9.h>
//#include <d3dx9.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx9.h"


class DX9Viewport {
public:
    DX9Viewport(IDirect3DDevice9* device, int width, int height)
        : m_pRenderTexture(NULL), m_pRenderTextureSurface(NULL), m_Width(width), m_Height(height) {
        device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pRenderTexture, NULL);
        m_pRenderTexture->GetSurfaceLevel(0, &m_pRenderTextureSurface);
      
    }

    ~DX9Viewport() {
        if (m_pRenderTextureSurface) {
            m_pRenderTextureSurface->Release();
            m_pRenderTextureSurface = NULL;
        }
        if (m_pRenderTexture) {
            m_pRenderTexture->Release();
            m_pRenderTexture = NULL;
        }
    }

    void ResizeViewport() {

    }

    void BeginRender(IDirect3DDevice9* device) {
        device->GetRenderTarget(0, &oldRenderTarget);
        device->SetRenderTarget(0, m_pRenderTextureSurface);
        device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        device->BeginScene();
    }

    void EndRender(IDirect3DDevice9* device) {
        device->EndScene();
        device->SetRenderTarget(0, oldRenderTarget);
    }

    void RenderImGui() {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoTitleBar;

        ImGui::Begin("Viewport", NULL, window_flags);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::Image(m_pRenderTexture, ImVec2(m_Width, m_Height));
        ImGui::End();
    }

private:
    IDirect3DTexture9* m_pRenderTexture;
    IDirect3DSurface9* m_pRenderTextureSurface;
    IDirect3DSurface9* oldRenderTarget = NULL;

    int m_Width, m_Height;
};
