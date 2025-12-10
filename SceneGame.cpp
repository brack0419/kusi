#include "shader.h"
#include "texture.h"
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneEnd.h"
#include "framework.h"
#include "System/Graphics.h"
#include "SceneGame.h"
#include <algorithm>
#define BT_NO_SIMD_OPERATOR_OVERLOADS
#include <bullet/btBulletDynamicsCommon.h>
#include <dxgi.h>
#undef min
#undef max
#include <vector>
#include<random>


int menu_count = 0;
static float animation_tick_walk = 0.0f;
static float animation_tick = 0.0f;
static float animation_tick1 = 0.0f;
static bool isAnimationFinished = false;   // ★ 追加
static float animation_speed = 0.85f;
static int play_count = 0;        // 何回再生したか
static bool isPlaying = false;   // 再生中フラグ
static bool wasRPressed = false;   // 押しっぱなし防止用
float saveTime = 0;


int SceneGame::CheckMenu(const int* a, const int* b, int size)
{
	// すべて一致しているか確認
	for (int i = 0; i < size; ++i)
	{
		if (a[i] != b[i])
		{
			return 2; // 一致しない要素があれば 0 (Default) を返す
		}
	}
	return 1; // 全て一致すれば 1 (Game Clear) を返す
}

SceneGame::SceneGame(HWND hwnd, framework* fw) : hwnd(hwnd), fw_(fw)
{
	//SceneManager::instance().ChangeScene(new SceneTitle(fw));
}

// 初期化
void SceneGame::Initialize()	// deviceとimmediate_contextを定義しているものがいまframeworkのinitializeにあるんだけど、
// いまSceneGameで描画をしようとしていてそのためにこのコードをもう一度書かずに、
// deviceとimmediate_contextを定義したい
{
	HRESULT hr{ S_OK };

	//fw_->skinned_meshes[2] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Patty4.cereal");
	//fw_->skinned_meshes[3] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\shop5.cereal");
	//fw_->skinned_meshes[4] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Spautla.cereal");
	//skinned_meshes[5] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\bus3.cereal");
	//fw_->skinned_meshes[6] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\cube.000.cereal");
	//fw_->dynamic_texture = std::make_unique<framebuffer>(fw_->device.Get(), 512, 512);
	//fw_->skinned_meshes[7] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Skewers.cereal");
	//fw_->skinned_meshes[8] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_up.cereal");
	//fw_->skinned_meshes[9] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_under.cereal");
	//fw_->skinned_meshes[10] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Cheese.cereal");

	//skinned_meshes[0] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\hamburger_man.cereal");
	//skinned_meshes[1] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\load_hamburger.cereal");
	skinned_meshes[2] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Patty.cereal");
	skinned_meshes[3] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\shop22.cereal");
	skinned_meshes[4] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Spautla.cereal");
	//skinned_meshes[5] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\bus.cereal");

	skinned_meshes[6] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\cube.000.cereal");
	fw_->dynamic_texture = std::make_unique<framebuffer>(fw_->device.Get(), 512, 512);

	skinned_meshes[7] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Skewers.cereal");
	skinned_meshes[8] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_up1.cereal");
	skinned_meshes[9] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_under1.cereal");

	skinned_meshes[10] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Cheese2.cereal");
	skinned_meshes[11] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\hands38.cereal");
	skinned_meshes[12] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Vegetable.cereal");
	skinned_meshes[13] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\luser8.cereal");
	skinned_meshes[14] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\menu1.fbx");
	skinned_meshes[15] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\menu2.fbx");
	skinned_meshes[16] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\menu3.fbx");


	sprite_batches[1] = std::make_unique<sprite_batch>(fw_->device.Get(), L".\\resources\\select.png", 1);

	bgmGame = Audio::Instance().LoadAudioSource(".\\resources\\ハンバーガーショップの戦い.wav");

	if (bgmGame)
	{
		bgmGame->Play(true);
		//TitleSource->Play(false);
	}

	initBulletWorld();   // ★ 追加

	patties.clear();

	cheeses.clear();

	vegetables.clear();

	menu_count = 0;
	generatePattern();

	camera_focus = { 0.0f, 0.343f, -10.0f };
	selectMenu = true;

	target_camera_focus_y = camera_focus.y;

	target_object_y = translation_object11.y;
	saveTime = 0;

	fw_->bloomer->bloom_intensity = 0.15;
	//prepareSpawnList();
}

