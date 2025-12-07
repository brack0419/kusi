#include "shader.h"
#include "texture.h"
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneEnd.h"
#include "framework.h"
#include "System/Graphics.h"
#include "SceneGame.h"
#include "SceneEnd.h"
#include "tutorial.h"
#include <algorithm>
#define BT_NO_SIMD_OPERATOR_OVERLOADS
#include <bullet/btBulletDynamicsCommon.h>
#include <dxgi.h>
#undef min
#undef max
#include <vector>
#include<random>


SceneTutorial::SceneTutorial(HWND hwnd, framework* fw) : hwnd(hwnd), fw_(fw)
{
	//SceneManager::instance().ChangeScene(new SceneTitle(fw));
}

bool SceneTutorial::IsMouseClicked()
{
	static bool prev_click = false;
	bool now_click = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	bool triggered = now_click && !prev_click;
	prev_click = now_click;
	return triggered;
}


// 初期化
void SceneTutorial::Initialize()
{
	HRESULT hr{ S_OK };

	skinned_meshes[2] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Patty.fbx");
//	skinned_meshes[3] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Shop.fbx");
	skinned_meshes[3] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\shop7.fbx");
	skinned_meshes[4] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Spautla.fbx");
	//skinned_meshes[5] = std::make_unique<skinned_mesh>(device.Get(), ".\\resources\\bus.fbx");

	skinned_meshes[6] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\cube.000.fbx");
	fw_->dynamic_texture = std::make_unique<framebuffer>(fw_->device.Get(), 512, 512);

	skinned_meshes[7] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Skewers.fbx");
	// 修正: Buns_up.fbx -> Buns_up1.fbx
		// skinned_meshes[8] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_up.fbx");
	skinned_meshes[8] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_up1.fbx");

	// 修正: Buns_under.fbx -> Buns_under1.fbx
	// skinned_meshes[9] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_under.fbx");
	skinned_meshes[9] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Buns_under1.fbx");

	// 修正: Cheese.fbx -> Cheese2.fbx
	// skinned_meshes[10] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Cheese.fbx");
	skinned_meshes[10] = std::make_unique<skinned_mesh>(fw_->device.Get(), ".\\resources\\Cheese2.fbx");

	tutorialState = TutorialState::INTRO;

	initBulletWorld();   // ★ 追加

	patties.clear();

	cheeses.clear();

	// チュートリアル用のメニュー設定（例: パティ、チーズ）
	for (int i = 0; i < MENU_MAX; ++i) karimenu[i] = 0;
	karimenu[0] = 1; // Patty
	karimenu[1] = 2; // Cheese

	allSpawn = false; // 自動スポーンはOFFにして、ステートマシンで制御

	menu_count = 0;
	generatePattern();
	//prepareSpawnList();
	camera_focus = { 0.0f, 0.343f, -10.0f };
	/////////////マージ
	target_camera_focus_y = camera_focus.y;
	////////////
}

std::vector<int> SceneTutorial::generatePattern()
{
	result.clear();

	// 7. 作れないパターン
	if (tutorialState == TutorialState::BAD_PATTERN_SPAWN)
	{
		result = { 1, 1, 1, 0, 0 };
	}
	// 9. だるま落とし用パターン (例: パティ, チーズ, [邪魔なパティ])
	else if (tutorialState == TutorialState::GOOD_PATTERN_SPAWN)
	{
		result = { 0, 1, 0, 0, 1 }; // 0:Patty, 1:Cheese,
	}
	// デフォルト（デモ用）
	else
	{
		result = { 0, 1, 0 };
	}
	return result;
}

// 終了化
void SceneTutorial::Finalize()
{
}

