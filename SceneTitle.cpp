#include "System/Graphics.h"
#include "SceneTitle.h"
#include "System/Input.h"
#include "SceneGame.h"
#include "tutorial.h"
#include "SceneLoading.h"
#include "SceneEnd.h"
#include "SceneManager.h"
#include "framework.h"
#include <random>

SceneTitle::SceneTitle(framework* fw) : fw_(fw)
{
}

//初期化
void SceneTitle::Initialize()
{
	HRESULT hr{ S_OK };

	skinned_meshes[0] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\neon7.cereal");
	skinned_meshes[1] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\shop40.cereal");
	skinned_meshes[2] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\neon6.cereal");
	skinned_meshes[3] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\human.cereal");
	skinned_meshes[4] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\hamburger_man2.cereal");
	skinned_meshes[5] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\neon_title1.cereal");
	skinned_meshes[6] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\neon_title2.cereal");

	bgmTitle = Audio::Instance().LoadAudioSource(".\\resources\\僕のだぞっ！.wav");

	if (bgmTitle)
	{
		bgmTitle->Play(true);
	}

	fw_->bloomer->bloom_extraction_threshold = 0.253f;
	fw_->bloomer->bloom_intensity = 0.4f;
	fw_->light_direction = DirectX::XMFLOAT4{ -1.0f, -1.0f, 1.0f, 0.0f };

	fw_->distance = 20.0f;

	fw_->camera_position.x = 7.712f;
	fw_->camera_position.y = -0.130f;
	fw_->camera_position.z = -28.445f;

	translation_object0.x = -1.867f;
	translation_object0.y = -3.2f;
	translation_object0.z = -12.0f;

	translation_object3.x = 1.146f;
	translation_object3.y = -2.768f;
	translation_object3.z = -6.875f;

	rotation_object0.y = 2.533f;
	rotation_object0.z = -0.067f;

	translation_object2.x = 0.4f;
	translation_object2.y = -3.733f;
	translation_object2.z = -13.2f;

	rotation_object2.y = 3.333f;

	translation_object4.x = -3.2f;
	translation_object4.y = 2.8f;

	rotation_object4.x = 1.333f;
	rotation_object4.y = 1.2f;
	rotation_object4.z = -2.0f;

	translation_object5.x = -1.067f;
	translation_object5.y = -5.067f;
	translation_object5.z = -16.4f;

	rotation_object5.y = 2.4f;

	title0_brightness = 6.666f;
	title2_brightness = 4.0f;
	title5_brightness = 7.0f;
	title6_brightness = 3.333f;
}

//終了化
void SceneTitle::Finalize()
{
	for (auto& mesh : skinned_meshes)
	{
		mesh.reset();
	}
	// BGM停止・破棄
	if (bgmTitle)
	{
		bgmTitle->Stop();
		delete bgmTitle;
		bgmTitle = nullptr;
	}
}

DirectX::XMFLOAT3 HSVtoRGB(float h, float s, float v)
{
	float r, g, b;

	int i = int(h * 6.0f);
	float f = (h * 6.0f) - i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - f * s);
	float t = v * (1.0f - (1.0f - f) * s);

	switch (i % 6)
	{
	case 0: r = v; g = t; b = p; break;
	case 1: r = q; g = v; b = p; break;
	case 2: r = p; g = v; b = t; break;
	case 3: r = p; g = q; b = v; break;
	case 4: r = t; g = p; b = v; break;
	case 5: r = v; g = p; b = q; break;
	}

	return { r, g, b };
}

