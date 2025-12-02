#include "framework.h"

#include "shader.h"
#include "texture.h"
// ADAPTER
#include <dxgi.h>
#undef min
#undef max
#include <algorithm>
#define BT_NO_SIMD_OPERATOR_OVERLOADS
#include <bullet/btBulletDynamicsCommon.h>

#include<random>

int menu_count = 0;

const int CheckMenu(const int* a, const int* b, int size);


framework::framework(HWND hwnd) : hwnd(hwnd)
{
}

bool framework::initialize()
{
	HRESULT hr{ S_OK };

	// ADAPTER
	IDXGIFactory* factory;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	IDXGIAdapter* adapter;
	for (UINT adapter_index = 0; S_OK == factory->EnumAdapters(adapter_index, &adapter); ++adapter_index) {
		DXGI_ADAPTER_DESC adapter_desc;
		adapter->GetDesc(&adapter_desc);
		if (adapter_desc.VendorId == 0x1002/*AMD*/ || adapter_desc.VendorId == 0x10DE/*NVIDIA*/)
		{
			break;
		}
		adapter->Release();
	}
	factory->Release();

	UINT create_device_flags{ 0 };
#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_levels{ D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.BufferDesc.Width = SCREEN_WIDTH;
	swap_chain_desc.BufferDesc.Height = SCREEN_HEIGHT;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.SampleDesc.Count = 8;
	swap_chain_desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	swap_chain_desc.Windowed = !FULLSCREEN;
	hr = D3D11CreateDeviceAndSwapChain(adapter/*ADAPTER*/, D3D_DRIVER_TYPE_UNKNOWN/*ADAPTER*/, NULL, create_device_flags,
		&feature_levels, 1, D3D11_SDK_VERSION, &swap_chain_desc,
		&swap_chain, &device, NULL, &immediate_context);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// ADAPTER
	adapter->Release();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer{};
	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(back_buffer.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = device->CreateRenderTargetView(back_buffer.Get(), NULL, render_target_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

#if 0
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer{};
	D3D11_TEXTURE2D_DESC texture2d_desc{};
	texture2d_desc.Width = SCREEN_WIDTH;
	texture2d_desc.Height = SCREEN_HEIGHT;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2d_desc, NULL, depth_stencil_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = texture2d_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc, depth_stencil_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif // 0

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	immediate_context->RSSetViewports(1, &viewport);

	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.BorderColor[0] = 0;
	sampler_desc.BorderColor[1] = 0;
	sampler_desc.BorderColor[2] = 0;
	sampler_desc.BorderColor[3] = 0;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, sampler_states[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = device->CreateSamplerState(&sampler_desc, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = device->CreateSamplerState(&sampler_desc, sampler_states[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// UNIT.32
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.BorderColor[0] = 0;
	sampler_desc.BorderColor[1] = 0;
	sampler_desc.BorderColor[2] = 0;
	sampler_desc.BorderColor[3] = 0;
	hr = device->CreateSamplerState(&sampler_desc, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// UNIT.32
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.BorderColor[0] = 1;
	sampler_desc.BorderColor[1] = 1;
	sampler_desc.BorderColor[2] = 1;
	sampler_desc.BorderColor[3] = 1;
	hr = device->CreateSamplerState(&sampler_desc, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// RADIAL_BLUR
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.BorderColor[0] = 0;
	sampler_desc.BorderColor[1] = 0;
	sampler_desc.BorderColor[2] = 0;
	sampler_desc.BorderColor[3] = 0;
	hr = device->CreateSamplerState(&sampler_desc, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_CLAMP)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_OFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_ON)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_BLEND_DESC blend_desc{};
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = FALSE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::NONE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //D3D11_BLEND_ONE D3D11_BLEND_SRC_ALPHA
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::ADD)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO; //D3D11_BLEND_DEST_COLOR
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR; //D3D11_BLEND_SRC_COLOR
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blend_states[static_cast<size_t>(BLEND_STATE::MULTIPLY)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	// UNIT.21
	//rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.FrontCounterClockwise = TRUE;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0;
	rasterizer_desc.SlopeScaledDepthBias = 0;
	rasterizer_desc.DepthClipEnable = TRUE;
	rasterizer_desc.ScissorEnable = FALSE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::WIREFRAME)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizer_states[static_cast<size_t>(RASTER_STATE::WIREFRAME_CULL_NONE)].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// DYNAMIC_TEXTURE
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(scene_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffers[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// UNIT.32
	buffer_desc.ByteWidth = sizeof(parametric_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffers[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// RADIAL_BLUR
	buffer_desc.ByteWidth = sizeof(radial_blur_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffers[2].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	framebuffers[0] = std::make_unique<framebuffer>(device.Get(), 1920, 1080);
	framebuffers[1] = std::make_unique<framebuffer>(device.Get(), 1920, 1080);

	bit_block_transfer = std::make_unique<fullscreen_quad>(device.Get());
	create_ps_from_cso(device.Get(), "blur_ps.cso", pixel_shaders[1].GetAddressOf());

	// DYNAMIC_TEXTURE
	//skinned_meshes[0] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\hamburger_man.fbx");
	//skinned_meshes[1] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\load_hamburger.fbx");
	skinned_meshes[2] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Patty.fbx");
	skinned_meshes[3] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Shop.fbx");
	skinned_meshes[4] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Spautla.fbx");
	//skinned_meshes[5] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\bus.fbx");
	// BOUNDING_BOX
	skinned_meshes[6] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\cube.000.fbx");
	dynamic_texture = std::make_unique<framebuffer>(device.Get(), 512, 512);

	skinned_meshes[7] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Skewers.fbx");
	skinned_meshes[8] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Buns_up.fbx");
	skinned_meshes[9] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Buns_under.fbx");

	skinned_meshes[10] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\Cheese.fbx");

	// テクスチャ表示用シェーダー
	create_ps_from_cso(device.Get(), "dynamic_texture_ps.cso", effect_shaders[0].GetAddressOf());

	// 背景アニメーション用シェーダー
	create_ps_from_cso(device.Get(), "dynamic_background_ps.cso", effect_shaders[1].GetAddressOf());
	dynamic_background = std::make_unique<framebuffer>(device.Get(), 1920, 1080);

	// ブラー効果シェーダー
	create_ps_from_cso(device.Get(), "radial_blur_ps.cso", pixel_shaders[0].GetAddressOf());

	// BLOOM
	bloomer = std::make_unique<bloom>(device.Get(), 1920, 1080);
	create_ps_from_cso(device.Get(), "final_pass_ps.cso", pixel_shaders[0].ReleaseAndGetAddressOf());

	initBulletWorld();   // ★ 追加

	patties.clear();

	cheeses.clear();

	menu_count = 0;

	prepareSpawnList();


	return true;
}

void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
	//if (allSpawn)
	//{
	//	coolTimer += elapsed_time;

	//	// 0.1 秒経過ごとに発動
	//	const float interval = 0.1f;

	//	while (coolTimer >= interval && callCount >= SPAWN_MAX)
	//	{
	//		srand((unsigned int)time(nullptr));
	//		coolTimer -= interval;  // 残り時間を繰り越し
	//		switch (rand() % 2 == 0)
	//		{
	//		case 0:
	//			add_patty({ patty_spawn_x, 0.5f, patty_spawn_z }, patty_default_half_extents, patty_default_mass, patty_default_restitution);
	//			callCount++;
	//			break;

	//		case 1:
	//			add_cheese({ patty_spawn_x, 0.8f, patty_spawn_z }, cheese_default_half_extents, cheese_default_mass, cheese_default_restitution);
	//			callCount++;
	//			break;
	//		}
	//	}
	//	allSpawn = false;
	//}

	Spwan(elapsed_time);


	if (kusi.pos.y + 0.028 <= patty_ground_y) // +の数値はkusiの位置調整のため
	{
		kusi.kusi_End = true;
		kusi.move = false;
	}

	if (kusi.kusi_End)
	{
		gameEnd = CheckMenu(kusi_menu, karimenu, MENU_MAX);
	}

	camera_focus.y += wheel * 0.0005f;

	if (camera_focus.y < 0.283f)
	{
		camera_focus.y = 0.283f;
	}

	wheel = 0.0f; // 処理したらリセットする

	static bool prev_down = false;
	bool now_down = (GetAsyncKeyState(VK_DOWN) & 0x8000);

	if (now_down && !prev_down) { // 押した瞬間だけ反応
		kusi.move = !kusi.move;               // トグル
		hitStop = !hitStop;
	}

	prev_down = now_down; // 状態保存

	if (kusi.move)
	{
		kusi.pos.y -= 0.0005;
	}

	if (!bun_spawned_initial)
	{
		bun.exists = true;
		bun.pos = { -0.3f, 1.3f, -10.0f }; // だいたいPattyと同じ奥行き
		bun.vel = { 0.0f, 0.0f, 0.0f };
		bun.half_extents = { 0.020f, 0.011f, 0.020f }; // 好みで微調整可
		bun.mass = 1.0f;
		bun.restitution = 0.0f;
		bun_spawned_initial = true;
	}


	if (!patty_spawned_initial)
	{
		add_patty({ 0.0f, 1.3f, translation.z }, // 位置（必要なら調整）
			patty_default_half_extents,
			patty_default_mass,
			patty_default_restitution);
		patty_spawned_initial = true;
	}

	// Bullet 物理ステップ
	if (m_btWorld)
	{
		m_btWorld->setGravity(btVector3(0, patty_gravity_y, 0));


		// ★ 追加: パテの位置を Bullet のボディに同期させる
		if (!hitStop && m_pateBody)
		{
			// 前回位置を保存するための変数をstaticで用意（またはメンバ変数にする）
			static DirectX::XMFLOAT3 prev_pate_pos = pate_position;

			// 速度の計算: (現在位置 - 前回位置) / 経過時間
			if (elapsed_time > 0.0f)
			{
				btVector3 currentPos(pate_position.x, pate_position.y, pate_position.z);
				btVector3 prevPos(prev_pate_pos.x, prev_pate_pos.y, prev_pate_pos.z);

				// 速度ベクトル
				btVector3 velocity = (currentPos - prevPos) / elapsed_time;

				// Kinematic物体に速度を設定（これで衝突相手に勢いが伝わります）
				m_pateBody->setLinearVelocity(velocity);
			}

			// 位置の反映
			btTransform tr;
			tr.setIdentity();
			tr.setOrigin(btVector3(pate_position.x, pate_position.y, pate_position.z));
			m_pateBody->getMotionState()->setWorldTransform(tr);

			// 現在位置を保存
			prev_pate_pos = pate_position;
		}

		// 1/60固定ステップで回す例（最大サブステップ10回）
		m_btWorld->stepSimulation(elapsed_time, 10, 1.0f / 60.0f);

		// Bullet → 自前 struct へ反映
		for (auto& P : patties)
		{
			if (!P.body) continue;
			if (hitStop) continue;

			btTransform tr;
			P.body->getMotionState()->getWorldTransform(tr);

			btVector3 pos = tr.getOrigin();
			P.pos = { (float)pos.getX(), (float)pos.getY(), (float)pos.getZ() };

			btQuaternion q = tr.getRotation();
			P.rotQuat = { (float)q.x(), (float)q.y(), (float)q.z(), (float)q.w() };


			//if (StartTimer >= 2000)
			//{
			//	btVector3 vel = P.body->getLinearVelocity();
			//	vel.setX(0.0f);  // X方向を停止
			//	vel.setZ(0.0f);  // Z方向を停止
			//	P.body->setLinearVelocity(vel);
			//}
			//////////////////////////////バグを出す原因かも
			//resolve_pate_collision(P);

			////// ★ 修正済みの位置
			//tr.setOrigin(btVector3(P.pos.x, P.pos.y, P.pos.z));
			//P.body->setWorldTransform(tr);
			//P.body->getMotionState()->setWorldTransform(tr);
			////P.body->setLinearVelocity(btVector3(P.vel.x, P.vel.y, P.vel.z));

			/////////////////////////////////////
		}

		for (auto& C : cheeses)
		{
			if (!C.body) continue;
			if (hitStop) continue;

			// 串に刺さっている場合は、串の動きに追従させるなどの処理が必要だが
			// ここでは物理演算の結果を取得するだけにする（Stuck時は物理無効化などを想定）
			if (C.isStuck) {
				// 串と一緒に動かすならここに追従処理
				// C.pos.y = kusi.pos.y + offset... 等
				continue;
			}

			btTransform tr;
			C.body->getMotionState()->getWorldTransform(tr);
			btVector3 pos = tr.getOrigin();
			C.pos = { (float)pos.getX(), (float)pos.getY(), (float)pos.getZ() };
			btQuaternion q = tr.getRotation();
			C.rotQuat = { (float)q.x(), (float)q.y(), (float)q.z(), (float)q.w() };

			//if (StartTimer >= 2000)
			//{
			//	btVector3 vel = C.body->getLinearVelocity();
			//	vel.setX(0.0f);  // X方向を停止
			//	vel.setZ(0.0f);  // Z方向を停止
			//	C.body->setLinearVelocity(vel);
			//}
		}
	}



#ifdef USE_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

#ifdef USE_IMGUI
	ImGui::Begin("ImGUI");
	calculate_frame_stats(); // フレームごとに呼ぶ

	if (gameEnd == 0)
		ImGui::Text("gameEnd = 0 : Defult");
	else if (gameEnd == 1)
		ImGui::Text("gameEnd = 1 : Game Clear");
	else if (gameEnd == 2)
		ImGui::Text("gameEnd = 2 : Game Over");

		ImGui::Text("gameEnd = 2 : Game Over");

		ImGui::Text("StartTimer: %.2f", StartTimer);

	ImGui::Checkbox("flat_shading", &flat_shading);
	ImGui::Checkbox("Enable Dynamic Shader", &enable_dynamic_shader);
	ImGui::Checkbox("Enable Dynamic Background", &enable_dynamic_background);
	//ImGui::Checkbox("Enable RADIAL_BLUR", &enable_radial_blur);
	ImGui::Checkbox("Enable Bloom", &enable_bloom);

	ImGui::SliderFloat("light_direction.x", &light_direction.x, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.y", &light_direction.y, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.z", &light_direction.z, -1.0f, +1.0f);

	// UNIT.32
	ImGui::SliderFloat("extraction_threshold", &parametric_constants.extraction_threshold, +0.0f, +5.0f);
	ImGui::SliderFloat("gaussian_sigma", &parametric_constants.gaussian_sigma, +0.0f, +10.0f);
	ImGui::SliderFloat("exposure", &parametric_constants.exposure, +0.0f, +10.0f);

	// RADIAL_BLUR
	ImGui::DragFloat2("blur_center", &radial_blur_data.blur_center.x, 0.01f);
	ImGui::SliderFloat("blur_strength", &radial_blur_data.blur_strength, +0.0f, +1.0f);
	ImGui::SliderFloat("blur_radius", &radial_blur_data.blur_radius, +0.0f, +1.0f);
	ImGui::SliderFloat("blur_decay", &radial_blur_data.blur_decay, +0.0f, +1.0f);

	// BLOOM
	ImGui::SliderFloat("bloom_extraction_threshold", &bloomer->bloom_extraction_threshold, +0.0f, +5.0f);
	ImGui::SliderFloat("bloom_intensity", &bloomer->bloom_intensity, +0.0f, +5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); // 内側の余白を増やす
	ImGui::SliderFloat("Camera Distance", &distance, 1.0f, 20.0f);
	ImGui::PopStyleVar();

	ImGui::Text("Stage Rotation");
	ImGui::SliderFloat("Rot X##obj3", &rotation_object3.x, -DirectX::XM_PI, DirectX::XM_PI);
	ImGui::SliderFloat("Rot Y##obj3", &rotation_object3.y, -DirectX::XM_PI, DirectX::XM_PI);
	ImGui::SliderFloat("Rot Z##obj3", &rotation_object3.z, -DirectX::XM_PI, DirectX::XM_PI);
	ImGui::SliderFloat("Pos X##obj3", &translation_object3.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Pos Y##obj3", &translation_object3.y, -10.0f, 10.0f);
	ImGui::SliderFloat("Pos Z##obj3", &translation_object3.z, -10.0f, 10.0f);

	ImGui::Text("Camera Focus (Buttonで微調整)");
	ImGui::SliderFloat("Focus X", &camera_focus.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Focus Y", &camera_focus.y, 0.0f, 10.0f);
	ImGui::SliderFloat("Focus Z", &camera_focus.z, -10.0f, 10.0f);

	ImGui::Separator();
	ImGui::TextUnformatted("[Patty System]");
	ImGui::Checkbox("Simulate", &patty_sim_enabled);
	ImGui::SliderFloat("Gravity Y", &patty_gravity_y, -50.0f, 0.0f, "%.2f");
	ImGui::SliderFloat("Ground Y", &patty_ground_y, -5.0f, 5.0f, "%.3f");

	ImGui::SliderFloat("Default Restitution", &patty_default_restitution, 0.0f, 1.0f);
	ImGui::SliderFloat("Patty Spawn X", &patty_spawn_x, -10.0f, 10.0f, "%.3f");

	if (ImGui::Button("Add 1 Patty"))
	{
		add_patty({ patty_spawn_x, 0.5f, patty_spawn_z }, patty_default_half_extents, patty_default_mass, patty_default_restitution);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Patties")) { patties.clear(); }
	ImGui::Text("Count: %d", (int)patties.size());

	if (ImGui::Button("Add 1 Cheese"))
	{
		add_cheese({ patty_spawn_x, 0.8f, patty_spawn_z }, cheese_default_half_extents, cheese_default_mass, cheese_default_restitution);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Cheeses")) {
		// 本当はBulletからも削除が必要だが簡易的にクリア
		cheeses.clear();
	}


	ImGui::Separator();
	ImGui::TextUnformatted("[Pate Collider]");
	ImGui::Checkbox("Enable Pate Collider", &pate_collider_enabled);
	ImGui::SliderFloat("Pate plane Y", &pate_plane_y, -1.0f, 3.0f, "%.3f");
	ImGui::SliderFloat("Pate half X (width/2)", &pate_half_extents.x, 0.05f, 2.0f, "%.3f");
	ImGui::SliderFloat("Pate half Y (thickness/2)", &pate_half_extents.y, 0.01f, 1.0f, "%.3f");
	ImGui::SliderFloat("Pate half Z (depth/2)", &pate_half_extents.z, 0.05f, 2.0f, "%.3f");
	ImGui::SliderFloat("Pate restitution", &pate_restitution, 0.0f, 1.0f);
	ImGui::Text("Pate pos = (%.2f, %.2f, %.2f)", pate_position.x, pate_position.y, pate_position.z);
	//ImGui::SliderFloat("pate_follow_smooth", &pate_follow_smooth, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::TextUnformatted("[Patty Z Controls]");
	ImGui::Checkbox("Show Patty Collider", &patty_collider_visible);
	// 生成時のZ
	ImGui::DragFloat3("Default HalfExtents (hx,hy,hz)",
		&patty_default_half_extents.x, 0.001f, 0.001f, 1.0f, "%.3f");

	// 既存すべてを一括シフト（差分適用）
	if (ImGui::SliderFloat("All Patties Z (offset)", &patties_z_offset, -10.0f, 10.0f, "%.3f"))
	{
		float dz = patties_z_offset - patties_z_offset_prev;
		for (auto& P : patties) P.pos.z += dz;
		patties_z_offset_prev = patties_z_offset;
	}

	// 1体だけ直接編集
	if (!patties.empty())
	{
		ImGui::SliderInt("Select Patty", &patty_selected_index, 0, (int)patties.size() - 1);
		if (patty_selected_index >= 0 && patty_selected_index < (int)patties.size())
		{
			ImGui::SliderFloat("Selected Patty Z", &patties[patty_selected_index].pos.z, -10.0f, 10.0f, "%.3f");
		}
	}

	if (!patties.empty())
	{
		ImGui::SliderInt("Select Patty", &patty_selected_index, 0, (int)patties.size() - 1);

		if (patty_selected_index >= 0 && patty_selected_index < (int)patties.size())
		{
			auto& S = patties[patty_selected_index];

			// 入力 UI（ドラッグ操作で微調整しやすく）
			ImGui::DragFloat("Selected Patty X", &S.pos.x, 0.01f, -10.0f, 10.0f, "%.3f");
			ImGui::DragFloat("Selected Patty Y", &S.pos.y, 0.01f, -1.0f, 5.0f, "%.3f");
			ImGui::DragFloat("Selected Patty Z", &S.pos.z, 0.01f, -10.0f, 10.0f, "%.3f");

			// 視覚化用の現在値表示
			ImGui::Text("Pos = (%.3f, %.3f, %.3f)", S.pos.x, S.pos.y, S.pos.z);
		}
	}

	ImGui::Separator();
	ImGui::TextUnformatted("[Pate Control]");
	ImGui::SliderFloat("Pate fixed X", &pate_fixed_x, -10.0f, 10.0f, "%.3f");
	ImGui::SliderFloat("Pate fixed Z", &pate_fixed_z, -10.0f, 10.0f, "%.3f");

	ImGui::TextUnformatted("Camera Orientation");
	ImGui::SliderAngle("Yaw (rotateY)", &rotateY, -180.0f, 180.0f);
	ImGui::SliderAngle("Pitch (rotateX)", &rotateX, -89.0f, 89.0f);



	ImGui::SliderFloat("kusi Y", &kusi.pos.y, -30.0f, 30.0f, "%.3f");

	ImGui::Separator();
	ImGui::TextUnformatted("[Buns]");
	ImGui::Checkbox("Show Bun Collider", &bun_collider_visible);

	ImGui::Separator();
	ImGui::TextUnformatted("[Buns]");

	// 当たり可視化の切替（既存の可視化に連動）
	ImGui::Checkbox("Show Bun Collider", &bun_collider_visible);

	ImGui::DragFloat3("Buns position (X,Y,Z)", &bun.pos.x, 0.01f, -20.0f, 20.0f, "%.3f");

	if (ImGui::DragFloat3("Buns collider half extents", &bun.half_extents.x, 0.005f, 0.0f, 5.0f, "%.3f")) {
		bun.half_extents.x = std::max(0.0f, bun.half_extents.x);
		bun.half_extents.y = std::max(0.0f, bun.half_extents.y);
		bun.half_extents.z = std::max(0.0f, bun.half_extents.z);
	}

	// プリセット（ワンボタンで大きさをサクッと変更したい人類向け）
	if (ImGui::Button("Small"))  bun.half_extents = { 0.06f, 0.03f, 0.06f };
	ImGui::SameLine();
	if (ImGui::Button("Medium")) bun.half_extents = { 0.10f, 0.04f, 0.10f };
	ImGui::SameLine();
	if (ImGui::Button("Large"))  bun.half_extents = { 0.14f, 0.06f, 0.14f };
	ImGui::End();

	ImGui::Begin("Hit Check");
	for (int i = 0; i < patties.size(); ++i)
	{
		ImGui::Text("Patty %d : %s", i,
			patties[i].isStuck ? "STUCK" :
			patties[i].isColliding ? "HIT" : "FREE");
	}
	for (int i = 0; i < cheeses.size(); ++i)
	{
		ImGui::Text("Cheese %d : %s", i,
			cheeses[i].isStuck ? "STUCK" :
			cheeses[i].isColliding ? "HIT" : "FREE");
	}
	ImGui::End();

	ImGui::Begin("Menu Debug");

	for (int i = 0; i < 10; ++i) {
		ImGui::Text("kusi menu %d : %d", i, kusi_menu[i]);
	}

	ImGui::End();

	ImGui::Begin("Menu");

	for (int i = 0; i < 10; ++i) {
		ImGui::Text("menu %d : %d", i, karimenu[i]);
	}

	ImGui::End();


#endif
	// ★ ここを追加：毎フレーム物理更新
	//simulate_patties(elapsed_time);

	simulate_bun(elapsed_time);
	CheckKusiCheeseCollision();
	CheckKusiPattyCollision();
	//CheckPateCollision();
	StartTimer++;
}
void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	HRESULT hr{ S_OK };

	// UNIT.32
	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	immediate_context->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	immediate_context->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	immediate_context->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);

	FLOAT color[]{ 0.2f, 0.2f, 0.2f, 1.0f };
	immediate_context->ClearRenderTargetView(render_target_view.Get(), color);
#if 0
	immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_zCLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif // 0
	immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	immediate_context->PSSetSamplers(0, 1, sampler_states[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
	immediate_context->PSSetSamplers(1, 1, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
	immediate_context->PSSetSamplers(2, 1, sampler_states[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
	// UNIT.32
	immediate_context->PSSetSamplers(3, 1, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
	immediate_context->PSSetSamplers(4, 1, sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());

	immediate_context->OMSetBlendState(blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);
	immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());

	// 追加：skinned_meshes[4] の world 行列を保存する（バウンディングボックス用）
	DirectX::XMFLOAT4X4 world_obj4 = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	D3D11_VIEWPORT viewport;
	UINT num_viewports{ 1 };
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	float aspect_ratio{ viewport.Width / viewport.Height };
	DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30), aspect_ratio, 0.1f, 100.0f) };
	DirectX::XMFLOAT3 center_of_rotation = camera_position; // 起動時のカメラ位置を中心にする

	DirectX::XMMATRIX V;
	{
		DirectX::XMVECTOR up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

		float sx = ::sinf(rotateX), cx = ::cosf(rotateX);
		float sy = ::sinf(rotateY), cy = ::cosf(rotateY);
		DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&camera_focus);
		DirectX::XMVECTOR Front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
		DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance, distance, distance, 0.0f);
		Front = DirectX::XMVectorScale(Front, distance);
		DirectX::XMVECTOR Eye = DirectX::XMVectorSubtract(Focus, Front);
		DirectX::XMStoreFloat3(&camera_position, Eye);
		V = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&camera_position),
			DirectX::XMLoadFloat3(&camera_focus),
			up);
	}

	//ワールドへの変換
	{
		const float vx = static_cast<float>(mouse_client_pos.x);
		const float vy = static_cast<float>(mouse_client_pos.y);

		using namespace DirectX;
		XMVECTOR nearPt = XMVector3Unproject(
			XMVectorSet(vx, vy, 0.0f, 1.0f),
			0.0f, 0.0f, viewport.Width, viewport.Height,
			0.0f, 1.0f,
			P, V, XMMatrixIdentity());
		XMVECTOR farPt = XMVector3Unproject(
			XMVectorSet(vx, vy, 1.0f, 1.0f),
			0.0f, 0.0f, viewport.Width, viewport.Height,
			0.0f, 1.0f,
			P, V, XMMatrixIdentity());
		XMVECTOR dir = XMVector3Normalize(farPt - nearPt);

		// --- ここから差し替え（YZ移動: X=const の平面と交差）---
		const float planeX = pate_fixed_x;
		const float nearX = XMVectorGetX(nearPt);
		const float dirX = XMVectorGetX(dir);

		if (fabsf(dirX) > 1e-6f) {
			const float t = (planeX - nearX) / dirX;
			XMVECTOR hit = XMVectorMultiplyAdd(dir, XMVectorReplicate(t), nearPt);
			XMFLOAT3 hit3{}; XMStoreFloat3(&hit3, hit);

			// X は固定、Y/Z を目標値に
			pate_target.x = planeX;
			pate_target.y = hit3.y;
			pate_target.z = hit3.z;
		}

		// 追従スムージング：Y/Z のみ補間、X は固定
		auto lerp = [](float a, float b, float s) { return a + (b - a) * s; };
		pate_position.y = lerp(pate_position.y, pate_target.y, pate_follow_smooth);
		pate_position.z = lerp(pate_position.z, pate_target.z, pate_follow_smooth);
		pate_position.x = planeX;
	}

	//scene_constants data{};
	DirectX::XMStoreFloat4x4(&data.view_projection, V * P);
	data.light_direction = light_direction;
	// UNIT.16

	// DYNAMIC_TEXTURE
	data.elapsed_time = elapsed_time;
	data.time += elapsed_time;

	immediate_context->UpdateSubresource(constant_buffers[0].Get(), 0, 0, &data, 0, 0);
	immediate_context->VSSetConstantBuffers(1, 1, constant_buffers[0].GetAddressOf());
	// UNIT.16
	immediate_context->PSSetConstantBuffers(1, 1, constant_buffers[0].GetAddressOf());
	// UNIT.32
	immediate_context->UpdateSubresource(constant_buffers[1].Get(), 0, 0, &parametric_constants, 0, 0);
	immediate_context->PSSetConstantBuffers(2, 1, constant_buffers[1].GetAddressOf());

	// DYNAMIC_TEXTURE
	if (enable_dynamic_shader)
	{
		dynamic_texture->clear(immediate_context.Get());
		dynamic_texture->activate(immediate_context.Get());
		immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		bit_block_transfer->blit(immediate_context.Get(), nullptr, 0, 0, effect_shaders[0].Get());
		dynamic_texture->deactivate(immediate_context.Get());
		immediate_context->PSSetShaderResources(15, 1, dynamic_texture->shader_resource_views[0].GetAddressOf());
	}
	else
	{
		ID3D11ShaderResourceView* null_srv[] = { nullptr };
		immediate_context->PSSetShaderResources(15, 1, null_srv);
	}

	// RADIAL_BLUR
	if (enable_radial_blur)
	{
		immediate_context->UpdateSubresource(constant_buffers[1].Get(), 0, 0, &radial_blur_data, 0, 0);
		immediate_context->PSSetConstantBuffers(2, 1, constant_buffers[1].GetAddressOf());
	}

	// UNIT.32
	framebuffers[0]->clear(immediate_context.Get());
	framebuffers[0]->activate(immediate_context.Get());

	// DYNAMIC_BACKGROUND
	if (enable_dynamic_background)
	{
		immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		immediate_context->OMSetBlendState(blend_states[static_cast<size_t>(BLEND_STATE::NONE)].Get(), nullptr, 0xFFFFFFFF);
		bit_block_transfer->blit(immediate_context.Get(), nullptr, 0, 0, effect_shaders[1].Get());
	}

	immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());
	// UNIT.21
	const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
		{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
		{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
		{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
	};
#if 1
	const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#else
	const float scale_factor = 0.01f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
#endif
	const float spacing = 2.0f;

	DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };

	// UNIT.18
	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z) };
	DirectX::XMFLOAT4X4 world;
	// UNIT.21

	// === skinned_meshes[2]
	for (const auto& P : patties)
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(P.pos.x, P.pos.y - P.half_extents.y, P.pos.z);
		XMMATRIX S = XMMatrixScaling(0.34f, 0.34f, 0.34f);

		XMVECTOR q = XMLoadFloat4(&P.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);  // ★ パティ個別の回転

		XMFLOAT4X4 world;
		//XMStoreFloat4x4(&world, C * S * R * Rp * T);  // 好きな順で調整
		XMStoreFloat4x4(&world, C * S * Rp * T);
		skinned_meshes[2]->render(immediate_context.Get(), world, material_color, nullptr, flat_shading);
	}

	if (patty_collider_visible)
	{
		using namespace DirectX;

		// キューブFBXの実寸（1辺サイズ）を取得して正規化
		XMFLOAT3 cubeDim =
		{
			skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
			skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
			skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z
		};

		for (const auto& P : patties)
		{
			const float hx = P.half_extents.x;
			const float hy = P.half_extents.y;
			const float hz = P.half_extents.z;

			// Pattyのハーフエクステントに合わせてキューブをスケーリング
			XMMATRIX Sbox = XMMatrixScaling((2.0f * hx) / cubeDim.x,
				(2.0f * hy) / cubeDim.y,
				(2.0f * hz) / cubeDim.z);
			XMMATRIX Tbox = XMMatrixTranslation(P.pos.x, P.pos.y, P.pos.z);
			XMVECTOR q = XMLoadFloat4(&P.rotQuat);
			XMMATRIX Rp = XMMatrixRotationQuaternion(q);

			XMFLOAT4X4 world_box;
			//	XMStoreFloat4x4(&world_box, C* Sbox* R* Rp* Tbox);
			XMStoreFloat4x4(&world_box, C * Sbox * Rp * Tbox);

			// 半透明で描画（線ではなく中身あり。嫌ならアルファ上げ下げする）
			skinned_meshes[6]->render(immediate_context.Get(), world_box,
				{ 1.0f, 0.2f, 0.2f, 0.35f }, nullptr, true);
		}
	}



	//=== skinned_meshes[3]
	{
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
			translation_object3.x,
			translation_object3.y,
			translation_object3.z
		);
		DirectX::XMMATRIX Scale = DirectX::XMMatrixScaling(1, 1, 1);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y,
			rotation_object3.z
		);
		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, C * Scale * R * T);
		bool prev_flat = flat_shading;
		flat_shading = true; // Object3 はフラット描画
		skinned_meshes[3]->render(immediate_context.Get(), world, material_color, nullptr, flat_shading);
		flat_shading = prev_flat;
	}


	//// === skinned_meshes[4]（アニメーションなし）===
	///変更箇所

	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(pate_position.x, pate_position.y, pate_position.z);
		XMMATRIX Scale = XMMatrixScaling(0.01f, 0.01f, 0.01f);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y + 6.3,
			rotation_object3.z
		);

		XMFLOAT4X4 world_local;
		XMStoreFloat4x4(&world_local, C * Scale * R * T);

		// 描画に使う world を保存しておく（後でバウンディングボックス計算に使う）
		world_obj4 = world_local;

		skinned_meshes[4]->render(immediate_context.Get(), world_local, material_color, nullptr, flat_shading);
	}

	//// === skinned_meshes[7]（アニメーションなし）===
	///変更箇所
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(kusi.pos.x, kusi.pos.y, kusi.pos.z);
		XMMATRIX Scale = XMMatrixScaling(0.08f, 0.08f, 0.08f);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y,
			rotation_object3.z
		);

		XMFLOAT4X4 world_local;
		XMStoreFloat4x4(&world_local, C * Scale * R * T);

		// 描画に使う world を保存しておく（後でバウンディングボックス計算に使う）
		world_obj4 = world_local;

		skinned_meshes[7]->render(immediate_context.Get(), world_local, material_color, nullptr, flat_shading);
	}

	{
		using namespace DirectX;

		// ✅ まず、参照となる「キューブFBX(skinned_mesh[6])」の実寸を取得
		XMFLOAT3 cubeDim =
		{
			skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
			skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
			skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z
		};

		// ✅ 次に、Kusi（串？）のハーフエクステントを指定
		const float hx = kusi.half_extents.x;  // Xスケールに対応
		const float hy = kusi.half_extents.y;  // Yスケールに対応
		const float hz = kusi.half_extents.z;  // Zスケールに対応

		// ✅ Kusiのスケーリング行列を、実寸に基づいて調整
		XMMATRIX Sbox = XMMatrixScaling(
			(2.0f * hx) / cubeDim.x,
			(2.0f * hy) / cubeDim.y,
			(2.0f * hz) / cubeDim.z
		);

		// ✅ 位置・回転行列
		XMMATRIX T = XMMatrixTranslation(kusi.pos.x, kusi.pos.y, kusi.pos.z);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y,
			rotation_object3.z
		);

		// ✅ 最終的なワールド行列
		XMFLOAT4X4 world_box;
		XMStoreFloat4x4(&world_box, C * Sbox * R * T);

		// ✅ バウンディングボックス描画
		// 半透明（赤）で中身あり。線だけにしたければαを0.2以下に。
		skinned_meshes[6]->render(
			immediate_context.Get(),
			world_box,
			{ 0.2f, 1.0f, 0.2f, 1.0f },  // RGBA（赤系で透明度あり）
			nullptr,
			true
		);

		// ✅ 後で使う場合にワールド行列を保存（オプション）
		world_obj4 = world_box;
	}

	// === skinned_meshes[8] = buns.fbx を1体表示 ===
	if (bun.exists)
	{
		using namespace DirectX;
		// FBXはセンチ系のことが多いので、Pattyと同じくらいの見た目スケールに
		XMMATRIX T = XMMatrixTranslation(bun.pos.x, bun.pos.y, bun.pos.z);
		XMMATRIX S = XMMatrixScaling(0.14f, 0.14f, 0.14f); // 必要なら調整
		XMFLOAT4X4 world_bun;
		XMStoreFloat4x4(&world_bun, C * S * R * T);
		skinned_meshes[8]->render(immediate_context.Get(), world_bun, material_color, nullptr, flat_shading);
	}

	// （任意）Bunの当たりAABBを可視化
	if (bun.exists && bun_collider_visible)
	{
		using namespace DirectX;

		// キューブFBXの実寸（1辺サイズ）を取得
		XMFLOAT3 cubeDim =
		{
			skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
			skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
			skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z
		};

		XMMATRIX Sbox = XMMatrixScaling(
			(2.0f * bun.half_extents.x) / cubeDim.x,
			(2.0f * bun.half_extents.y) / cubeDim.y,
			(2.0f * bun.half_extents.z) / cubeDim.z);

		XMMATRIX Tbox = XMMatrixTranslation(-0.3f, bun.pos.y, bun.pos.z);

		XMFLOAT4X4 world_box;
		XMStoreFloat4x4(&world_box, C * Sbox * Tbox);
		skinned_meshes[6]->render(immediate_context.Get(), world_box, bun_collider_color, nullptr, true);
	}


	for (const auto& cheese : cheeses)
	{
		using namespace DirectX;
		// C.pos ではなく cheese.pos に変更 - P.half_extents.y
		XMMATRIX T = XMMatrixTranslation(cheese.pos.x, cheese.pos.y - cheese.half_extents.y, cheese.pos.z);
		XMMATRIX S = XMMatrixScaling(0.3f, 0.35f, 0.3f);

		// C.rotQuat ではなく cheese.rotQuat に変更
		XMVECTOR q = XMLoadFloat4(&cheese.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);

		XMFLOAT4X4 world;
		// ここにある 'C' は、外側で定義されている「座標変換行列」を指すようになるのでエラーが消えます
		//XMStoreFloat4x4(&world, C * S * R * Rp * T);

		XMStoreFloat4x4(&world, C * S * Rp * T);

		// skinned_meshes[10] が Cheese
		skinned_meshes[10]->render(immediate_context.Get(), world, material_color, nullptr, true);
	}


	if (patty_collider_visible) // フラグはPattyと共用
	{
		using namespace DirectX;
		XMFLOAT3 cubeDim = {
			skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
			skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
			skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z
		};

		// 変数名を 'C' から 'cheese' に変更して、行列 'C' と区別する
		for (const auto& cheese : cheeses)
		{
			const float hx = cheese.half_extents.x;
			const float hy = cheese.half_extents.y;
			const float hz = cheese.half_extents.z;

			XMMATRIX Sbox = XMMatrixScaling((2.0f * hx) / cubeDim.x, (2.0f * hy) / cubeDim.y, (2.0f * hz) / cubeDim.z);

			// cheese.pos を使用
			XMMATRIX Tbox = XMMatrixTranslation(cheese.pos.x, cheese.pos.y, cheese.pos.z);

			// cheese.rotQuat を使用
			XMVECTOR q = XMLoadFloat4(&cheese.rotQuat);
			XMMATRIX Rp = XMMatrixRotationQuaternion(q);

			XMFLOAT4X4 world_box;
			// これで 行列C * 行列Sbox... という正しい計算になります
		//	XMStoreFloat4x4(&world_box, C * Sbox * R * Rp * Tbox);
			XMStoreFloat4x4(&world_box, C * Sbox * Rp * Tbox);

			// 黄色っぽく表示
			skinned_meshes[6]->render(immediate_context.Get(), world_box, { 1.0f, 1.0f, 0.0f, 0.35f }, nullptr, true);
		}
	}

	// ★ Buns_under 描画
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(bunUnder.pos.x, bunUnder.pos.y, bunUnder.pos.z);
		XMMATRIX S = XMMatrixScaling(0.14f, 0.14f, 0.14f); // 他のモデルとスケールを合わせる

		XMVECTOR q = XMLoadFloat4(&bunUnder.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);

		XMFLOAT4X4 world;
		//XMStoreFloat4x4(&world, C * S * R * Rp * T);
		XMStoreFloat4x4(&world, C * S * Rp * T);
		// skinned_meshes[9] が Buns_under
		skinned_meshes[9]->render(immediate_context.Get(), world, material_color, nullptr, flat_shading);
	}

	// ★ Buns_under コライダー可視化 (bun_collider_visibleと連動させる例)
	if (bun_collider_visible)
	{
		using namespace DirectX;
		XMFLOAT3 cubeDim = {
			skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
			skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
			skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z
		};

		const float hx = bunUnder.half_extents.x;
		const float hy = bunUnder.half_extents.y;
		const float hz = bunUnder.half_extents.z;

		XMMATRIX Sbox = XMMatrixScaling((2.0f * hx) / cubeDim.x, (2.0f * hy) / cubeDim.y, (2.0f * hz) / cubeDim.z);
		XMMATRIX Tbox = XMMatrixTranslation(bunUnder.pos.x, bunUnder.pos.y, bunUnder.pos.z);

		XMFLOAT4X4 world_box;

		XMVECTOR q = XMLoadFloat4(&bunUnder.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);
		//XMStoreFloat4x4(&world_box, C * Sbox * R * Tbox); // 回転Rpは初期0なのでRのみ考慮でもOK

		XMStoreFloat4x4(&world_box, C * Sbox * Rp * Tbox);

		// 青っぽく表示
		skinned_meshes[6]->render(immediate_context.Get(), world_box, { 0.2f, 0.2f, 1.0f, 0.35f }, nullptr, true);
	}


	// BOUNDING_BOX
	{
		using namespace DirectX;

		// 0: Target model
		// 1: Bounding box model
		XMFLOAT3 dimensions[] = {
#if 0
			{
				skinned_meshes[4]->bounding_box[1].x - skinned_meshes[4]->bounding_box[0].x,
				skinned_meshes[4]->bounding_box[1].y - skinned_meshes[4]->bounding_box[0].y,
				skinned_meshes[4]->bounding_box[1].z - skinned_meshes[4]->bounding_box[0].z,
			},
#else
			{ 100.0f, 150.0f, 60.0f },
#endif
			{
				skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
				skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
				skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z,
			},
		};
		XMFLOAT3 centers[] = {
			{
				skinned_meshes[4]->bounding_box[0].x + (skinned_meshes[4]->bounding_box[1].x - skinned_meshes[4]->bounding_box[0].x) * 0.5f,
				skinned_meshes[4]->bounding_box[0].y + (skinned_meshes[4]->bounding_box[1].y - skinned_meshes[4]->bounding_box[0].y) * 0.5f,
				skinned_meshes[4]->bounding_box[0].z + (skinned_meshes[4]->bounding_box[1].z - skinned_meshes[4]->bounding_box[0].z) * 0.5f,
			},
			{
				skinned_meshes[6]->bounding_box[0].x + (skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x) * 0.5f,
				skinned_meshes[6]->bounding_box[0].y + (skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y) * 0.5f,
				skinned_meshes[6]->bounding_box[0].z + (skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z) * 0.5f,
			},
		};

		XMMATRIX S = XMMatrixScaling(dimensions[0].x / dimensions[1].x, dimensions[0].y / dimensions[1].y, dimensions[0].z / dimensions[1].z);
		XMMATRIX T = XMMatrixTranslation(centers[0].x - centers[1].x, centers[0].y - centers[1].y, centers[0].z - centers[1].z);

		// world_obj4 を使ってバウンディングボックスの最終 world 行列を作る
		DirectX::XMFLOAT4X4 transform;
		{
			// S と T は既に計算済み（あなたのコード内で定義されていることを前提）
			// もし S/T が同じブロック外に無ければ再計算してください（下に再掲あり）
			DirectX::XMMATRIX worldMat = DirectX::XMLoadFloat4x4(&world_obj4);
			DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(dimensions[0].x / dimensions[1].x,
				dimensions[0].y / dimensions[1].y,
				dimensions[0].z / dimensions[1].z);
			DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(centers[0].x - centers[1].x,
				centers[0].y - centers[1].y,
				centers[0].z - centers[1].z);

			XMStoreFloat4x4(&transform, scaleMat * transMat * worldMat);
		}
		}
	{
		using namespace DirectX;

		const float hx = pate_half_extents.x;
		const float hy = pate_half_extents.y;
		const float hz = pate_half_extents.z;

		XMFLOAT3 cubeDim =
		{
			skinned_meshes[6]->bounding_box[1].x - skinned_meshes[6]->bounding_box[0].x,
			skinned_meshes[6]->bounding_box[1].y - skinned_meshes[6]->bounding_box[0].y,
			skinned_meshes[6]->bounding_box[1].z - skinned_meshes[6]->bounding_box[0].z
		};

		XMMATRIX S = XMMatrixScaling((2.0f * hx) / cubeDim.x,
			(2.0f * hy) / cubeDim.y,
			(2.0f * hz) / cubeDim.z);

		XMMATRIX T = XMMatrixTranslation(pate_position.x, pate_position.y, pate_position.z);
		XMFLOAT4X4 world_pate;
		XMStoreFloat4x4(&world_pate, C * S * T);
		skinned_meshes[6]->render(immediate_context.Get(), world_pate, { 0.2f, 0.9f, 1.0f, 0.25f }, nullptr, /*flat_shading*/ true);
	}
	// UNIT.32
	framebuffers[0]->deactivate(immediate_context.Get());

	if (enable_bloom)
	{
		// BLOOM
		bloomer->make(immediate_context.Get(), framebuffers[0]->shader_resource_views[0].Get());

		immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		immediate_context->OMSetBlendState(blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);
		ID3D11ShaderResourceView* shader_resource_views[] =
		{
			framebuffers[0]->shader_resource_views[0].Get(),
			bloomer->shader_resource_view(),
		};
		bit_block_transfer->blit(immediate_context.Get(), shader_resource_views, 0, 2, pixel_shaders[0].Get());
	}

	if (enable_radial_blur)
	{
		// RADIAL_BLUR
		immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		ID3D11ShaderResourceView* shader_resource_views[]{ framebuffers[0]->shader_resource_views[0].Get() };
		bit_block_transfer->blit(immediate_context.Get(), shader_resource_views, 0, _countof(shader_resource_views), pixel_shaders[0].Get());
	}

	framebuffers[1]->clear(immediate_context.Get());
	framebuffers[1]->activate(immediate_context.Get());
	immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
	immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
	bit_block_transfer->blit(immediate_context.Get(), framebuffers[0]->shader_resource_views[0].GetAddressOf(), 0, 1, pixel_shaders[0].Get());
	framebuffers[1]->deactivate(immediate_context.Get());