// 更新処理
void SceneTutorial::Update(float elapsedTime)
{
	StartTime += elapsedTime;

	//if(GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	if (pateStop)
	{
		POINT p = { 1500, 540 };
		SetCursorPos(p.x, p.y);
	}

	// --------------------------------------------------
		// ホイール入力によるカメラ目標位置の更新
		// --------------------------------------------------
	target_camera_focus_y += fw_->wheel * cameraSpeed;

	// まず target を範囲内にクランプ
	const float minY = 0.22f;
	const float maxY = 0.8f;

	if (target_camera_focus_y < minY)
		target_camera_focus_y = minY;
	if (target_camera_focus_y > maxY)
		target_camera_focus_y = maxY;

	// --------------------------------------------------
	// カメラが下限に来た時：ホイール↓なら動けるように速度付与
	// --------------------------------------------------
	if (camera_focus.y <= minY + 0.0001f)  // 誤差吸収
	{
		camera_focus.y = minY;

		if (fw_->wheel < 0)   // ホイール下
		{
			cameraSpeed = 0.0005f;
			target_camera_focus_y += fw_->wheel * cameraSpeed;
		}
	}

	// --------------------------------------------------
	// 上限に来た時：ホイール↑なら動く
	// --------------------------------------------------
	if (camera_focus.y >= maxY - 0.0001f)
	{
		camera_focus.y = maxY;

		if (fw_->wheel > 0)   // ホイール上
		{
			cameraSpeed = 0.0005f;
			target_camera_focus_y += fw_->wheel * cameraSpeed;
		}
	}

	// --------------------------------------------------
	// スムーズに追従
	// --------------------------------------------------
	camera_focus.y += (target_camera_focus_y - camera_focus.y) * 1.0f * elapsedTime;

	// ホイールは毎フレーム消費
	fw_->wheel = 0.0f;

	// ★ チュートリアルのメインループ
	switch (tutorialState)
	{
	case TutorialState::INTRO:
		// 1. 説明中 -> クリックで次へ
		if (IsMouseClicked()) {
			tutorialState = TutorialState::SPAWN_DEMO;

			// スポーン準備
			generatePattern(); // デフォルトパターン
			spawnIndex = 0;
			allSpawn = true;   // スポーン開始
			isWaitingForPhysics = false;
		}
		pateStop = true;
		break;

	case TutorialState::SPAWN_DEMO:
		// 2. 具材落下中
		Spawn(elapsedTime);

		// 全て落ちきったら次へ（Spawn関数内で isWaitingForPhysics が false に戻るのを待つ）
		if (spawnIndex >= result.size() && !isWaitingForPhysics && !isSpawningPhase)
		{
			// 物理が安定したら次の説明へ
			tutorialState = TutorialState::EXPLAIN_FALL;
		}
		break;

	case TutorialState::EXPLAIN_FALL:
		// 3. 「具材を落としてみましょう」 -> クリックで次へ
		pateStop = false;

		if (IsMouseClicked()) {
			tutorialState = TutorialState::EXPLAIN_CONTROL;
		}
		break;

	case TutorialState::EXPLAIN_CONTROL:
		// 5. 操作説明 (Rキー待ち)
		// 4クリック相当はこの画面遷移で包含
		if (GetAsyncKeyState('R') & 0x8000) {
			Reset(); // 実際にリセット

			// 次の「悪いパターン」の準備
			tutorialState = TutorialState::BAD_PATTERN_SPAWN;
			pateStop = true;
			generatePattern();
			spawnIndex = 0;
			allSpawn = true;
		}
		break;

	case TutorialState::BAD_PATTERN_SPAWN:
		// 悪いパターン落下中
		Spawn(elapsedTime);
		if (spawnIndex >= result.size() && !isWaitingForPhysics && !isSpawningPhase)
		{
			tutorialState = TutorialState::EXPLAIN_BAD;
		}
		break;

	case TutorialState::EXPLAIN_BAD:
		// 7. 「作れないのでリセット」 -> クリックで次へ
		if (IsMouseClicked()) {
			tutorialState = TutorialState::WAIT_RESET_2;
		}
		break;

	case TutorialState::WAIT_RESET_2:
		// 8. Rキー待ち
		if (GetAsyncKeyState('R') & 0x8000) {
			Reset();

			// 次の「だるま落としパターン」の準備
			tutorialState = TutorialState::GOOD_PATTERN_SPAWN;
			generatePattern();
			spawnIndex = 0;
			allSpawn = true;
		}
		break;

	case TutorialState::GOOD_PATTERN_SPAWN:
		// 9. 良いパターン落下中
		Spawn(elapsedTime);

		// 全て落ちきったら次へ（Spawn関数内で isWaitingForPhysics が false に戻るのを待つ）
		if (spawnIndex >= result.size() && !isWaitingForPhysics && !isSpawningPhase)
		{
			if (IsMouseClicked())
			{
				tutorialState = TutorialState::EXPLAIN_DARUMA;
			}
		}

		break;

	case TutorialState::EXPLAIN_DARUMA:
		// 10. 「一番上だけはじく」説明
		pateStop = false;
		pate_position.y = 0.31;
		camera_focus.y = 0.283f;
		for (auto& p : patties) {
			if (p.body) {
				// 直線速度と回転速度をゼロにして静止させる
				p.body->setLinearVelocity(btVector3(0, 0, 0));
				p.body->setAngularVelocity(btVector3(0, 0, 0));


			}
		}
		if (IsMouseClicked()) {
			tutorialState = TutorialState::EXPLAIN_KUSI;
		}
		break;

	case TutorialState::EXPLAIN_KUSI:
		// 11. 「串を刺しましょう」
		kusi.move = true;
		hitStop = true;
		pateStop = true;
		if (IsMouseClicked() || (GetAsyncKeyState(VK_DOWN) & 0x8000)) {
			//if (IsMouseClicked() || (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
			kusi.color_a = true;
			tutorialState = TutorialState::WAIT_KUSI;

			// 串を下ろすフラグ（既存コード流用）
			bun.exists = true;
			bun.pos = { -0.3f, 3.0f, -10.0f };
		}
		break;

	case TutorialState::WAIT_KUSI:
		// 12. 串が降りてくる
		if (kusi.move) {
			kusi.pos.y -= KUSI_SPEED; // 速度は適宜調整
		}

		// 地面に到達判定
		if (kusi.pos.y + 0.05 <= patty_ground_y) {
			kusi.kusi_End = true;
			kusi.move = false;
			tutorialState = TutorialState::COMPLETED;
		}
		break;

	case TutorialState::COMPLETED:
		// 13. 完成 -> タイトルへ
		if (IsMouseClicked()) {
			SceneManager::instance().ChangeScene(new SceneTitle(fw_));
		}
		break;
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
		m_btWorld->stepSimulation(elapsedTime, 10, 1.0f / 60.0f);

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
	}

	simulate_bun(elapsedTime);
	CheckKusiCheeseCollision();
	CheckKusiPattyCollision();
}

