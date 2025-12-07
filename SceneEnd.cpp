#include "System/Graphics.h"
#include "SceneTitle.h"
#include "SceneEnd.h"
#include "System/Input.h"
#include "SceneGame.h"
#include "SceneLoading.h"
#include "SceneManager.h"
#include "framework.h"

SceneEnd::SceneEnd(framework* fw) : fw_(fw)
{
}

//初期化
void SceneEnd::Initialize()
{
	HRESULT hr{ S_OK };

	skinned_meshes[0] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\neon6.fbx");
	skinned_meshes[1] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\shop40.fbx");
	skinned_meshes[2] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\neon7.fbx");

	sprite_batches[0] = std::make_unique<sprite_batch>(fw_->device.Get(), L".\\resources\\END.png", 1);
}

//終了化
void SceneEnd::Finalize()
{
	skinned_meshes[0].reset();
	skinned_meshes[1].reset();
	skinned_meshes[2].reset();
}
//DirectX::XMFLOAT3 HSVtoRGB(float h, float s, float v)
//{
//	float r, g, b;
//
//	int i = int(h * 6.0f);
//	float f = (h * 6.0f) - i;
//	float p = v * (1.0f - s);
//	float q = v * (1.0f - f * s);
//	float t = v * (1.0f - (1.0f - f) * s);
//
//	switch (i % 6)
//	{
//	case 0: r = v; g = t; b = p; break;
//	case 1: r = q; g = v; b = p; break;
//	case 2: r = p; g = v; b = t; break;
//	case 3: r = p; g = q; b = v; break;
//	case 4: r = t; g = p; b = v; break;
//	case 5: r = v; g = p; b = q; break;
//	}
//
//	return { r, g, b };
//}

//更新処理
void SceneEnd::Update(float elaspedTime)
{
	fw_->light_direction = DirectX::XMFLOAT4{ -1.0f, -1.0f, 1.0f, 0.0f };

	fw_->distance = 20.0f;

	fw_->camera_position.x = 7.712f;
	fw_->camera_position.y = -0.130f;
	fw_->camera_position.z = -28.445f;

	translation_object3.x = 1.146f;
	translation_object3.y = -2.768f;
	translation_object3.z = -6.875f;

	HWND hwnd = FindWindow(APPLICATION_NAME, L"");
	// -------------------------
	// マウス座標取得
	// -------------------------
	//POINT mouse_client_pos{};
	//GetCursorPos(&mouse_client_pos);

	// クライアント座標へ変換
	//ScreenToClient(hwnd, &mouse_client_pos);

	// 100×100 のクリック判定
	//if (mouse_client_pos.x >= 0 &&
	//	mouse_client_pos.x <= 100 &&
	//	mouse_client_pos.y >= 0 &&
	//	mouse_client_pos.y <= 100)
	//{
	//	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	//	{
	//		HWND hwnd = FindWindow(APPLICATION_NAME, L"");
	//		SceneManager::instance().ChangeScene(new SceneLoading(new SceneGame(hwnd, fw_), fw_)); // fwでframeworkポインタを取得している
	//	}
	//}

	// -------------------------
	// 元々の SPACE キー遷移
	// -------------------------
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		SceneManager::instance().ChangeScene(new SceneTitle(fw_));
		return;
	}
	// ==========================
	// material_color1 と material_color2 の自動色変化
	// ==========================
	//float hue_speed = 0.1f;       // 色相の回転速度
	//float breathe_speed = 1.0f;   // 呼吸ライトの速度
	//float fixed_pulse = 0.8f;     // パルス固定値

	//float t = fw_->data.time;

	//// 1?? 色相を回す（滑らかな虹色）
	//float hue = fmodf(t * hue_speed, 1.0f);
	//DirectX::XMFLOAT3 rgb = HSVtoRGB(hue, 1.0f, 1.0f); // 彩度=1、明度=1

	//// 2?? 呼吸ライト（明度をゆらゆら変化させる）
	//float breathe = 0.5f * (sinf(t * breathe_speed) + 1.0f); // 0?1

	//// 3?? パルス（固定値）
	//float pulse = fixed_pulse;

	//// 4?? RGB に明度・パルスを掛ける
	//float brightness = 0.5f + 0.5f * breathe + pulse;

	//// material_color1 に反映
	//fw_->material_color1.x = rgb.x * brightness;
	//fw_->material_color1.y = rgb.y * brightness;
	//fw_->material_color1.z = rgb.z * brightness;
	//fw_->material_color1.w = 1.0f;

	//// material_color2 に反映
	//fw_->material_color2.x = rgb.x * brightness;
	//fw_->material_color2.y = rgb.y * brightness;
	//fw_->material_color2.z = rgb.z * brightness;
	//fw_->material_color2.w = 1.0f;
}

