#include <d3d9.h>
#include <DirectXMath.h>

struct DXVec2 {
    float x, y;

    DXVec2() : x(0), y(0) {}
    DXVec2(float x, float y) : x(x), y(y) {}

    DXVec2 operator+(const DXVec2& other) const {
        return DXVec2(x + other.x, y + other.y);
    }

    DXVec2 operator-(const DXVec2& other) const {
        return DXVec2(x - other.x, y - other.y);
    }

    DXVec2 operator*(float scalar) const {
        return DXVec2(x * scalar, y * scalar);
    }

    DXVec2 operator*(const DXVec2& other) const {
        return DXVec2(x * other.x, y * other.x);
    }

    DXVec2 operator/(float scalar) const {
        return DXVec2(x / scalar, y / scalar);
    }

    DXVec2 operator/(const DXVec2& other) const {
        return DXVec2(x / other.x, y / other.x);
    }


};



static void RenderRectangle(IDirect3DDevice9* device, DXVec2 position, DXVec2 size, D3DCOLOR color) {
    IDirect3DVertexBuffer9* vertexBuffer;
    device->CreateVertexBuffer(4 * sizeof(DXVec2), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL);

    DXVec2* vertices;
    vertexBuffer->Lock(0, 0, (void**)&vertices, 0);

    // add the vertices that form the rectangle
    vertices[0] = position;
    vertices[1] = position + DXVec2(size.x, 0);
    vertices[2] = position + size;
    vertices[3] = position + DXVec2(0, size.y);

    for (int i = 0; i < 4; i++) {
        *((DWORD*)&vertices[i] + 1) = color;
    }

    vertexBuffer->Unlock();

    // set the render states for rendering the rectangle
    device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
    device->SetStreamSource(0, vertexBuffer, 0, sizeof(DXVec2));
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

    // draw the rectangle
    device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

    vertexBuffer->Release();
}

static void RenderCircle(IDirect3DDevice9* device, DXVec2 center, float radius, D3DCOLOR color) {
    const int NUM_SEGMENTS = 32; // the number of vertices in the circle
    const int VERTEX_SIZE = sizeof(DXVec2) + sizeof(D3DCOLOR); // size of each vertex
    const float ANGLE_INCREMENT = 3.14159f * 2.0f / NUM_SEGMENTS; // the angle between each vertex
    const int NUM_VERTICES = NUM_SEGMENTS + 2; // add two extra vertices to close the circle

    IDirect3DVertexBuffer9* vertexBuffer;
    device->CreateVertexBuffer(NUM_VERTICES * VERTEX_SIZE, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL);

    DXVec2* vertices;
    vertexBuffer->Lock(0, 0, (void**)&vertices, 0);

    // add the center vertex
    vertices[0] = center;
    *((DWORD*)&vertices[0] + 1) = color;

    // add the vertices that form the circle
    for (int i = 1; i <= NUM_SEGMENTS; i++) {
        float angle = ANGLE_INCREMENT * i;
        vertices[i] = center + DXVec2(cosf(angle), sinf(angle)) * radius;
        *((DWORD*)&vertices[i] + 1) = color;
    }

    // add the final vertex to close the circle
    vertices[NUM_VERTICES - 1] = vertices[1];
    *((DWORD*)&vertices[NUM_VERTICES - 1] + 1) = color;

    vertexBuffer->Unlock();

    // set the render states for rendering the circle
    device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
    device->SetStreamSource(0, vertexBuffer, 0, VERTEX_SIZE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

    // draw the circle
    device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, NUM_SEGMENTS);

    vertexBuffer->Release();
}