// 終了化
void SceneGame::Finalize()
{
	// BGM停止・破棄
	if (bgmGame)
	{
		bgmGame->Stop();
		delete bgmGame;
		bgmGame = nullptr;
	}
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	fw_->bloomer->bloom_extraction_threshold = 0.268f;
	fw_->bloomer->bloom_intensity = 0.23f;
	if (selectMenu)
	{
		if (fw_->mouse_client_pos.x >= 0 &&
			fw_->mouse_client_pos.x <= 640 &&
			fw_->mouse_client_pos.y >= 0 &&
			fw_->mouse_client_pos.y <= 1080)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			{
				ifMenu = 1;
				selectMenu = false;
				generatePattern();

			}
		}
		if (fw_->mouse_client_pos.x >= 641 &&
			fw_->mouse_client_pos.x <= 1280 &&
			fw_->mouse_client_pos.y >= 0 &&
			fw_->mouse_client_pos.y <= 1080)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			{
				ifMenu = 2;
				selectMenu = false;
				generatePattern();

			}
		}
		if (fw_->mouse_client_pos.x >= 1281 &&
			fw_->mouse_client_pos.x <= 1920 &&
			fw_->mouse_client_pos.y >= 0 &&
			fw_->mouse_client_pos.y <= 1080)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			{
				ifMenu = 3;
				selectMenu = false;
				generatePattern();

			}
		}
	}
	else
	{
		ShowCursor(FALSE);
		StartTime += elapsedTime;
		if (!isAnimationStarted)
		{
			animation_tick_walk = 0.0f;   // ← リセット
			animation_tick = 0.0f;   // ← リセット
			isAnimationStarted = true;    // ← 開始フラグON
		}
		// ★ Hキーが「押された瞬間」だけ判定
		if ((GetAsyncKeyState('H') & 0x0001))
		{
			// ★ 再生していない or 再生が完全に終わったときだけ再スタート
			if (!isAnimationStarted || isAnimationFinished)
			{
				animation_tick_walk = 0.0f;

				animation_tick = 0.0f;
				isAnimationStarted = true;
				isAnimationFinished = false;
				animation_speed = 1.5f;
			}
		}

		//if(GetAsyncKeyState(VK_LSHIFT) & 0x8000)
		if ((StartTime <= 5.0) || (isAnimationFinished == false))
		{
			POINT p = { 1500, 540 };
			SetCursorPos(p.x, p.y);
		}

		Spawn(elapsedTime);

		if (kusi.pos.y + 0.05 <= patty_ground_y) // +の数値はkusiの位置調整のため
		{
			kusi.kusi_End = true;
			kusi.move = false;
		}

		if (kusi.kusi_End)
		{
			switch (ifMenu)
			{
			case 1:
				gameEnd = CheckMenu(kusi_menu, Emenu, MENU_MAX);
				break;

			case 2:
				gameEnd = CheckMenu(kusi_menu, Nmenu, MENU_MAX);
				break;

			case 3:
				gameEnd = CheckMenu(kusi_menu, Hmenu, MENU_MAX);
				break;
			}

			if (gameEnd == 1)
			{
				saveTime = StartTime;
				SceneManager::instance().ChangeScene(new SceneEnd(fw_));
			}
			if (gameEnd == 2)
			{
				Reset();
			}
			// SceneEnd へ
		}

		// SceneEnd へ
		//SceneManager::instance().ChangeScene(new SceneEnd(fw_));
		bool isRPressed = (GetAsyncKeyState('R') & 0x8000);

		if (isRPressed && !wasRPressed && !isPlaying)  // ★ isPlaying を追加
		{
			// 再生中でない時だけスタート
			animation_tick1 = 0.0f;
			play_count = 0;
			isPlaying = true;
		}

		wasRPressed = isRPressed;

		static bool prev_R = false;
		bool now_R = (GetAsyncKeyState('R') & 0x8000);
		if (now_R && !prev_R)
		{
			Reset();
		}
		prev_R = now_R;
		// -----------------------------------------------
		// ホイール入力でターゲット位置を更新
		// -----------------------------------------------
		target_camera_focus_y += fw_->wheel * cameraSpeed;
		target_object_y += fw_->wheel * cameraSpeed;   // ★ カメラと同じ動作

		// -----------------------------------------------
		// クランプ
		// -----------------------------------------------
		const float minY = 0.22f;
		const float maxY = 0.8f;

		const float HminY = -0.156f;
		const float HmaxY = 0.424f;

		target_camera_focus_y = std::clamp(target_camera_focus_y, minY, maxY);
		target_object_y = std::clamp(target_object_y, HminY, HmaxY);

		// -----------------------------------------------
		// 下限付近でホイール方向への動き許可
		// -----------------------------------------------
		if (camera_focus.y <= minY + 0.0001f)
		{
			camera_focus.y = minY;

			if (fw_->wheel < 0)
			{
				cameraSpeed = 0.0005f;
				target_camera_focus_y += fw_->wheel * cameraSpeed;
				target_object_y += fw_->wheel * cameraSpeed;
			}
		}

		if (translation_object11.y <= HminY + 0.0001f)
		{
			translation_object11.y = HminY;

			if (fw_->wheel < 0)
			{
				cameraSpeed = 0.0005f;
				target_camera_focus_y += fw_->wheel * cameraSpeed;
				target_object_y += fw_->wheel * cameraSpeed;
			}
		}

		// -----------------------------------------------
		// 上限付近でホイール方向への動き許可
		// -----------------------------------------------
		if (camera_focus.y >= maxY - 0.0001f)
		{
			camera_focus.y = maxY;

			if (fw_->wheel > 0)
			{
				cameraSpeed = 0.0005f;
				target_camera_focus_y += fw_->wheel * cameraSpeed;
				target_object_y += fw_->wheel * cameraSpeed;
			}
		}

		if (translation_object11.y >= HmaxY - 0.0001f)
		{
			translation_object11.y = HmaxY;

			if (fw_->wheel > 0)
			{
				cameraSpeed = 0.0005f;
				target_camera_focus_y += fw_->wheel * cameraSpeed;
				target_object_y += fw_->wheel * cameraSpeed;
			}
		}

		// -----------------------------------------------
		// スムーズ追従
		// -----------------------------------------------
		camera_focus.y += (target_camera_focus_y - camera_focus.y) * elapsedTime;
		translation_object11.y += (target_object_y - translation_object11.y) * elapsedTime;

		// ホイールリセット
		fw_->wheel = 0.0f;

		static bool prev_down = false;
		//bool now_down = (GetAsyncKeyState(VK_DOWN) & 0x8000);
		
		if (StartTime >= 5.0f)
		{
			now_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
		}

		if (now_down && !prev_down) { // 押した瞬間だけ反応
			kusi.move = !kusi.move;               // トグル
			hitStop = !hitStop;
			if (kusi.move)
			{
				bun.exists = true;                 // 描画・物理演算を有効化
				bun.pos = { -0.3f, 3.0f, -10.0f }; // 上空(Y=4.0)に移動。
				bun.vel = { 0.0f, 0.0f, 0.0f };    // 落下速度をリセット
			}
			//spawn_bun_up();
		}

		prev_down = now_down; // 状態保存

		if (kusi.move)
		{
			kusi.pos.y -= KUSI_SPEED;
		}

		if (!bun_spawned_initial)
		{
			bun.exists = false;
			bun.pos = { -0.3f, 1.3f, -10.0f }; // だいたいPattyと同じ奥行き
			bun.vel = { 0.0f, 0.0f, 0.0f };

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
				if (elapsedTime > 0.0f)
				{
					btVector3 currentPos(pate_position.x, pate_position.y, pate_position.z);
					btVector3 prevPos(prev_pate_pos.x, prev_pate_pos.y, prev_pate_pos.z);

					// 速度ベクトル
					btVector3 velocity = (currentPos - prevPos) / elapsedTime;

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
			m_btWorld->stepSimulation(elapsedTime, 20, 1.0f / 240.0f);

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
			}

			for (auto& C : cheeses)
			{
				if (!C.body) continue;
				if (hitStop) continue;

				// 串に刺さっている場合は、串の動きに追従させるなどの処理が必要だが
				// ここでは物理演算の結果を取得するだけにする（Stuck時は物理無効化などを想定）
				if (C.isStuck) {
					// 串と一緒に動かすならここに追従処理
					continue;
				}

				btTransform tr;
				C.body->getMotionState()->getWorldTransform(tr);
				btVector3 pos = tr.getOrigin();
				C.pos = { (float)pos.getX(), (float)pos.getY(), (float)pos.getZ() };
				btQuaternion q = tr.getRotation();
				C.rotQuat = { (float)q.x(), (float)q.y(), (float)q.z(), (float)q.w() };
			}

			for (auto& V : vegetables)
			{
				if (!V.body) continue;
				if (hitStop) continue;

				// 串に刺さっている場合などはスキップする処理が必要ならここに追加
				if (V.isStuck) continue;

				btTransform tr;
				V.body->getMotionState()->getWorldTransform(tr);

				btVector3 pos = tr.getOrigin();
				V.pos = { (float)pos.getX(), (float)pos.getY(), (float)pos.getZ() };

				btQuaternion q = tr.getRotation();
				V.rotQuat = { (float)q.x(), (float)q.y(), (float)q.z(), (float)q.w() };
			}
		}

		simulate_bun(elapsedTime);
		CheckKusiCheeseCollision();
		CheckKusiPattyCollision();
		CheckKusiVegetableCollision();
	}
}