// 描画処理
void SceneTutorial::Render(float elapsedTime)
{
	HRESULT hr{ S_OK };

	FLOAT color[]{ 0.2f, 0.2f, 0.2f, 1.0f };
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
			if (tutorialState == TutorialState::EXPLAIN_DARUMA)
			{
				pate_target.y = 0.31f; // Updateで指定している高さに固定
			}
			// それ以外で、右クリック中でなければマウス追従
			else if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
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

		if (tutorialState == TutorialState::EXPLAIN_DARUMA)
		{
			pate_position.y = 0.31f;
		}
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
		flat_shading = true; // Object3 はフラット描画
		skinned_meshes[3]->render(fw_->immediate_context.Get(), world, material_color, nullptr, flat_shading);
		flat_shading = prev_flat;
	}

	{
		using namespace DirectX;
		XMMATRIX T = XMMatrixTranslation(pate_position.x, pate_position.y - pate_half_extents.y - 0.005, pate_position.z);
		XMMATRIX Scale = XMMatrixScaling(0.02f, 0.02f, 0.02f);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
			rotation_object3.x,
			rotation_object3.y + 6.3,
			rotation_object3.z
		);

		XMFLOAT4X4 world_local;
		XMStoreFloat4x4(&world_local, C * Scale * R * T);

		// 描画に使う world を保存しておく（後でバウンディングボックス計算に使う）
		world_obj4 = world_local;

		skinned_meshes[4]->render(fw_->immediate_context.Get(), world_local, material_color, nullptr, flat_shading);
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

		if (kusi.color_a)
		{
			skinned_meshes[7]->render(fw_->immediate_context.Get(), world_local, fw_->material_color, nullptr, fw_->flat_shading);
		}
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
		//skinned_meshes[6]->render(
		//	fw_->immediate_context.Get(),
		//	world_box,
		//	{ 0.2f, 1.0f, 0.2f, 1.0f },  // RGBA（赤系で透明度あり）
		//	nullptr,
		//	true
		//);

		// ✅ 後で使う場合にワールド行列を保存（オプション）
		world_obj4 = world_box;
	}

	// === skinned_meshes[8] = buns.fbx を1体表示 ===
	if (bun.exists)
	{
		using namespace DirectX;
		// FBXはセンチ系のことが多いので、Pattyと同じくらいの見た目スケールに
		XMMATRIX T = XMMatrixTranslation(bun.pos.x, bun.pos.y, bun.pos.z);
		XMMATRIX S = XMMatrixScaling(0.28f, 0.28f, 0.28f); // 必要なら調整
		XMFLOAT4X4 world_bun;
		XMStoreFloat4x4(&world_bun, C * S * R * T);
		skinned_meshes[8]->render(fw_->immediate_context.Get(), world_bun, material_color, nullptr, flat_shading);
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
		skinned_meshes[6]->render(fw_->immediate_context.Get(), world_pate, { 0.2f, 0.9f, 1.0f, 0.25f }, nullptr, /*flat_shading*/ true);
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

}// Renderのマージ！！}

