#ifdef BUILDING_STELLAR_DLL
#define STELLAR_API __declspec(dllexport)
#else
#define STELLAR_API __declspec(dllimport)
#endif

#include "StellarRender.h"

StellarRender::StellarRender(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC scd = { 0 };
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferDesc.Width = 800;
    scd.BufferDesc.Height = 600;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    backBuffer = NULL;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, NULL, &context);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create device and swap chain!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    ID3D11Texture2D* pBackBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to get back buffer!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    hr = device->CreateRenderTargetView(pBackBuffer, NULL, &backBuffer);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create render target view!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    pBackBuffer->Release();

    // Viewport
    D3D11_VIEWPORT viewport = { 0 };
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;

    context->RSSetViewports(1, &viewport);

    // Triangle
    XMFLOAT3 vertices[] = { 
        XMFLOAT3(0.0f, 0.5f, 0.0f), 
        XMFLOAT3(0.45f, -0.5f, 0.0f), 
        XMFLOAT3(-0.45f, -0.5f, 0.0f) 
    };

    D3D11_BUFFER_DESC bd = { 0 };
    ZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = sizeof(XMFLOAT3) * 3;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA srd = { vertices, 0, 0 };
    hr = device->CreateBuffer(&bd, &srd, &vertexBuffer);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create vertex buffer!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    // copy the vertices into the buffer
    //D3D11_MAPPED_SUBRESOURCE ms;
    //context->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    //memcpy(ms.pData, vertices, sizeof(vertices));                           // copy the data
    //context->Unmap(vertexBuffer, NULL);                                      // unmap the buffer
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.CullMode = D3D11_CULL_NONE;  // This disables back-face culling.
    rasterDesc.FillMode = D3D11_FILL_SOLID;

    // Create the rasterizer state from the description we just filled out.
    ID3D11RasterizerState* rasterState;
    hr = device->CreateRasterizerState(&rasterDesc, &rasterState);

    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create rasterizer state!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    // Now set the rasterizer state.
    context->RSSetState(rasterState);

    rasterState->Release();
    // Shaders
    ID3DBlob* VS;
    ID3DBlob* PS;
    ID3DBlob* errorBlob = nullptr;

    hr = D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "main", "vs_5_0", 0, 0, &VS, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), "Error!", MB_ICONEXCLAMATION | MB_OK);
            errorBlob->Release();
        }
        else
        {
            MessageBox(NULL, "Failed to compile vertex shader!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        }
        return;
    }

    hr = D3DCompileFromFile(L"PixelShader.hlsl", 0, 0, "main", "ps_5_0", 0, 0, &PS, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), "Error!", MB_ICONEXCLAMATION | MB_OK);
            errorBlob->Release();
        }
        else
        {
            MessageBox(NULL, "Failed to compile pixel shader!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        }
        return;
    }

    hr = device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &vertexShader);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create vertex shader!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    hr = device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pixelShader);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create pixel shader!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    context->VSSetShader(vertexShader, 0, 0);
    context->PSSetShader(pixelShader, 0, 0);

    // Vertex input layout
    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = device->CreateInputLayout(ied, 1, VS->GetBufferPointer(), VS->GetBufferSize(), &inputLayout);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create input layout!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    context->IASetInputLayout(inputLayout);

    VS->Release();
    PS->Release();
}

StellarRender::~StellarRender()
{
    // Clean up D3D objects.
    // If the objects were properly created, they will not be null.

    if (vertexBuffer) vertexBuffer->Release();
    if (inputLayout) inputLayout->Release();
    if (pixelShader) pixelShader->Release();
    if (vertexShader) vertexShader->Release();
    if (backBuffer) backBuffer->Release();
    if (context) context->Release();
    if (device) device->Release();
    if (swapChain) swapChain->Release();
}

void StellarRender::Render()
{
    float clearColor[4] = { 0.529f, 0.808f, 0.922f, 1.0f };  // Sky Blue color
    //context->ClearRenderTargetView(backBuffer, clearColor);

    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->Draw(3, 0);

    swapChain->Present(0, 0);
}