void SceneTitle::Update(float elaspedTime)
{
	static HWND hwnd = FindWindow(APPLICATION_NAME, L"");
	ShowCursor(TRUE);
	// 追加：元の状態保持用
//	static bool isHover = false;
	//static float defaultLightX = 0.0f;
	static DirectX::XMFLOAT4 defaultTitle6Color;

	static float neonFlickerTimer3 = 0.0f;
	static float neonFlickerIntensity3 = 1.0f;
	static float neonTargetIntensity3 = 1.0f;

	static float neonSubChannelDelay3 = 0.0f;   // 一部遅延用
	static float neonNoisePhase3 = 0.0f;        // 微細ノイズ

	static bool  neonPowerDrop3 = false;
	static float neonShakeTime3 = 0.0f;

	static std::mt19937 rng{ std::random_device{}() };

	// ★ 実機相当のリアル設定
	static std::uniform_real_distribution<float> flickerIntervalDist(0.18f, 1.2f);
	static std::uniform_real_distribution<float> flickerNormalDist(0.85f, 1.05f);
	static std::uniform_real_distribution<float> flickerDropDist(0.08f, 0.35f);
	static std::uniform_real_distribution<float> flickerOffDist(1.5f, 3.0f);     // 完全消灯時間
	static std::uniform_real_distribution<float> subChannelDelayDist(0.02f, 0.09f);
	static std::uniform_real_distribution<float> shakeDist(-0.03f, 0.03f);

	/*static bool initialized = false;
	if (!initialized)
	{

		initialized = true;
	}*/

	bool nowHover =
		fw_->mouse_client_pos.x >= click_min.x &&
		fw_->mouse_client_pos.x <= click_max.x &&
		fw_->mouse_client_pos.y >= click_min.y &&
		fw_->mouse_client_pos.y <= click_max.y;

	// ---- ホバー開始時 ----
	if (nowHover && !isHover)
	{
		isHover = true;

		// 元の値を保持
		defaultLightX = fw_->light_direction.x;
		defaultTitle6Color = material_color_title6;

		// light_direction.x を 10 に
		fw_->light_direction.y = 10.0f;
		title6_brightness = 10.0f;
	}

	// ---- ホバー終了時 ----
	if (!nowHover && isHover)
	{
		isHover = false;

		// 元に戻す
		fw_->light_direction.y = defaultLightX;
		material_color_title6 = defaultTitle6Color;
	}

	// ---- クリック処理 ----
	if (nowHover)
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			SceneManager::instance().ChangeScene(
				new SceneLoading(new SceneGame(hwnd, fw_), fw_));
			return;
		}
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		SceneManager::instance().ChangeScene(new SceneTutorial(hwnd, fw_));
		return;
	}

	const float hue_speed = 0.07f;
	const float breathe_speed = 1.0f;

	float t = fw_->data.time;
	float hue = fmodf(t * hue_speed, 1.0f);
	auto rgb = HSVtoRGB(hue, 1.0f, 1.0f);

	float breathe = 0.5f * (sinf(t * breathe_speed) + 1.0f);
	float brightness = 0.6f + breathe;

	auto col = DirectX::XMFLOAT4(rgb.x * brightness, rgb.y * brightness, rgb.z * brightness, 1.0f);
	material_color1 = col;
	material_color2 = col;

	const float hue_speed6 = 0.05f;
	float hue6 = fmodf(t * hue_speed6, 1.0f);
	auto rgb6 = HSVtoRGB(hue6, 1.0f, 1.0f);

	// mesh[0]
	material_color1 = {
		title0_brightness,
		title0_brightness,
		title0_brightness,
		1.0f
	};

	// mesh[2]
	material_color2 = {
		title2_brightness,
		title2_brightness,
		title2_brightness,
		1.0f
	};

	// ==========================================
	// material_color3 超リアル劣化ネオン（実機挙動）
	// ==========================================
	neonFlickerTimer3 -= elaspedTime;
	neonNoisePhase3 += elaspedTime * 18.0f;

	// 微細な電圧ノイズ（常に少しだけ揺らぐ）
	float voltageNoise =
		1.0f + sinf(neonNoisePhase3) * 0.015f +
		sinf(neonNoisePhase3 * 2.7f) * 0.01f;

	// フリッカーイベント発生
	if (neonFlickerTimer3 <= 0.0f)
	{
		neonFlickerTimer3 = flickerIntervalDist(rng);

		int pattern = rng() % 12;

		if (pattern == 0)
		{
			// ごく稀：完全消灯
			neonTargetIntensity3 = 0.0f;
			neonPowerDrop3 = true;
			neonShakeTime3 = 0.12f;
			neonSubChannelDelay3 = flickerOffDist(rng);
		}
		else if (pattern <= 3)
		{
			// 電圧ドロップ（ふわっと暗く）
			neonTargetIntensity3 = flickerDropDist(rng);
			neonPowerDrop3 = true;
			neonShakeTime3 = 0.06f;
		}
		else
		{
			// 通常揺らぎ
			neonTargetIntensity3 = flickerNormalDist(rng);
			neonPowerDrop3 = false;

			if (rng() % 3 == 0)
				neonSubChannelDelay3 = subChannelDelayDist(rng);
		}
	}

	// 一部遅延チャンネル復帰
	if (neonSubChannelDelay3 > 0.0f)
	{
		neonSubChannelDelay3 -= elaspedTime;
	}

	// 滑らかな電圧復帰
	float recoverSpeed = neonPowerDrop3 ? 4.0f : 7.0f;
	neonFlickerIntensity3 +=
		(neonTargetIntensity3 - neonFlickerIntensity3) *
		recoverSpeed * elaspedTime;

	// 微ブレ時間の減衰
	if (neonShakeTime3 > 0.0f)
		neonShakeTime3 -= elaspedTime;
	else
		neonShakeTime3 = 0.0f;

	// チャンネル遅延分（部分的に遅れて点く感じ）
	float channelDelayFactor =
		(neonSubChannelDelay3 > 0.0f) ? 0.75f : 1.0f;

	// 最終輝度
	float flickerBrightness3 =
		title5_brightness *
		neonFlickerIntensity3 *
		voltageNoise *
		channelDelayFactor;

	material_color3 =
	{
		flickerBrightness3,
		flickerBrightness3,
		flickerBrightness3,
		1.0f
	};

	if (isHover)
	{
		// ホバー中：赤く固定発光
		material_color_title6 = {
			title6_brightness,   // R
			0.0f,                     // G
			0.0f,                     // B
			1.0f
		};
	}
	else
	{
		// 非ホバー：従来の虹色発光
		material_color_title6 = {
			rgb6.x * title6_brightness,
			rgb6.y * title6_brightness,
			rgb6.z * title6_brightness,
			1.0f
		};
	}
}