void SceneTutorial::add_patty(const DirectX::XMFLOAT3& p,
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
	//body->setDamping(0.2f, 0.2f);
	body->setLinearFactor(btVector3(0, 1, 0));

	if (isSpawningPhase)
	{
		// LinearFactor: 1=動く, 0=動かない (X, Y, Z) -> Yだけ動かす


		// AngularFactor: 1=回転する, 0=回転しない -> 全てロック
		body->setAngularFactor(btVector3(0, 0, 0));
	}

	m_btWorld->addRigidBody(body);

	obj.body = body;

	patties.push_back(obj);
	m_pattyBodies.push_back(body);
}

void SceneTutorial::resolve_pair(int ia, int ib)
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

void  SceneTutorial::simulate_bun(float dt)
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

void  SceneTutorial::simulate_patties(float dt)
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


//bool IsIntersectAABB(
//	const DirectX::XMFLOAT3& posA, const DirectX::XMFLOAT3& halfA,
//	const DirectX::XMFLOAT3& posB, const DirectX::XMFLOAT3& halfB)
//{
//	using namespace DirectX;
//
//	// 各軸方向での距離を計算
//	float dx = fabs(posA.x - posB.x);
//	float dy = fabs(posA.y - posB.y);
//	float dz = fabs(posA.z - posB.z);
//
//	// 各軸で重なっているかチェック
//	if (dx > (halfA.x + halfB.x)) return false;
//	if (dy > (halfA.y + halfB.y)) return false;
//	if (dz > (halfA.z + halfB.z)) return false;
//
//	// すべての軸で重なっていれば交差している
//	return true;
//}