#if 0
	immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
	bit_block_transfer->blit(immediate_context.Get(), framebuffers[1]->shader_resource_views[0].GetAddressOf(), 0, 1);
#endif

#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	UINT sync_interval{ 0 };
	swap_chain->Present(sync_interval, 0);
	}

const int CheckMenu(const int* a, const int* b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (a[i] != b[i]) return 2;
	}
	return 1;
}

void framework::add_patty(const DirectX::XMFLOAT3& p,
	const DirectX::XMFLOAT3& half_extents,
	float mass,
	float restitution)
{
	Patty obj{};
	obj.pos = p;
	obj.vel = { 0.0f, 0.0f, 0.0f };
	obj.half_extents = half_extents;
	obj.mass = mass;
	obj.restitution = restitution;

	// --- Bullet rigid body 作成 ---
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(p.x, p.y, p.z));

	btVector3 inertia(0, 0, 0);
	if (mass > 0.0f)
		m_pattyShape->calculateLocalInertia(mass, inertia);

	btDefaultMotionState* motion = new btDefaultMotionState(tr);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, m_pattyShape, inertia);
	info.m_restitution = restitution;

	info.m_friction = 0.5f;

	btRigidBody* body = new btRigidBody(info);


	if (isSpawningPhase)
	{
		// LinearFactor: 1=動く, 0=動かない (X, Y, Z) -> Yだけ動かす
		body->setLinearFactor(btVector3(0, 1, 0));

		// AngularFactor: 1=回転する, 0=回転しない -> 全てロック
		body->setAngularFactor(btVector3(0, 0, 0));
	}

	m_btWorld->addRigidBody(body);

	obj.body = body;

	patties.push_back(obj);
	m_pattyBodies.push_back(body);
}