//描画処理
void SceneEnd::Render(float elapsedTime)
{
	// DYNAMIC_TEXTURE
	fw_->data.elapsed_time = elapsed_time;
	fw_->data.time += elapsed_time;

	fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[0].Get(), 0, 0, &fw_->data, 0, 0);
	fw_->immediate_context->VSSetConstantBuffers(1, 1, fw_->constant_buffers[0].GetAddressOf());
	// UNIT.16
	fw_->immediate_context->PSSetConstantBuffers(1, 1, fw_->constant_buffers[0].GetAddressOf());
	// UNIT.32
	fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[1].Get(), 0, 0, &fw_->parametric_constants, 0, 0);
	fw_->immediate_context->PSSetConstantBuffers(2, 1, fw_->constant_buffers[1].GetAddressOf());

	// DYNAMIC_TEXTURE
	if (fw_->enable_dynamic_shader)
	{
		fw_->dynamic_texture->clear(fw_->immediate_context.Get());
		fw_->dynamic_texture->activate(fw_->immediate_context.Get());
		fw_->immediate_context->OMSetDepthStencilState(depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), nullptr, 0, 0, effect_shaders[0].Get());
		fw_->dynamic_texture->deactivate(fw_->immediate_context.Get());
		fw_->immediate_context->PSSetShaderResources(15, 1, fw_->dynamic_texture->shader_resource_views[0].GetAddressOf());
	}
	else
	{
		ID3D11ShaderResourceView* null_srv[] = { nullptr };
		fw_->immediate_context->PSSetShaderResources(15, 1, null_srv);
	}

	// RADIAL_BLUR
	if (fw_->enable_radial_blur)
	{
		fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[1].Get(), 0, 0, &fw_->radial_blur_data, 0, 0);
		fw_->immediate_context->PSSetConstantBuffers(2, 1, fw_->constant_buffers[1].GetAddressOf());
	}

	// UNIT.32
	fw_->framebuffers[0]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[0]->activate(fw_->immediate_context.Get());

	// DYNAMIC_BACKGROUND
	if (fw_->enable_dynamic_background)
	{
		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		fw_->immediate_context->OMSetBlendState(fw_->blend_states[static_cast<size_t>(BLEND_STATE::NONE)].Get(), nullptr, 0xFFFFFFFF);
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), nullptr, 0, 0, fw_->effect_shaders[1].Get());
	}

	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());

	fw_->immediate_context->OMSetBlendState(fw_->blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);

	const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
	};

	const float scale_factor = 1.0f; // To change the units from centimeters to meters, set 'scale_factor' to 0.01.

	const float spacing = 2.0f;

	DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };

	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(fw_->scaling.x, fw_->scaling.y, fw_->scaling.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(fw_->rotation.x, fw_->rotation.y, fw_->rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(fw_->translation.x, fw_->translation.y, fw_->translation.z) };
	DirectX::XMFLOAT4X4 world;

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
		skinned_meshes[1]->render(fw_->immediate_context.Get(), world, material_color, nullptr, flat_shading);
		flat_shading = prev_flat;
	}

	fw_->framebuffers[0]->deactivate(fw_->immediate_context.Get());

	if (fw_->enable_bloom)
	{
		// BLOOM
		fw_->bloomer->make(fw_->immediate_context.Get(), fw_->framebuffers[0]->shader_resource_views[0].Get());

		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		fw_->immediate_context->OMSetBlendState(fw_->blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);
		ID3D11ShaderResourceView* shader_resource_views[] =
		{
			fw_->framebuffers[0]->shader_resource_views[0].Get(),
			fw_->bloomer->shader_resource_view(),
		};
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), shader_resource_views, 0, 2, fw_->pixel_shaders[0].Get());
	}

	if (fw_->enable_radial_blur)
	{
		// RADIAL_BLUR
		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		ID3D11ShaderResourceView* shader_resource_views[]{ fw_->framebuffers[0]->shader_resource_views[0].Get() };
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), shader_resource_views, 0, _countof(shader_resource_views), fw_->pixel_shaders[0].Get());
	}

	// UNIT.32
	fw_->framebuffers[1]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[1]->activate(fw_->immediate_context.Get());
	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
	fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), fw_->framebuffers[0]->shader_resource_views[0].GetAddressOf(), 0, 1, fw_->pixel_shaders[0].Get());
	fw_->framebuffers[1]->deactivate(fw_->immediate_context.Get());

	sprite_batches[0]->begin(fw_->immediate_context.Get());
	sprite_batches[0]->render(fw_->immediate_context.Get(), 0, 0, 1920, 1080);
	sprite_batches[0]->end(fw_->immediate_context.Get());
}

void SceneEnd::DrawGUI()
{
#ifdef USE_IMGUI
	ImGui::Begin("ImGUI");

	ImGui::End();
#endif
}