void SceneTutorial::CheckKusiPattyCollision()
{
	using namespace DirectX;

	for (auto& patty : patties)
	{
		if (patty.isStuck)
		{
			// すでに刺さっている場合は無視（動かない）
			continue;
		}

		bool hit = SceneGame::IsIntersectAABB(kusi.pos, kusi.half_extents, patty.pos, patty.half_extents);
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

void  SceneTutorial::add_cheese(const DirectX::XMFLOAT3& p,
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
	//body->setDamping(0.2f, 0.2f);

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


void  SceneTutorial::CheckKusiCheeseCollision()
{
	using namespace DirectX;

	for (auto& cheese : cheeses)
	{
		if (cheese.isStuck) continue;

		// IsIntersectAABB は既存の関数を利用
		bool hit = SceneGame::IsIntersectAABB(kusi.pos, kusi.half_extents, cheese.pos, cheese.half_extents);
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

void  SceneTutorial::initBulletWorld()
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


void  SceneTutorial::Spawn(float dt)
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
		else
		{
			add_cheese({ patty_spawn_x, SPAWN_HIGH, patty_spawn_z },
				cheese_default_half_extents, cheese_default_mass, cheese_default_restitution);
		}

		// 全て出現完了
		if (spawnIndex >= result.size())
		{
			isWaitingForPhysics = true;   // 待機開始
			physicsWaitTimer = 0.0f;      // タイマーリセット
		}
	}
	camera_focus = { 0.0f, 0.343f, -10.0f };

}

void  SceneTutorial::EnablePhysicsForGameplay()
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
void  SceneTutorial::CheckPateCollision()
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
		if (SceneGame::IsIntersectAABB(pate_position, pate_half_extents, p.pos, p.half_extents))
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

		if (SceneGame::IsIntersectAABB(pate_position, pate_half_extents, c.pos, c.half_extents))
		{
			// ★ ロック解除
			c.body->setLinearFactor(btVector3(1, 1, 1));
			c.body->setAngularFactor(btVector3(1, 1, 1));
			c.body->activate(true);
		}
	}
}
void  SceneTutorial::Reset()
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

}
//*************************************
void SceneTutorial::DrawGUI()	//	GUIのマージ + SceneGameの問題修正
{							//*************************************
#ifdef USE_IMGUI
	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Tutorial Guide"); // ウィンドウ開始

	ImGui::Text("Pate pos = (%.2f, %.2f, %.2f)", pate_position.x, pate_position.y, pate_position.z);

	switch (tutorialState)
	{
	case TutorialState::INTRO:
		ImGui::TextWrapped(u8"1. このゲームはメニュー通りになるように\n食材をだるま落としするゲームです。");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8">> クリックで次へ");
		break;
	case TutorialState::SPAWN_DEMO:
		ImGui::TextWrapped(u8"2. 具材が落ちてきます...\n(物理演算中)");
		break;
	case TutorialState::EXPLAIN_FALL:
		ImGui::TextWrapped(u8"3. まずは具材をパテで落としてみましょう。。");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8">> クリックで次へ");
		break;
	case TutorialState::EXPLAIN_CONTROL:
		ImGui::TextWrapped(u8"5. 【操作説明】\n[R]キー: 具材をリセット\n[マウス]: パテを動かす:マウスホイール視点を動かす");
		ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), u8"Rキーを押してリセットしてください");
		break;
	case TutorialState::BAD_PATTERN_SPAWN:
		ImGui::TextWrapped(u8"悪いパターンが出現中...");
		break;
	case TutorialState::EXPLAIN_BAD:
		ImGui::TextWrapped(u8"7. メニュー表通りに作れない配置です。\nこれではクリアできません。");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8">> クリックで次へ");
		break;
	case TutorialState::WAIT_RESET_2:
		ImGui::TextWrapped(u8"8. もう一度リセットしましょう。");
		ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), u8"Rキーを押してリセット");
		break;
	case TutorialState::GOOD_PATTERN_SPAWN:
		ImGui::TextWrapped(u8"9. この並びなら作れますね！\n見てみましょう。");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8">> クリックで次へ");
		break;
	case TutorialState::EXPLAIN_DARUMA:
		ImGui::TextWrapped(u8"10. 一番上の具材が余分です。\n赤いパテをマウスで操作して、\n「一番上だけ」弾き飛ばしてください！");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8">> クリックで次へ");
		break;

	case TutorialState::EXPLAIN_KUSI:
		ImGui::TextWrapped(u8"11. メニュー表通りになりましたね。\n仕上げに串を刺しましょう。");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8"左クリックで串を刺す");
		break;
	case TutorialState::WAIT_KUSI:
		ImGui::TextWrapped(u8"12. 串が刺さります...");
		break;
	case TutorialState::COMPLETED:
		ImGui::TextWrapped(u8"13. 完成です！\nチュートリアル終了。");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), u8"クリックしてタイトルへ");
		break;
	}

	ImGui::End(); // ウィンドウ終了
#endif
}