void framework::resolve_pair(int ia, int ib)
{
	using namespace DirectX;
	Patty& A = patties[ia];
	Patty& B = patties[ib];

	// 中心差
	float dx = B.pos.x - A.pos.x;
	float dy = B.pos.y - A.pos.y;
	float dz = B.pos.z - A.pos.z;

	// 各軸のオーバーラップ量
	float ox = (A.half_extents.x + B.half_extents.x) - fabsf(dx);
	float oy = (A.half_extents.y + B.half_extents.y) - fabsf(dy);
	float oz = (A.half_extents.z + B.half_extents.z) - fabsf(dz);

	// 分離してたら終了
	if (ox <= 0.0f || oy <= 0.0f || oz <= 0.0f) return;

	// 最小貫入軸で解決
	float penetration;
	XMFLOAT3 n{ 0,0,0 }; // 法線

	if (ox <= oy && ox <= oz) { penetration = ox; n.x = (dx >= 0.0f) ? 1.0f : -1.0f; }
	else if (oy <= oz) { penetration = oy; n.y = (dy >= 0.0f) ? 1.0f : -1.0f; }
	else { penetration = oz; n.z = (dz >= 0.0f) ? 1.0f : -1.0f; }

	float invMa = (A.mass > 0.0f) ? 1.0f / A.mass : 0.0f;
	float invMb = (B.mass > 0.0f) ? 1.0f / B.mass : 0.0f;
	float invSum = invMa + invMb;
	if (invSum > 0.0f)
	{
		float moveA = (invMa / invSum) * penetration;
		float moveB = (invMb / invSum) * penetration;

		A.pos.x -= n.x * moveA;  A.pos.y -= n.y * moveA;  A.pos.z -= n.z * moveA;
		B.pos.x += n.x * moveB;  B.pos.y += n.y * moveB;  B.pos.z += n.z * moveB;
	}

	// 速度の反発（1次元）
	XMFLOAT3 rv{
		B.vel.x - A.vel.x,
		B.vel.y - A.vel.y,
		B.vel.z - A.vel.z
	};
	float relN = rv.x * n.x + rv.y * n.y + rv.z * n.z;
	if (relN > 0.0f) return;

	float e = std::clamp(std::min(A.restitution, B.restitution), 0.0f, 1.0f);
	float j = -(1.0f + e) * relN;
	j /= (invSum > 0.0f) ? invSum : 1.0f;

	A.vel.x -= n.x * j * invMa;  A.vel.y -= n.y * j * invMa;  A.vel.z -= n.z * j * invMa;
	B.vel.x += n.x * j * invMb;  B.vel.y += n.y * j * invMb;  B.vel.z += n.z * j * invMb;
}

