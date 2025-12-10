#include "System/Graphics.h"
#include "System/Input.h"
#include "SceneLoading.h"
#include "SceneManager.h"
#include "framework.h"

SceneLoading::SceneLoading(Scene* nextScene, framework* fw) : nextScene(nextScene), fw_(fw)
{
}

//初期化
void SceneLoading::Initialize()
{
	HRESULT hr{ S_OK };

	skinned_meshes[0] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\load_hamburger9.cereal");
	skinned_meshes[1] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\loading_text4.cereal");
	skinned_meshes[2] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\load_hamburger11.cereal");

	thread = new std::thread(LoadingThread, this);
}

//終了化
void SceneLoading::Finalize()
{
	if (thread != nullptr)
	{
		thread->join();
		delete thread;
		thread = nullptr;
	}
	skinned_meshes[0].reset();
	skinned_meshes[1].reset();
	skinned_meshes[2].reset();
}

//更新処理
void SceneLoading::Update(float elapsedTime)
{
	ShowCursor(TRUE);
	anim_time_ += elapsed_time;
	fw_->camera_position.x = 7.712f;
	fw_->camera_position.y = -0.130f;
	fw_->camera_position.z = -28.445f;

	fw_->light_direction.x = 1.0f;
	fw_->light_direction.y = -1.0f;
	fw_->light_direction.z = 1.0f;
	fw_->bloomer->bloom_extraction_threshold = 0.0f;

	hamburger_object.x = 2.667f;
	hamburger_object.y = -3.333f;
	hamburger_object.z = -11.000f;

	rotation_hamburger.x = 0.0f;
	rotation_hamburger.y = 3.115f;
	rotation_hamburger.z = 0.0f;

	if (nextScene->IsReady())
	{
		SceneManager::instance().ChangeScene(nextScene);
		nextScene = nullptr;
	}
}

//描画処理
void SceneLoading::Render(float elapsedTime)
{
	fw_->data.elapsed_time = elapsedTime;
	fw_->data.time += elapsedTime;

	fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[0].Get(), 0, 0, &fw_->data, 0, 0);
	fw_->immediate_context->VSSetConstantBuffers(1, 1, fw_->constant_buffers[0].GetAddressOf());
	fw_->immediate_context->PSSetConstantBuffers(1, 1, fw_->constant_buffers[0].GetAddressOf());
	fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[1].Get(), 0, 0, &fw_->parametric_constants, 0, 0);
	fw_->immediate_context->PSSetConstantBuffers(2, 1, fw_->constant_buffers[1].GetAddressOf());

	fw_->framebuffers[0]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[0]->activate(fw_->immediate_context.Get());

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
{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
	};

	const float scale_factor = 1.0f;

	const float spacing = 2.0f;

	DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[0]) * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) };

	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(fw_->scaling.x, fw_->scaling.y, fw_->scaling.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(fw_->rotation.x, fw_->rotation.y, fw_->rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(fw_->translation.x, fw_->translation.y, fw_->translation.z) };
	DirectX::XMFLOAT4X4 world;

	{
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
			hamburger_object.x,
			hamburger_object.y,
			hamburger_object.z
		);
		DirectX::XMMATRIX Scale = DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_hamburger.x,
			rotation_hamburger.y,
			rotation_hamburger.z
		);

		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, C * Scale * R * T);
		animation::keyframe keyframe_walk{};

		if (skinned_meshes[0]->animation_clips.size() > 0)
		{
			int clip_index{ 0 };
			int frame_index{ 0 };
			static float animation_tick_walk{ 0.0f };

			animation& anim_walk = skinned_meshes[0]->animation_clips.at(clip_index);
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
		skinned_meshes[0]->render(fw_->immediate_context.Get(), world, fw_->material_color, &keyframe_walk, fw_->flat_shading);
	}

	{
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
			hamburger_object.x,
			hamburger_object.y,
			hamburger_object.z
		);
		DirectX::XMMATRIX Scale = DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_hamburger.x,
			rotation_hamburger.y,
			rotation_hamburger.z
		);

		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, C * Scale * R * T);
		animation::keyframe keyframe_walk{};

		if (skinned_meshes[2]->animation_clips.size() > 0)
		{
			int clip_index{ 2 };
			int frame_index{ 0 };
			static float animation_tick_walk{ 0.0f };

			animation& anim_walk = skinned_meshes[2]->animation_clips.at(clip_index);
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
		skinned_meshes[2]->render(fw_->immediate_context.Get(), world, fw_->material_color, &keyframe_walk, fw_->flat_shading = true);
	}

	static float moveX = -25.0f;
	static float dir = 1.0f;
	static bool isRightLane = true;

	float move_speed = 5.0f;

	moveX += move_speed * elapsedTime * dir;

	if (moveX >= 22.0f)
	{
		moveX = 22.0f;
		dir = -1.0f;
		isRightLane = !isRightLane;
	}

	else if (moveX <= -25.0f)
	{
		moveX = -25.0f;
		dir = 1.0f;
		isRightLane = !isRightLane;
	}

	if (isRightLane)
	{
		translation_object.y = 0.667f;
		translation_object.z = -9.0f;
		rotation_object3.x = 0.187f;
	}
	else
	{
		translation_object.y = -6.667f;
		translation_object.z = -12.0f;
		rotation_object3.x = -0.28f;
		rotation_object3.y = 3.115f;
	}

	translation_object.x = moveX;

	{
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
			translation_object.x,
			translation_object.y,
			translation_object.z
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
		flat_shading = true;

		// ===== 明るさ変更（ここが追加点）=====
		DirectX::XMFLOAT4 prev_color = material_color;

		material_color.x *= loading_text_brightness_;
		material_color.y *= loading_text_brightness_;
		material_color.z *= loading_text_brightness_;

		// 描画
		skinned_meshes[1]->render(
			fw_->immediate_context.Get(),
			world,
			material_color,
			nullptr,
			flat_shading = false
		);

		// 元の色に戻す
		material_color = prev_color;
		// =====================================

		flat_shading = prev_flat;
	}

	fw_->framebuffers[0]->deactivate(fw_->immediate_context.Get());

	if (fw_->enable_bloom)
	{
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
		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		ID3D11ShaderResourceView* shader_resource_views[]{ fw_->framebuffers[0]->shader_resource_views[0].Get() };
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), shader_resource_views, 0, _countof(shader_resource_views), fw_->pixel_shaders[0].Get());
	}

	fw_->framebuffers[1]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[1]->activate(fw_->immediate_context.Get());
	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
	fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), fw_->framebuffers[0]->shader_resource_views[0].GetAddressOf(), 0, 1, fw_->pixel_shaders[0].Get());
	fw_->framebuffers[1]->deactivate(fw_->immediate_context.Get());
}

//GUI描画
void SceneLoading::DrawGUI()
{
}

//ローディングスレッド
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//COM関連の初期化でスレッド毎に呼ぶ必要がある
	CoInitialize(nullptr);

	//次のシーンの初期化を行う
	scene->nextScene->Initialize();

	//スレッドが終わる前にCOM関連の終了化
	CoUninitialize();

	//次のシーンの準備完了設定
	scene->nextScene->SetReady();
}