// 描画処理
void SceneGame::Render(float elapsedTime)
{
	HRESULT hr{ S_OK };

	FLOAT color[]{ 0.0f, 0.0f, 0.0f, 0.2f };
	fw_->immediate_context->ClearRenderTargetView(fw_->render_target_view.Get(), color);
#if 0
	immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_zCLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif // 0
	fw_->immediate_context->OMSetRenderTargets(1, fw_->render_target_view.GetAddressOf(), fw_->depth_stencil_view.Get());

	fw_->immediate_context->PSSetSamplers(0, 1, fw_->sampler_states[static_cast<size_t>(SAMPLER_STATE::POINT)].GetAddressOf());
	fw_->immediate_context->PSSetSamplers(1, 1, fw_->sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR)].GetAddressOf());
	fw_->immediate_context->PSSetSamplers(2, 1, fw_->sampler_states[static_cast<size_t>(SAMPLER_STATE::ANISOTROPIC)].GetAddressOf());
	// UNIT.32
	fw_->immediate_context->PSSetSamplers(3, 1, fw_->sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_BLACK)].GetAddressOf());
	fw_->immediate_context->PSSetSamplers(4, 1, fw_->sampler_states[static_cast<size_t>(SAMPLER_STATE::LINEAR_BORDER_WHITE)].GetAddressOf());

	fw_->immediate_context->OMSetBlendState(fw_->blend_states[static_cast<size_t>(BLEND_STATE::ALPHA)].Get(), nullptr, 0xFFFFFFFF);
	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());

	// 追加：skinned_meshes[4] の world 行列を保存する（バウンディングボックス用）
	DirectX::XMFLOAT4X4 world_obj4 = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	D3D11_VIEWPORT viewport;
	UINT num_viewports{ 1 };
	fw_->immediate_context->RSGetViewports(&num_viewports, &viewport);

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
			DirectX::XMLoadFloat3(&camera_focus), up);
	}

	//ワールドへの変換
	{
		const float vx = static_cast<float>(fw_->mouse_client_pos.x);
		const float vy = static_cast<float>(fw_->mouse_client_pos.y);

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
			if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
			{
				pate_target.y = hit3.y;
			}
			pate_target.z = hit3.z;
		}

		// 追従スムージング：Y/Z のみ補間、X は固定
		auto lerp = [](float a, float b, float s) { return a + (b - a) * s; };
		pate_position.y = lerp(pate_position.y, pate_target.y, pate_follow_smooth);
		pate_position.z = lerp(pate_position.z, pate_target.z, pate_follow_smooth);
		pate_position.x = planeX;
	}

	//scene_constants data{};
	DirectX::XMStoreFloat4x4(&fw_->data.view_projection, V * P);
	fw_->data.light_direction = light_direction;
	// UNIT.16

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
	if (enable_dynamic_shader)
	{
		fw_->dynamic_texture->clear(fw_->immediate_context.Get());
		fw_->dynamic_texture->activate(fw_->immediate_context.Get());
		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), nullptr, 0, 0, fw_->effect_shaders[0].Get());
		fw_->dynamic_texture->deactivate(fw_->immediate_context.Get());
		fw_->immediate_context->PSSetShaderResources(15, 1, fw_->dynamic_texture->shader_resource_views[0].GetAddressOf());
	}
	else
	{
		ID3D11ShaderResourceView* null_srv[] = { nullptr };
		fw_->immediate_context->PSSetShaderResources(15, 1, null_srv);
	}

	// RADIAL_BLUR
	if (enable_radial_blur)
	{
		fw_->immediate_context->UpdateSubresource(fw_->constant_buffers[1].Get(), 0, 0, &fw_->radial_blur_data, 0, 0);
		fw_->immediate_context->PSSetConstantBuffers(2, 1, fw_->constant_buffers[1].GetAddressOf());
	}

	// UNIT.32
	fw_->framebuffers[0]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[0]->activate(fw_->immediate_context.Get());

	// DYNAMIC_BACKGROUND
	if (enable_dynamic_background)
	{
		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
		fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
		fw_->immediate_context->OMSetBlendState(fw_->blend_states[static_cast<size_t>(BLEND_STATE::NONE)].Get(), nullptr, 0xFFFFFFFF);
		fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), nullptr, 0, 0, fw_->effect_shaders[1].Get());
	}

	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_ON_ZW_ON)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::SOLID)].Get());
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
	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(fw_->scaling.x, fw_->scaling.y, fw_->scaling.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(fw_->rotation.x, fw_->rotation.y, fw_->rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(fw_->translation.x, fw_->translation.y, fw_->translation.z) };
	DirectX::XMFLOAT4X4 world;
	// UNIT.21

	// === skinned_meshes[2]
	for (const auto& P : patties)
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(P.pos.x, P.pos.y - P.half_extents.y, P.pos.z); // ※今のモデルと同じXオフセット
		XMMATRIX S = XMMatrixScaling(0.34f, 0.34f, 0.34f);

		XMVECTOR q = XMLoadFloat4(&P.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);  // ★ パティ個別の回転

		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, C * S * Rp * T);
		skinned_meshes[2]->render(fw_->immediate_context.Get(), world, material_color, nullptr, flat_shading);
	}

	if (patty_collider_visible)
	{
		using namespace DirectX;

		// キューブcerealの実寸（1辺サイズ）を取得して正規化
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

			// モデルと同じ場所に配置（今はX=-0.3固定なので合わせる）
			XMMATRIX Tbox = XMMatrixTranslation(P.pos.x, P.pos.y, P.pos.z);
			XMVECTOR q = XMLoadFloat4(&P.rotQuat);
			XMMATRIX Rp = XMMatrixRotationQuaternion(q);

			XMFLOAT4X4 world_box;
			XMStoreFloat4x4(&world_box, C * Sbox * Rp * Tbox);

			// 半透明で描画（線ではなく中身あり。嫌ならアルファ上げ下げする）
			//skinned_meshes[6]->render(fw_->immediate_context.Get(), world_box,{ 1.0f, 0.2f, 0.2f, 0.35f }, nullptr, true);
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
		flat_shading = false; // Object3 はフラット描画
		skinned_meshes[3]->render(fw_->immediate_context.Get(), world, material_color, nullptr, flat_shading);
		flat_shading = prev_flat;
	}
	if (ifMenu == 1)
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
		flat_shading = false;

		// ★ ここが可変になる
		skinned_meshes[14]->render(
			fw_->immediate_context.Get(),
			world,
			material_color,
			nullptr,
			flat_shading
		);

		flat_shading = prev_flat;
	}
	else if (ifMenu == 2)
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
		flat_shading = false;

		// ★ ここが可変になる
		skinned_meshes[15]->render(
			fw_->immediate_context.Get(),
			world,
			material_color,
			nullptr,
			flat_shading
		);

		flat_shading = prev_flat;
	}
	else if (ifMenu == 3)
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
		flat_shading = false;

		// ★ ここが可変になる
		skinned_meshes[16]->render(
			fw_->immediate_context.Get(),
			world,
			material_color,
			nullptr,
			flat_shading
		);

		flat_shading = prev_flat;
	}
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(
			pate_position.x,
			pate_position.y - pate_half_extents.y - 0.005,
			pate_position.z
		);
		XMMATRIX Scale = XMMatrixScaling(0.02f, 0.02f, 0.02f);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y + 6.3,
			rotation_object3.z
		);

		XMFLOAT4X4 world_local;
		XMStoreFloat4x4(&world_local, C * Scale * R * T);

		// ワールド保存（当たり判定用など）
		world_obj4 = world_local;

		// ★ 5秒経過後のみ描画する
		if (StartTime > 5.0f)
		{
			skinned_meshes[4]->render(
				fw_->immediate_context.Get(),
				world_local,
				material_color,
				nullptr,
				flat_shading
			);
		}
	}

	//// === skinned_meshes[7]（アニメーションなし）===
	///変更箇所
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(kusi.pos.x, kusi.pos.y, kusi.pos.z);
		XMMATRIX Scale = XMMatrixScaling(0.07f, 0.16f, 0.07f);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y,
			rotation_object3.z
		);

		XMFLOAT4X4 world_local;
		XMStoreFloat4x4(&world_local, C * Scale * R * T);

		// 描画に使う world を保存しておく（後でバウンディングボックス計算に使う）
		world_obj4 = world_local;

		skinned_meshes[7]->render(fw_->immediate_context.Get(), world_local, fw_->material_color, nullptr, fw_->flat_shading);
	}
	{
		DirectX::XMMATRIX T11 = DirectX::XMMatrixTranslation(
			translation_object11.x,
			translation_object11.y,
			translation_object11.z
		);
		// 行列作成
		DirectX::XMMATRIX Scale4 =
			DirectX::XMMatrixScaling(scaleUniform_, scaleUniform_, scaleUniform_);
		DirectX::XMMATRIX R11 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object11.x,
			rotation_object11.y,
			rotation_object11.z
		);

		DirectX::XMFLOAT4X4 world4;
		DirectX::XMStoreFloat4x4(&world4, C * Scale4 * R11 * T11);
		animation::keyframe keyframe_walk{};

		if (skinned_meshes[11]->animation_clips.size() > 0)
		{
			int clip_index{ 0 };
			int frame_index{ 0 };

			animation& anim_walk = skinned_meshes[11]->animation_clips.at(clip_index);

			if (isAnimationStarted && !isAnimationFinished)   // ★ 再生中のみ更新
			{
				//frame_index = static_cast<int>(animation_tick_walk * anim_walk.sampling_rate);
				frame_index = static_cast<int>(animation_tick * anim_walk.sampling_rate);

				if (frame_index >= anim_walk.sequence.size() - 1)
				{
					frame_index = static_cast<int>(anim_walk.sequence.size() - 1); // ★ 最終フレーム固定
					isAnimationFinished = true;   // ★ 再生完了
				}
				else
				{
					//animation_tick_walk += elapsedTime * animation_speed;

					animation_tick += elapsedTime * animation_speed;
				}
			}
			else if (!isAnimationStarted)
			{
				frame_index = 0;   // メニュー中は最初のフレーム固定
			}
			else
			{
				// ★ 再生終了後は最終フレームを保持
				frame_index = static_cast<int>(anim_walk.sequence.size() - 1);
			}

			keyframe_walk = anim_walk.sequence.at(frame_index);
		}
		bool prev_flat = flat_shading;
		flat_shading = true;
		skinned_meshes[11]->render(fw_->immediate_context.Get(), world4, material_color, &keyframe_walk, flat_shading);
		flat_shading = prev_flat;
	}

	{
		using namespace DirectX;

		// ✅ まず、参照となる「キューブcereal(skinned_mesh[6])」の実寸を取得
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

		// ✅ 後で使う場合にワールド行列を保存（オプション）
		world_obj4 = world_box;
	}

	// === skinned_meshes[8] = buns.cereal を1体表示 ===
	if (bun.exists)
	{
		using namespace DirectX;
		// cerealはセンチ系のことが多いので、Pattyと同じくらいの見た目スケールに
		XMMATRIX T = XMMatrixTranslation(bun.pos.x, bun.pos.y, bun.pos.z);
		XMMATRIX S = XMMatrixScaling(0.28f, 0.28f, 0.28f); // 必要なら調整
		XMFLOAT4X4 world_bun;
		XMStoreFloat4x4(&world_bun, C * S * R * T);
		flat_shading = true;
		skinned_meshes[8]->render(fw_->immediate_context.Get(), world_bun, material_color, nullptr, flat_shading);
	}

	// （任意）Bunの当たりAABBを可視化
	if (bun.exists && bun_collider_visible)
	{
		using namespace DirectX;

		// キューブcerealの実寸（1辺サイズ）を取得
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
		//skinned_meshes[6]->render(fw_->immediate_context.Get(), world_box, bun_collider_color, nullptr, true);
	}

	for (const auto& cheese : cheeses)
	{
		using namespace DirectX;
		// C.pos ではなく cheese.pos に変更 - P.half_extents.y
		XMMATRIX T = XMMatrixTranslation(cheese.pos.x, cheese.pos.y - cheese.half_extents.y - 0.01f, cheese.pos.z);
		XMMATRIX S = XMMatrixScaling(0.3f, 0.35f, 0.3f);

		// C.rotQuat ではなく cheese.rotQuat に変更
		XMVECTOR q = XMLoadFloat4(&cheese.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);

		XMFLOAT4X4 world;
		// ここにある 'C' は、外側で定義されている「座標変換行列」を指すようになるのでエラーが消えます
		//XMStoreFloat4x4(&world, C * S * R * Rp * T);

		XMStoreFloat4x4(&world, C * S * Rp * T);

		// skinned_meshes[10] が Cheese
		skinned_meshes[10]->render(fw_->immediate_context.Get(), world, material_color, nullptr, true);
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
			//XMStoreFloat4x4(&world_box, C * Sbox * R * Rp * Tbox);
			XMStoreFloat4x4(&world_box, C * Sbox * Rp * Tbox);

			// 黄色っぽく表示
			//skinned_meshes[6]->render(fw_->immediate_context.Get(), world_box, { 1.0f, 1.0f, 0.0f, 0.35f }, nullptr, true);
		}
	}

	// ★ Buns_under 描画
	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(bunUnder.pos.x, bunUnder.pos.y, bunUnder.pos.z);
		XMMATRIX S = XMMatrixScaling(0.28f, 0.28f, 0.28f); // 他のモデルとスケールを合わせる

		XMVECTOR q = XMLoadFloat4(&bunUnder.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);

		XMFLOAT4X4 world;
		//XMStoreFloat4x4(&world, C * S * R * Rp * T);
		XMStoreFloat4x4(&world, C * S * Rp * T);
		// skinned_meshes[9] が Buns_under
		flat_shading = true;
		skinned_meshes[9]->render(fw_->immediate_context.Get(), world, material_color, nullptr, flat_shading);
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
		//skinned_meshes[6]->render(fw_->immediate_context.Get(), world_box, { 0.2f, 0.2f, 1.0f, 0.35f }, nullptr, true);
	}

	for (const auto& V : vegetables)
	{
		using namespace DirectX;
		// 位置調整 (モデルの原点に合わせてYオフセットが必要なら調整)
		XMMATRIX T = XMMatrixTranslation(V.pos.x, V.pos.y - V.half_extents.y, V.pos.z);
		XMMATRIX S = XMMatrixScaling(0.0034f, 0.0034f, 0.0034f); // スケールはモデルに合わせて調整

		XMVECTOR q = XMLoadFloat4(&V.rotQuat);
		XMMATRIX Rp = XMMatrixRotationQuaternion(q);

		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, C * S * Rp * T);

		// skinned_meshes[12] を使用
		if (skinned_meshes[12])
		{
			skinned_meshes[12]->render(fw_->immediate_context.Get(), world, material_color, nullptr, flat_shading);
		}
	}
	{
		DirectX::XMMATRIX T4 = DirectX::XMMatrixTranslation(
			translation_object12.x,
			translation_object12.y,
			translation_object12.z
		);
		DirectX::XMMATRIX Scale5 = DirectX::XMMatrixScaling(scaleUniform_1, scaleUniform_1, scaleUniform_1);
		DirectX::XMMATRIX R4 = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object12.x,
			rotation_object12.y,
			rotation_object12.z
		);

		DirectX::XMFLOAT4X4 world4;
		DirectX::XMStoreFloat4x4(&world4, C * Scale5 * R4 * T4);
		animation::keyframe keyframe_walk{};

		if (skinned_meshes[13]->animation_clips.size() > 0 && isPlaying)
		{
			int clip_index = 0;
			int frame_index = 0;

			animation& anim_walk = skinned_meshes[12]->animation_clips.at(clip_index);
			frame_index = static_cast<int>(animation_tick1 * anim_walk.sampling_rate);

			if (frame_index > anim_walk.sequence.size() - 1)
			{
				play_count++;

				if (play_count >= 1)
				{
					isPlaying = false;
					animation_tick1 = 0.0f;
					play_count = 0;
					frame_index = anim_walk.sequence.size() - 1;
				}
				else
				{
					animation_tick1 = 0.0f;
					frame_index = 0;
				}
			}
			else
			{
				animation_tick1 += elapsedTime;
			}

			keyframe_walk = anim_walk.sequence.at(frame_index);
		}

		if (isPlaying)   // ★Rを押した時だけ描画される
		{
			bool prev_flat = flat_shading;
			flat_shading = false;
			skinned_meshes[13]->render(
				fw_->immediate_context.Get(),
				world4,
				material_color,
				&keyframe_walk,
				flat_shading
			);
			flat_shading = prev_flat;
		}
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
		//skinned_meshes[6]->render(fw_->immediate_context.Get(), world_pate, { 0.2f, 0.9f, 1.0f, 0.25f }, nullptr, /*flat_shading*/ true);
	}
	// UNIT.32
	fw_->framebuffers[0]->deactivate(fw_->immediate_context.Get());

	if (fw_->enable_bloom)
	{
		// BLOOM
		fw_->bloomer->make(fw_->immediate_context.Get(), fw_->framebuffers[0]->shader_resource_views[0].Get());

		fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);	//ここに問題ありかも？
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

	fw_->framebuffers[1]->clear(fw_->immediate_context.Get());
	fw_->framebuffers[1]->activate(fw_->immediate_context.Get());
	fw_->immediate_context->OMSetDepthStencilState(fw_->depth_stencil_states[static_cast<size_t>(DEPTH_STATE::ZT_OFF_ZW_OFF)].Get(), 0);
	fw_->immediate_context->RSSetState(fw_->rasterizer_states[static_cast<size_t>(RASTER_STATE::CULL_NONE)].Get());
	fw_->bit_block_transfer->blit(fw_->immediate_context.Get(), fw_->framebuffers[0]->shader_resource_views[0].GetAddressOf(), 0, 1, fw_->pixel_shaders[0].Get());
	fw_->framebuffers[1]->deactivate(fw_->immediate_context.Get());

	if (selectMenu)
	{
		fw_->immediate_context->ClearRenderTargetView(fw_->render_target_view.Get(), color);
		sprite_batches[1]->begin(fw_->immediate_context.Get());
		sprite_batches[1]->render(fw_->immediate_context.Get(), 0, 0, 1920, 1080);
		sprite_batches[1]->end(fw_->immediate_context.Get());
	}
}// Renderのマージ！！}