void framework::simulate_bun(float dt)
{
	if (!bun.exists) return;

	// 重力
	bun.vel.y += patty_gravity_y * dt;

	// 積分
	bun.pos.x += bun.vel.x * dt;
	bun.pos.y += bun.vel.y * dt;
	bun.pos.z += bun.vel.z * dt;



	if (pate_collider_enabled)
	{
		using namespace DirectX;

		const XMFLOAT3& C = bun.pos;
		const XMFLOAT3  a = bun.half_extents;

		const XMFLOAT3& P = pate_position;
		const XMFLOAT3  b = pate_half_extents;

		// 中心差
		float dx = C.x - P.x;
		float dy = C.y - P.y;
		float dz = C.z - P.z;

		// 各軸オーバーラップ
		float ox = (a.x + b.x) - fabsf(dx);
		float oy = (a.y + b.y) - fabsf(dy);
		float oz = (a.z + b.z) - fabsf(dz);

		if (ox > 0.0f && oy > 0.0f && oz > 0.0f)
		{
			// 最小貫入軸
			float penetration;
			XMFLOAT3 n{ 0,0,0 };
			if (ox <= oy && ox <= oz) { penetration = ox; n.x = (dx >= 0.0f) ? 1.0f : -1.0f; }
			else if (oy <= oz) { penetration = oy; n.y = (dy >= 0.0f) ? 1.0f : -1.0f; }
			else { penetration = oz; n.z = (dz >= 0.0f) ? 1.0f : -1.0f; }

			// pate は運動学的（無限質量）として bun のみ押し戻す
			bun.pos.x += n.x * penetration;
			bun.pos.y += n.y * penetration;
			bun.pos.z += n.z * penetration;

			// 法線方向の速度を反射（Pattyの処理と同じ形）
			float relN = bun.vel.x * n.x + bun.vel.y * n.y + bun.vel.z * n.z;
			if (relN < 0.0f)
			{
				float e = std::clamp(std::min(bun.restitution, pate_restitution), 0.0f, 1.0f);
				float j = -(1.0f + e) * relN; // pate は無限質量
				bun.vel.x += n.x * j;
				bun.vel.y += n.y * j;
				bun.vel.z += n.z * j;
			}
		}
	}

	// Patty との AABB vs AABB 当たり
	// Patty側も AABB 表示＆パラメータを持っているので、素直にAABB同士で解決する
	for (const auto& S : patties)
	{
		const float dx = bun.pos.x - S.pos.x;
		const float px = (bun.half_extents.x + S.half_extents.x) - fabsf(dx);
		if (px <= 0) continue;

		const float dy = bun.pos.y - S.pos.y;
		const float py = (bun.half_extents.y + S.half_extents.y) - fabsf(dy);
		if (py <= 0) continue;

		const float dz = bun.pos.z - S.pos.z;
		const float pz = (bun.half_extents.z + S.half_extents.z) - fabsf(dz);
		if (pz <= 0) continue;

		// 最小貫通軸で押し戻し。反発係数は両者の大きい方を採用（単純化）
		const float e = std::max(bun.restitution, S.restitution);

		if (py <= px && py <= pz)
		{
			const float s = (dy >= 0.0f) ? 1.0f : -1.0f;
			bun.pos.y += s * py;
			if (s * bun.vel.y < 0.0f) bun.vel.y = -bun.vel.y * e;
		}
	}

	// 数値発散の簡易ブレーキ（必要なら）
	bun.vel.x *= 0.999f;
	bun.vel.z *= 0.999f;
}