//描画処理
void SceneTitle::Render(float elapsedTime)
{
	fw_->data.elapsed_time = elapsedTime;
	fw_->data.time += elapsedTime;

	fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[0].Get(), 0, 0, &fw_->data, 0, 0);
	fw_->immediate_context->VSSetConstantBuffers(1, 1, fw_->constant_buffers[0].GetAddressOf());
	fw_->immediate_context->PSSetConstantBuffers(1, 1, fw_->constant_buffers[0].GetAddressOf());

	fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[1].Get(), 0, 0, &fw_->parametric_constants, 0, 0);
	fw_->immediate_context->PSSetConstantBuffers(2, 1, fw_->constant_buffers[1].GetAddressOf());

	{
		ID3D11ShaderResourceView* null_srv[] = { nullptr };
		fw_->immediate_context->PSSetShaderResources(15, 1, null_srv);
	}

	fw_->framebuffers[0]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[0]->activate(fw_->immediate_context.Get());

	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());
	fw_->immediate_context->OMSetBlendState(fw_->blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);

	const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
		{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
		{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
	};
	const float scale_factor = 1.0f;
	DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };

	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(fw_->scaling.x, fw_->scaling.y, fw_->scaling.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(fw_->rotation.x, fw_->rotation.y, fw_->rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(fw_->translation.x, fw_->translation.y, fw_->translation.z) };

	{
		DirectX::XMMATRIX T0 = DirectX::XMMatrixTranslation(
			translation_object3.x,
			translation_object3.y,
			translation_object3.z
		);
		DirectX::XMMATRIX Scale0 = DirectX::XMMatrixScaling(1, 1, 1);
		DirectX::XMMATRIX R0 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y,
			rotation_object3.z
		);
		DirectX::XMFLOAT4X4 world0;
		DirectX::XMStoreFloat4x4(&world0, C * Scale0 * R0 * T0);
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[1]->render(fw_->immediate_context.Get(), world0, material_color, nullptr, flat_shading);
		flat_shading = prev_flat;
	}

	{
		DirectX::XMMATRIX T1 = DirectX::XMMatrixTranslation(
			translation_object0.x,
			translation_object0.y,
			translation_object0.z
		);
		DirectX::XMMATRIX Scale1 = DirectX::XMMatrixScaling(1, 1, 1);
		DirectX::XMMATRIX R1 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object0.x,
			rotation_object0.y,
			rotation_object0.z
		);
		DirectX::XMFLOAT4X4 world1;
		DirectX::XMStoreFloat4x4(&world1, C * Scale1 * R1 * T1);
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[0]->render(fw_->immediate_context.Get(), world1, material_color1, nullptr, flat_shading);
		flat_shading = prev_flat;
	}

	{
		DirectX::XMMATRIX T2 = DirectX::XMMatrixTranslation(
			translation_object2.x,
			translation_object2.y,
			translation_object2.z
		);
		DirectX::XMMATRIX Scale2 = DirectX::XMMatrixScaling(1, 1, 1);
		DirectX::XMMATRIX R2 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object2.x,
			rotation_object2.y,
			rotation_object2.z
		);
		DirectX::XMFLOAT4X4 world2;
		DirectX::XMStoreFloat4x4(&world2, C * Scale2 * R2 * T2);
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[2]->render(fw_->immediate_context.Get(), world2, material_color_title6, nullptr, flat_shading);
		flat_shading = prev_flat;
	}
	{
		DirectX::XMMATRIX T2 = DirectX::XMMatrixTranslation(
			translation_object5.x,
			translation_object5.y,
			translation_object5.z
		);
		DirectX::XMMATRIX Scale2 = DirectX::XMMatrixScaling(0.6f, 0.6f, 0.6f);
		DirectX::XMMATRIX R2 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object5.x,
			rotation_object5.y,
			rotation_object5.z
		);
		DirectX::XMFLOAT4X4 world2;
		DirectX::XMStoreFloat4x4(&world2, C * Scale2 * R2 * T2);
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[5]->render(fw_->immediate_context.Get(), world2, material_color2, nullptr, flat_shading);
		flat_shading = prev_flat;
	}
	{
		DirectX::XMMATRIX T2 = DirectX::XMMatrixTranslation(
			translation_object5.x,
			translation_object5.y,
			translation_object5.z
		);
		DirectX::XMMATRIX Scale2 = DirectX::XMMatrixScaling(0.6f, 0.6f, 0.6f);
		DirectX::XMMATRIX R2 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object5.x,
			rotation_object5.y,
			rotation_object5.z
		);
		DirectX::XMFLOAT4X4 world2;
		DirectX::XMStoreFloat4x4(&world2, C * Scale2 * R2 * T2);
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[6]->render(
			fw_->immediate_context.Get(),
			world2,
			material_color3,   // ← 専用カラー
			nullptr,
			flat_shading
		);
		flat_shading = prev_flat;
	}
	{
		DirectX::XMMATRIX T3 = DirectX::XMMatrixTranslation(
			translation_object3.x,
			translation_object3.y,
			translation_object3.z
		);
		DirectX::XMMATRIX Scale3 = DirectX::XMMatrixScaling(1, 1, 1);
		DirectX::XMMATRIX R3 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y,
			rotation_object3.z
		);
		DirectX::XMFLOAT4X4 world3;
		DirectX::XMStoreFloat4x4(&world3, C * Scale3 * R3 * T3);

		bool prev_flat = flat_shading;
		flat_shading = true;

		auto prevColor = material_color;
		material_color.w = 0.33f;

		skinned_meshes[3]->render(
			fw_->immediate_context.Get(),
			world3,
			material_color,
			nullptr,
			flat_shading
		);

		material_color = prevColor;
		flat_shading = prev_flat;
	}

	{
		DirectX::XMMATRIX T4 = DirectX::XMMatrixTranslation(
			translation_object4.x,
			translation_object4.y,
			translation_object4.z
		);
		DirectX::XMMATRIX Scale4 = DirectX::XMMatrixScaling(1.5f, 1.5f, 1.5f);
		DirectX::XMMATRIX R4 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object4.x,
			rotation_object4.y,
			rotation_object4.z
		);

		DirectX::XMFLOAT4X4 world4;
		DirectX::XMStoreFloat4x4(&world4, C * Scale4 * R4 * T4);
		animation::keyframe keyframe_walk{};

		if (skinned_meshes[4]->animation_clips.size() > 0)
		{
			int clip_index{ 1 };
			int frame_index{ 0 };
			static float animation_tick_walk{ 0.0f };

			animation& anim_walk = skinned_meshes[4]->animation_clips.at(clip_index);
			frame_index = static_cast<int>(animation_tick_walk * anim_walk.sampling_rate);
			if (frame_index > anim_walk.sequence.size() - 1)
			{
				frame_index = 0;
				animation_tick_walk = 0.0f;
			}
			else
			{
				animation_tick_walk += elapsedTime;
			}
			keyframe_walk = anim_walk.sequence.at(frame_index);
		}
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[4]->render(fw_->immediate_context.Get(), world4, material_color, &keyframe_walk, flat_shading);
		flat_shading = prev_flat;
	}

	fw_->framebuffers[0]->deactivate(fw_->immediate_context.Get());

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

	fw_->framebuffers[1]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[1]->activate(fw_->immediate_context.Get());
	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());

	fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), fw_->framebuffers[0]->shader_resource_views[0].GetAddressOf(), 0, 1, fw_->pixel_shaders[0].Get());
	fw_->framebuffers[1]->deactivate(fw_->immediate_context.Get());
}