void SceneGame::add_patty(const DirectX::XMFLOAT3& p,
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

	info.m_friction = 0.8f;

	btRigidBody* body = new btRigidBody(info);
	//body->setDamping(0.2f, 0.2f);

	float min_dimension = std::min({ half_extents.x, half_extents.y, half_extents.z }) * 2.0f;

	// 閾値: このサイズ以上動いたらCCD判定を行う（最小幅の半分程度が目安）
	body->setCcdMotionThreshold(min_dimension * 0.5f);

	// 半径: 掃引球の半径（物体の内側に収まるサイズにする）
	body->setCcdSweptSphereRadius(min_dimension * 0.2f);

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

void SceneGame::resolve_pair(int ia, int ib)
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

void  SceneGame::simulate_bun(float dt)
{
	if (!bun.exists) return;

	float bunGravityMultiplier = 15.0f;

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

	for (const auto& C : cheeses)
	{
		const float dx = bun.pos.x - C.pos.x;
		const float px = (bun.half_extents.x + C.half_extents.x) - fabsf(dx);
		if (px <= 0) continue;

		const float dy = bun.pos.y - C.pos.y;
		const float py = (bun.half_extents.y + C.half_extents.y) - fabsf(dy);
		if (py <= 0) continue;

		const float dz = bun.pos.z - C.pos.z;
		const float pz = (bun.half_extents.z + C.half_extents.z) - fabsf(dz);
		if (pz <= 0) continue;

		// 最小貫通軸で押し戻し（チーズの反発係数を考慮）
		const float e = std::max(bun.restitution, C.restitution);

		// 主に上下方向(Y軸)の衝突を解決して積み重なるようにする
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

void  SceneGame::simulate_patties(float dt)
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

bool SceneGame::IsIntersectAABB(
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

void SceneGame::CheckKusiPattyCollision()
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

void  SceneGame::add_cheese(const DirectX::XMFLOAT3& p,
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

	info.m_friction = 0.8f;

	btRigidBody* body = new btRigidBody(info);
	//body->setDamping(0.2f, 0.2f);
	float min_dimension = std::min({ half_extents.x, half_extents.y, half_extents.z }) * 2.0f;
	body->setCcdMotionThreshold(min_dimension * 0.5f);
	body->setCcdSweptSphereRadius(min_dimension * 0.2f);

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

void SceneGame::add_vegetable(const DirectX::XMFLOAT3& p,
	const DirectX::XMFLOAT3& half_extents,
	float mass,
	float restitution)
{
	Vegetable obj{};
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
		m_vegetableShape->calculateLocalInertia(mass, inertia);

	btDefaultMotionState* motion = new btDefaultMotionState(tr);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, m_vegetableShape, inertia);
	info.m_restitution = restitution;
	info.m_friction = 0.5f; // 必要に応じて摩擦を調整 (滑りやすくするなら下げる)

	btRigidBody* body = new btRigidBody(info);

	// CCD (Continuous Collision Detection) 設定
	float min_dimension = std::min({ half_extents.x, half_extents.y, half_extents.z }) * 2.0f;
	body->setCcdMotionThreshold(min_dimension * 0.5f);
	body->setCcdSweptSphereRadius(min_dimension * 0.2f);

	if (isSpawningPhase)
	{
		// スポーン中は回転をロック、Y軸移動のみ許可
		body->setLinearFactor(btVector3(0, 1, 0));
		body->setAngularFactor(btVector3(0, 0, 0));
	}

	m_btWorld->addRigidBody(body);
	obj.body = body;

	vegetables.push_back(obj);
}

void  SceneGame::CheckKusiCheeseCollision()
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

void  SceneGame::initBulletWorld()
{
	m_btConfig = new btDefaultCollisionConfiguration();
	m_btDispatcher = new btCollisionDispatcher(m_btConfig);
	m_btBroadphase = new btDbvtBroadphase();
	m_btSolver = new btSequentialImpulseConstraintSolver();

	m_btWorld = new btDiscreteDynamicsWorld(
		m_btDispatcher, m_btBroadphase, m_btSolver, m_btConfig);

	m_btWorld->getSolverInfo().m_numIterations = 20;

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

	m_vegetableShape = new btBoxShape(btVector3(
		vegetable_default_half_extents.x,
		vegetable_default_half_extents.y,
		vegetable_default_half_extents.z));

	// ★ Buns_under (静的剛体) の作成
	{
		// サイズ設定 (見た目に合わせて調整)
		bunUnder.half_extents = { 0.045f, 0.01, 0.045f };
		//bunUnder.half_extents = { 0.045f, 0.0020f, 0.045f };

		// 位置設定 (地面の上に置く)
		// x, z は他の具材が落ちてくる位置(-0.3, -10.0)に合わせる
		bunUnder.pos = { -0.3f, patty_ground_y, -10.0f };
		//bunUnder.pos = { -0.3f, patty_ground_y + bunUnder.half_extents.y, -10.0f };

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

std::vector<int>  SceneGame::generatePattern()
{
	result.clear(); // 結果をクリア

	// 比率設定
	int pattyCount = 5;
	int cheeseCount = 5;
	int vegetableCount = 0; // デフォルトは0
	int spawnLimit = 10;
	// ★ HARDモードなら野菜を追加
	if (ifMenu == 3) // 3 = HARD
	{
		pattyCount = 6;     // 3 -> 6
		cheeseCount = 6;    // 3 -> 6
		vegetableCount = 8; // 4 -> 8

		spawnLimit = 15;
	}

	for (int i = 0; i < pattyCount; ++i) result.push_back(0);   // 0: Patty
	for (int i = 0; i < cheeseCount; ++i) result.push_back(1);  // 1: Cheese
	for (int i = 0; i < vegetableCount; ++i) result.push_back(2); // 2: Vegetable (新規)

	// 乱数生成器の準備
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> typeDist(0, 1); // 0か1をランダムに出す

	// --- 2. 足りない分をランダムに埋める ---
	while (result.size() < spawnLimit)
	{
		// 0(パティ) または 1(チーズ) をランダムに追加
		result.push_back(typeDist(gen));
	}

	// --- 3. シャッフルする ---
	// 先にシャッフルすることで、この後の削除(resize)がランダムな削除と同じ意味になります
	std::shuffle(result.begin(), result.end(), gen);

	// --- 4. 余剰分を切り捨てる ---
	// SPAWN_MAX より多い場合は末尾を削除（シャッフル済みなのでランダム削除になる）
	if (result.size() > spawnLimit)
	{
		result.resize(spawnLimit);
	}

	return result;
}

void  SceneGame::Spawn(float dt)
{
	if (!allSpawn) return;

	if (isWaitingForPhysics)
	{
		physicsWaitTimer += dt; // 実時間でカウント（早送りしない）
		if (physicsWaitTimer >= 3.0f) // 3秒経過したら
		{
			EnablePhysicsForGameplay(); // ロック解除
			allSpawn = false;           // スポーン処理全体を終了
			isWaitingForPhysics = false;
			physicsWaitTimer = 0.0f;
		}
		return; // 待機中はスポーン処理を行わない
	}

	float spawnSpeedMultiplier = 5.0f;
	coolTimer += dt * spawnSpeedMultiplier;

	// 1回だけスポーン
	if (coolTimer >= interval && spawnIndex < result.size())
	{
		coolTimer -= interval;

		int type = result[spawnIndex++];

		if (type == 0)
		{
			add_patty({ patty_spawn_x, SPAWN_HIGH, patty_spawn_z },
				patty_default_half_extents, patty_default_mass, patty_default_restitution);
		}
		else if (type == 1) // Cheese
		{
			add_cheese({ patty_spawn_x, SPAWN_HIGH, patty_spawn_z },
				cheese_default_half_extents, cheese_default_mass, cheese_default_restitution);
		}
		// ★ 追加: Vegetable
		else if (type == 2)
		{
			add_vegetable({ patty_spawn_x, SPAWN_HIGH, patty_spawn_z },
				vegetable_default_half_extents, vegetable_default_mass, vegetable_default_restitution);
		}

		// 全て出現完了
		if (spawnIndex >= result.size())
		{
			isWaitingForPhysics = true;   // 待機開始
			physicsWaitTimer = 0.0f;      // タイマーリセット
		}
	}
}

void  SceneGame::EnablePhysicsForGameplay()
{
	if (!isSpawningPhase) return; // 既に解除済みなら何もしない
	isSpawningPhase = false;

	// 共通設定用のラムダ式（設定を一括で行う）
	auto setupBodyForGame = [](btRigidBody* body)
		{
			if (!body) return;

			// ロック解除
			body->setLinearFactor(btVector3(1, 1, 1));
			body->setAngularFactor(btVector3(1, 1, 0));

			//  修正: 回転減衰（第2引数）を上げる

			// これで「置いているだけなら倒れない粘り強さ」が出ます
			body->setDamping(0.05f, 0.01f);

			// 摩擦は高いままでOK
			body->setFriction(0.5f);

			body->activate(true);
		};

	// 全ての具材に適用
	for (auto& p : patties) setupBodyForGame(p.body);
	for (auto& c : cheeses) setupBodyForGame(c.body);
	for (auto& v : vegetables) setupBodyForGame(v.body);

	// 下のバンズ（土台）も摩擦だけは合わせておく（動かない設定は維持される）
	if (bunUnder.body)
	{
		bunUnder.body->setFriction(1.0f);
	}
}
void  SceneGame::CheckPateCollision()
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

	for (auto& v : vegetables)
	{
		if (v.body->getLinearFactor().x() != 0.0f) continue; // 既に解除済みならスキップ

		if (IsIntersectAABB(pate_position, pate_half_extents, v.pos, v.half_extents))
		{
			// ロック解除
			v.body->setLinearFactor(btVector3(1, 1, 1));
			v.body->setAngularFactor(btVector3(1, 1, 1));
			v.body->activate(true);
		}
	}
}

void SceneGame::CheckKusiVegetableCollision()
{
	using namespace DirectX;

	for (auto& veg : vegetables)
	{
		if (veg.isStuck) continue;

		bool hit = IsIntersectAABB(kusi.pos, kusi.half_extents, veg.pos, veg.half_extents);
		veg.isColliding = hit;

		if (hit)
		{
			veg.isStuck = true;

			if (veg.body) {
				// 物理演算を停止
				veg.body->setLinearVelocity(btVector3(0, 0, 0));
				veg.body->setAngularVelocity(btVector3(0, 0, 0));
				veg.body->setGravity(btVector3(0, 0, 0));
				// 必要に応じて衝突判定を無効化するなど
			}

			// メニュー判定用配列に記録 (3 = Vegetable とする)
			kusi_menu[menu_count] = 3;
			menu_count += 1;
		}
	}
}

void  SceneGame::Reset()
{
	// --- 物理エンジンのオブジェクト削除 ---

	// Patties (パティ) の削除
	for (auto& p : patties)
	{
		if (p.body)
		{
			m_btWorld->removeRigidBody(p.body);
			delete p.body->getMotionState();
			delete p.body;
		}
	}
	patties.clear();
	m_pattyBodies.clear();

	// Cheeses (チーズ) の削除
	for (auto& c : cheeses)
	{
		if (c.body)
		{
			m_btWorld->removeRigidBody(c.body);
			delete c.body->getMotionState();
			delete c.body;
		}
	}
	cheeses.clear();
	// m_cheeseBodies.clear(); // もし定義されている場合はクリア

	for (auto& v : vegetables)
	{
		if (v.body)
		{
			m_btWorld->removeRigidBody(v.body);
			delete v.body->getMotionState();
			delete v.body;
		}
	}
	vegetables.clear();

	// Bun (上のバンズ) の削除
	if (bun.body)
	{
		m_btWorld->removeRigidBody(bun.body);
		delete bun.body->getMotionState();
		delete bun.body;
		bun.body = nullptr;
	}
	bun.exists = false;
	bun.pos = { -0.3f, 1.3f, -10.0f }; // 初期位置
	bun.vel = { 0.0f, 0.0f, 0.0f };
	bun_spawned_initial = false;

	// --- ゲーム変数のリセット ---

	// 串の状態リセット
	kusi.pos = { -0.3f, 1.5f, -10.0f };
	kusi.kusi_End = false;
	kusi.move = false;

	// 獲得メニューのリセット
	for (int i = 0; i < MENU_MAX; i++)
	{
		kusi_menu[i] = 0;
	}
	menu_count = 0;

	// ゲーム進行状態リセット
	StartTime = 0;
	gameEnd = 0;
	spawnIndex = 0;
	coolTimer = 0.0f;
	allSpawn = true;
	isSpawningPhase = true;
	hitStop = false;

	isWaitingForPhysics = false;
	physicsWaitTimer = 0.0f;

	// スポーンパターンの再生成
	result.clear();
	generatePattern();

	// 初期生成フラグのリセット
	patty_spawned_initial = false;

	// BGMを最初から再生し直す場合（任意）
	if (bgmGame)
	{
		bgmGame->Stop();
		bgmGame->Play(true);
	}

	camera_focus = { 0.0f, 0.343f, -10.0f };
}
//*************************************
void SceneGame::DrawGUI()	//	GUIのマージ + SceneGameの問題修正
{							//*************************************
#ifdef USE_IMGUI
	ImGui::Begin("ImGUI");
	calculate_frame_stats(); // フレームごとに呼ぶ

	if (ifMenu == 1)
		ImGui::Text("EASY");
	else if (ifMenu == 2)
		ImGui::Text("NORMAL");
	else if (ifMenu == 3)
		ImGui::Text("HARD");

	if (gameEnd == 0)
		ImGui::Text("gameEnd = 0 : Defult");
	else if (gameEnd == 1)
		ImGui::Text("gameEnd = 1 : Game Clear");
	else if (gameEnd == 2)
		ImGui::Text("gameEnd = 2 : Game Over");

	ImGui::Text("Time: %.1f", StartTime);

	ImGui::Text("FPS: %.1f", current_fps);	ImGui::SameLine(); ImGui::Text("||Frame Time: %.2f ms", current_frame_time);

	ImGui::SliderFloat("camera_position.x", &fw_->camera_position.x, -100.0f, +100.0f);
	ImGui::SliderFloat("camera_position.y", &fw_->camera_position.y, -100.0f, +100.0f);
	ImGui::SliderFloat("camera_position.z", &fw_->camera_position.z, -100.0f, -1.0f);

	// ImGui側

	ImGui::Checkbox("flat_shading", &flat_shading);
	ImGui::Checkbox("Enable Dynamic Shader", &enable_dynamic_shader);
	ImGui::Checkbox("Enable Dynamic Background", &enable_dynamic_background);
	//ImGui::Checkbox("Enable RADIAL_BLUR", &enable_radial_blur);
	ImGui::Checkbox("Enable Bloom", &enable_bloom);

	ImGui::SliderFloat("light_direction.x", &light_direction.x, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.y", &light_direction.y, -1.0f, +1.0f);
	ImGui::SliderFloat("light_direction.z", &light_direction.z, -1.0f, +1.0f);

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
	ImGui::SliderFloat("bloom_extraction_threshold", &fw_->bloomer->bloom_extraction_threshold, +0.0f, +5.0f);
	ImGui::SliderFloat("bloom_intensity", &fw_->bloomer->bloom_intensity, +0.0f, +5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); // 内側の余白を増やす
	ImGui::SliderFloat("Camera Distance", &distance, 1.0f, 20.0f);
	ImGui::PopStyleVar();

	ImGui::Text("Stage Rotation");
	ImGui::SliderFloat("Rot X##obj30", &rotation_object3.x, -DirectX::XM_PI, DirectX::XM_PI);
	ImGui::SliderFloat("Rot Y##obj30", &rotation_object3.y, -DirectX::XM_PI, DirectX::XM_PI);
	ImGui::SliderFloat("Rot Z##obj30", &rotation_object3.z, -DirectX::XM_PI, DirectX::XM_PI);
	ImGui::SliderFloat("Pos X##obj3", &translation_object3.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Pos Y##obj3", &translation_object3.y, -10.0f, 10.0f);
	ImGui::SliderFloat("Pos Z##obj3", &translation_object3.z, -10.0f, 10.0f);

	//ImGui::Text("Camera Rotation");
	//ImGui::SliderFloat("camera", &rotateY, -10.0f, 10.0f);

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
	if (ImGui::Button("Add 1 v"))
	{
		add_vegetable({ patty_spawn_x, 0.8f, patty_spawn_z }, vegetable_default_half_extents, vegetable_default_mass, vegetable_default_restitution);
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

	ImGui::SliderFloat("kusi Y", &fw_->kusi.pos.y, -30.0f, 30.0f, "%.3f");

	ImGui::Separator();
	ImGui::TextUnformatted("[Buns]");
	ImGui::Checkbox("Show Bun Collider", &bun_collider_visible);

	ImGui::Separator();
	ImGui::TextUnformatted("[Buns]");

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

	switch (ifMenu)
	{
	case 1:
		for (int i = 0; i < 10; ++i) {
			ImGui::Text("Wmenu %d : %d", i, Emenu[i]);
		}
		break;

	case 2:
		for (int i = 0; i < 10; ++i) {
			ImGui::Text("Nmenu %d : %d", i, Nmenu[i]);
		}
		break;

	case 3:
		for (int i = 0; i < 10; ++i) {
			ImGui::Text("Hmenu %d : %d", i, Hmenu[i]);
		}
		break;
	}

	ImGui::End();
#endif
}