void framework::simulate_patties(float dt)
{
	using namespace DirectX;
	if (!patty_sim_enabled) return;

	// 1) 重力 & 積分
	for (auto& P : patties)
	{
		if (P.isStuck)
			continue; // ← 刺さってる場合は動かさない

		P.vel.y += patty_gravity_y * dt;
		P.pos.x += P.vel.x * dt;
		P.pos.y += P.vel.y * dt;
		P.pos.z += P.vel.z * dt;

		float minY = P.pos.y - P.half_extents.y;
		if (minY < patty_ground_y)
		{


			float minY = P.pos.y - P.half_extents.y;


		}
	}

	// 2) 相互衝突（全組み合わせ）
	const int n = static_cast<int>(patties.size());
	for (int i = 0; i < n; ++i)
		for (int j = i + 1; j < n; ++j)
			resolve_pair(i, j);


}


bool IsIntersectAABB(
	const DirectX::XMFLOAT3& posA, const DirectX::XMFLOAT3& halfA,
	const DirectX::XMFLOAT3& posB, const DirectX::XMFLOAT3& halfB)
{
	using namespace DirectX;

	// 各軸方向での距離を計算
	float dx = fabs(posA.x - posB.x);
	float dy = fabs(posA.y - posB.y);
	float dz = fabs(posA.z - posB.z);

	// 各軸で重なっているかチェック
	if (dx > (halfA.x + halfB.x)) return false;
	if (dy > (halfA.y + halfB.y)) return false;
	if (dz > (halfA.z + halfB.z)) return false;

	// すべての軸で重なっていれば交差している
	return true;
}