void SceneTitle::DrawGUI()
{
#ifdef USE_IMGUI
	ImGui::Begin("ImGUI");
	//ImGui::Text("FPS: %.1f", current_fps);	ImGui::SameLine(); ImGui::Text("||Frame Time: %.2f ms", current_frame_time);

	ImGui::SliderFloat("camera_position.x", &fw_->camera_position.x, -100.0f, +100.0f);
	ImGui::SliderFloat("camera_position.y", &fw_->camera_position.y, -100.0f, +100.0f);
	ImGui::SliderFloat("camera_position.z", &fw_->camera_position.z, -100.0f, -1.0f);

	ImGui::Checkbox("flat_shading", &fw_->flat_shading);
	ImGui::Checkbox("Enable Dynamic Shader", &fw_->enable_dynamic_shader);
	ImGui::Checkbox("Enable Dynamic Background", &fw_->enable_dynamic_background);
	ImGui::Checkbox("Enable RADIAL_BLUR", &fw_->enable_radial_blur);
	ImGui::Checkbox("Enable Bloom", &fw_->enable_bloom);

	ImGui::SliderFloat("light_direction.x", &fw_->light_direction.x, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.y", &fw_->light_direction.y, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.z", &fw_->light_direction.z, -1.0f, +1.0f);

	// UNIT.32
	ImGui::SliderFloat("extraction_threshold", &fw_->parametric_constants.extraction_threshold, +0.0f, +5.0f);
	ImGui::SliderFloat("gaussian_sigma", &fw_->parametric_constants.gaussian_sigma, +0.0f, +10.0f);
	ImGui::SliderFloat("exposure", &fw_->parametric_constants.exposure, +0.0f, +10.0f);

	// RADIAL_BLUR
	ImGui::DragFloat2("blur_center", &fw_->radial_blur_data.blur_center.x, 0.01f);
	ImGui::SliderFloat("blur_strength", &fw_->radial_blur_data.blur_strength, +0.0f, +1.0f);
	ImGui::SliderFloat("blur_radius", &fw_->radial_blur_data.blur_radius, +0.0f, +1.0f);
	ImGui::SliderFloat("blur_decay", &fw_->radial_blur_data.blur_decay, +0.0f, +1.0f);

	// BLOOM
	ImGui::SliderFloat("bloom_extraction_threshold", &fw_->bloomer->bloom_extraction_threshold, +0.0f, +1.0f);
	ImGui::SliderFloat("bloom_intensity", &fw_->bloomer->bloom_intensity, +0.0f, +5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); // 内側の余白を増やす
	ImGui::SliderFloat("Camera Distance", &fw_->distance, 1.0f, 20.0f);
	ImGui::PopStyleVar();

	ImGui::Text("Stage Rotation");
	ImGui::SliderFloat("Pos X##obj3", &fw_->translation_object3.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Pos Y##obj3", &fw_->translation_object3.y, -10.0f, 10.0f);
	ImGui::SliderFloat("Pos Z##obj3", &fw_->translation_object3.z, -10.0f, 10.0f);

	ImGui::End();
#endif
}