void framework::CheckKusiPattyCollision()
{
	using namespace DirectX;

	for (auto& patty : patties)
	{
		if (patty.isStuck)
		{
			// すでに刺さっている場合は無視（動かない）
			continue;
		}

		bool hit = IsIntersectAABB(kusi.pos, kusi.half_extents, patty.pos, patty.half_extents);
		patty.isColliding = hit;

		if (hit)
		{
			// 串に刺さった状態にする
			patty.isStuck = true;


			// 動きを止める
			patty.vel = { 0.0f, 0.0f, 0.0f };
			kusi_menu[menu_count] = 1;
			menu_count += 1;
		}
		else
		{
			patty.isColliding = false;
		}
	}
}

void framework::add_cheese(const DirectX::XMFLOAT3& p,
	const DirectX::XMFLOAT3& half_extents,
	float mass,
	float restitution)
{
	Cheese obj{};
	obj.pos = p;
	obj.vel = { 0.0f, 0.0f, 0.0f };
	obj.half_extents = half_extents;
	obj.mass = mass;
	obj.restitution = restitution;

	// Bullet rigid body 作成
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(p.x, p.y, p.z));

	btVector3 inertia(0, 0, 0);
	if (mass > 0.0f)
		m_cheeseShape->calculateLocalInertia(mass, inertia);

	btDefaultMotionState* motion = new btDefaultMotionState(tr);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, m_cheeseShape, inertia);
	info.m_restitution = restitution;

	info.m_friction = 0.5f;

	btRigidBody* body = new btRigidBody(info);

	if (isSpawningPhase)
	{
		body->setLinearFactor(btVector3(0, 1, 0));
		body->setAngularFactor(btVector3(0, 0, 0));
	}

	m_btWorld->addRigidBody(body);

	obj.body = body;

	cheeses.push_back(obj);
	m_cheeseBodies.push_back(body);
}


void framework::CheckKusiCheeseCollision()
{
	using namespace DirectX;

	for (auto& cheese : cheeses)
	{
		if (cheese.isStuck) continue;

		// IsIntersectAABB は既存の関数を利用
		bool hit = IsIntersectAABB(kusi.pos, kusi.half_extents, cheese.pos, cheese.half_extents);
		cheese.isColliding = hit;

		if (hit)
		{
			cheese.isStuck = true;


			if (cheese.body) {
				cheese.body->setLinearVelocity(btVector3(0, 0, 0));
				cheese.body->setAngularVelocity(btVector3(0, 0, 0));

				cheese.body->setGravity(btVector3(0, 0, 0));
			}
			kusi_menu[menu_count] = 2;
			menu_count += 1;
		}
	}
}

void framework::initBulletWorld()
{
	m_btConfig = new btDefaultCollisionConfiguration();
	m_btDispatcher = new btCollisionDispatcher(m_btConfig);
	m_btBroadphase = new btDbvtBroadphase();
	m_btSolver = new btSequentialImpulseConstraintSolver();

	m_btWorld = new btDiscreteDynamicsWorld(
		m_btDispatcher, m_btBroadphase, m_btSolver, m_btConfig);

	// 重力（Y マイナス方向）
	m_btWorld->setGravity(btVector3(0.0f, patty_gravity_y, 0.0f));

	// パティ共通の形状（AABB）
	m_pattyShape = new btBoxShape(btVector3(
		patty_default_half_extents.x,
		patty_default_half_extents.y,
		patty_default_half_extents.z));

	//チーズ
	m_cheeseShape = new btBoxShape(btVector3(
		cheese_default_half_extents.x,
		cheese_default_half_extents.y,
		cheese_default_half_extents.z));

	// ★ Buns_under (静的剛体) の作成
	{
		// サイズ設定 (見た目に合わせて調整)
		bunUnder.half_extents = { 0.045f, 0.0020f, 0.045f };

		// 位置設定 (地面の上に置く)
		// x, z は他の具材が落ちてくる位置(-0.3, -10.0)に合わせる
		bunUnder.pos = { -0.3f, patty_ground_y + bunUnder.half_extents.y, -10.0f };

		// 形状作成
		m_bunUnderShape = new btBoxShape(btVector3(
			bunUnder.half_extents.x,
			bunUnder.half_extents.y,
			bunUnder.half_extents.z));

		btTransform tr;
		tr.setIdentity();
		tr.setOrigin(btVector3(bunUnder.pos.x, bunUnder.pos.y, bunUnder.pos.z));

		// 質量 0.0f = 静的 (動かない)
		btDefaultMotionState* motion = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo info(0.0f, motion, m_bunUnderShape, btVector3(0, 0, 0));

		info.m_restitution = 0.0f; // 跳ね返り係数
		info.m_friction = 10.1f;    // 摩擦 (具材が滑り落ちないように少し高め)

		bunUnder.body = new btRigidBody(info);
		m_btWorld->addRigidBody(bunUnder.body);
	}



	{
		// パテの形状を作成 (pate_half_extentsを使用)
		m_pateShape = new btBoxShape(btVector3(pate_half_extents.x, pate_half_extents.y, pate_half_extents.z));

		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(btVector3(pate_position.x, pate_position.y, pate_position.z));

		// 質量0で作成（静的/キネマティック）
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, m_pateShape, btVector3(0, 0, 0));

		rbInfo.m_restitution = pate_restitution; // 反発係数を適用
		rbInfo.m_friction = 0.5f;                // 摩擦係数（必要に応じて調整）

		m_pateBody = new btRigidBody(rbInfo);

		// キネマティックオブジェクトとして設定（物理演算で勝手に動かないが、プログラムから動かせる壁のような存在）
		m_pateBody->setCollisionFlags(m_pateBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

		// 常にアクティブにする（動かしたときに他の物体を起こすため）
		m_pateBody->setActivationState(DISABLE_DEACTIVATION);

		m_btWorld->addRigidBody(m_pateBody);
	}
}



// 最初だけ生成してシャッフル
void framework::prepareSpawnList()
{
	spawnList.clear();
	spawnList.reserve(SPAWN_MAX * 2);

	// 0 を 5回、1 を 5回登録
	for (int i = 0; i < SPAWN_MAX; i++) spawnList.push_back(0);
	for (int i = 0; i < SPAWN_MAX; i++) spawnList.push_back(1);

	// 静的乱数ジェネレータを使う
	static std::mt19937 gen((unsigned int)std::time(nullptr));

	std::shuffle(spawnList.begin(), spawnList.end(), gen);

	spawnIndex = 0;
}

void framework::Spwan(float dt)
{
	if (!allSpawn) return;

	coolTimer += dt;

	while (coolTimer >= interval && spawnIndex < spawnList.size())
	{
		coolTimer -= interval;

		int type = spawnList[spawnIndex++];

		if (type == 0)
		{
			add_patty({ patty_spawn_x, 0.5f, patty_spawn_z }, patty_default_half_extents, patty_default_mass, patty_default_restitution);
		}
		else
		{
			add_cheese({ patty_spawn_x, 0.8f, patty_spawn_z }, cheese_default_half_extents, cheese_default_mass, cheese_default_restitution);
		}
	}

	// 全出現したら終了
	if (spawnIndex >= spawnList.size())
	{
		allSpawn = false;

		// ★ 追加: スポーン完了時にロックを解除する関数を呼ぶ
		// （少し待ってから解除したい場合はタイマーを使うなど調整してください）
		EnablePhysicsForGameplay();
	}
}

void framework::EnablePhysicsForGameplay()
{
	if (!isSpawningPhase) return; // 既に解除済みなら何もしない
	isSpawningPhase = false;

	
	// 共通設定用のラムダ式（設定を一括で行う）
	auto setupBodyForGame = [](btRigidBody* body)
		{
			if (!body) return;

			// ロック解除
			body->setLinearFactor(btVector3(1, 1, 1));
			body->setAngularFactor(btVector3(1, 1, 1));

			//  修正: 回転減衰（第2引数）を上げる
			
			// これで「置いているだけなら倒れない粘り強さ」が出ます
			body->setDamping(0.05f, 1.8f);

			// 摩擦は高いままでOK
			body->setFriction(0.5f);

			body->activate(true);
		};

	// 全ての具材に適用
	for (auto& p : patties) setupBodyForGame(p.body);
	for (auto& c : cheeses) setupBodyForGame(c.body);

	

	// 下のバンズ（土台）も摩擦だけは合わせておく（動かない設定は維持される）
	if (bunUnder.body)
	{
		bunUnder.body->setFriction(1.0f);
	}
}
void framework::CheckPateCollision()
{
	// パテが無効なら何もしない
	if (!m_pateBody) return;

	using namespace DirectX;

	// --- Patty との判定 ---
	for (auto& p : patties)
	{
		// 既にロック解除済み（自由落下モード）ならスキップ
		// (LinearFactorのX成分が0以外なら解除済みとみなす判定)
		if (p.body->getLinearFactor().x() != 0.0f) continue;

		// 当たり判定 (AABB)
		if (IsIntersectAABB(pate_position, pate_half_extents, p.pos, p.half_extents))
		{
			//  ロック解除: 全方向に移動・回転できるようにする
			p.body->setLinearFactor(btVector3(1, 1, 1));
			p.body->setAngularFactor(btVector3(1, 1, 1));
			p.body->activate(true);
		}
	}

	// --- Cheese との判定 ---
	for (auto& c : cheeses)
	{
		if (c.body->getLinearFactor().x() != 0.0f) continue;

		if (IsIntersectAABB(pate_position, pate_half_extents, c.pos, c.half_extents))
		{
			// ★ ロック解除
			c.body->setLinearFactor(btVector3(1, 1, 1));
			c.body->setAngularFactor(btVector3(1, 1, 1));
			c.body->activate(true);
		}
	}


}

bool framework::uninitialize()
{
	return true;
}

framework::~framework()
{
}
