#pragma once

#include "framework.h"
#include"Scene.h"
#define BT_NO_SIMD_OPERATOR_OVERLOADS
#include <bullet/btBulletDynamicsCommon.h>


#define MENU_MAX 10
#define SPAWN_MAX 10
#define SPAWN_HIGH 0.6f

#define KUSI_SPEED 0.001;

class SceneTutorial : public Scene
{
public:
	float cameraSpeed = 0.0005f;
	bool pateStop = false;

	enum class TutorialState {
		INTRO,              // 1. ゲーム説明
		SPAWN_DEMO,         // 2. 具材落下デモ
		EXPLAIN_FALL,       // 3. 落ちた後の説明
		EXPLAIN_CONTROL,    // 5. 操作説明 (R:リセット, P:パテ操作)
		WAIT_RESET_1,       // 6. プレイヤーのリセット待ち
		BAD_PATTERN_SPAWN,  // 7. 失敗パターン落下
		EXPLAIN_BAD,        // 7. 作れない説明
		WAIT_RESET_2,       // 8. もう一度リセット待ち
		GOOD_PATTERN_SPAWN, // 9. 成功（だるま落とし）パターン落下
		EXPLAIN_DARUMA,     // 10. だるま落とし実践指示
		EXPLAIN_KUSI,       // 11. 串を刺す指示
		WAIT_KUSI,          // 12. 串刺し中
		COMPLETED           // 13. 完成
	};

	// ★追加: 現在の状態を保持する変数
	TutorialState tutorialState = TutorialState::INTRO;

	SceneTutorial(HWND hwnd, framework* fw);
	~SceneTutorial() override {}

	void Initialize()override;

	void Finalize()override;

	void Update(float elapsedTime)override;

	void Render(float elapsedTime)override;

	void DrawGUI()override;

	bool IsMouseClicked();

	std::unique_ptr<skinned_mesh> skinned_meshes[11];

	std::unique_ptr<sprite_batch> sprite_batches[11];

	Microsoft::WRL::ComPtr<ID3D11PixelShader> effect_shaders[2];

	enum class SAMPLER_STATE { POINT, LINEAR, ANISOTROPIC, LINEAR_BORDER_BLACK, LINEAR_BORDER_WHITE, LINEAR_CLAMP };
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[6];

	enum class DEPTH_STATE { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF };
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states[4];

	enum class BLEND_STATE { NONE, ALPHA, ADD, MULTIPLY };
	//Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states[4];

	enum class RASTER_STATE { SOLID, WIREFRAME, CULL_NONE, WIREFRAME_CULL_NONE };
	//Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[4];

	bool hitStop = false;

	bool flat_shading = false;


	float target_camera_focus_y = 0.343f;

	DirectX::XMFLOAT3 camera_position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 light_direction{ -1.0, -0.8f, -1.0f, 0.0f };
	DirectX::XMFLOAT3 camera_focus{ 0.0f, 0.283f, -10.0f };
	float rotateX{ DirectX::XMConvertToRadians(5) };
	float rotateY{ DirectX::XMConvertToRadians(90) };
	POINT cursor_position;
	float wheel{ 0 };
	float distance{ 0.1f };

	DirectX::XMFLOAT3 center_of_rotation;
	DirectX::XMFLOAT3 translation{ 0, 0, 0 };
	DirectX::XMFLOAT3 scaling{ 1, 1, 1 };
	DirectX::XMFLOAT3 rotation{ 0, 0.643f, 0 };
	DirectX::XMFLOAT4 material_color{ 1 ,1, 1, 1 };
	DirectX::XMFLOAT3 rotation_object3{ 0.0f, 3.115f, 0.0f };
	DirectX::XMFLOAT3 rotation_object4{ 0.0f, 3.13f, 0.0f };
	DirectX::XMFLOAT3 translation_object3{ -0.254f, 0.0f, -9.391f };

	//ImGui追加設定
	// ピクセルシェーダー格納用配列
	bool enable_bloom = true;
	bool enable_dynamic_shader = false;
	bool enable_dynamic_background = false;
	bool enable_radial_blur = false;
	int current_effect = 0; // 0 = テクスチャ表示, 1 = 背景アニメーション

	struct parametric_constants
	{
		float extraction_threshold{ 0.8f };
		float gaussian_sigma{ 1.0f };
		float bloom_intensity{ 0.0f };
		float exposure{ 1.0f };
	};
	parametric_constants parametric_constants;

	struct radial_blur_constants
	{
		DirectX::XMFLOAT2 blur_center = { 0.5, 0.5 }; // center point where the blur is applied
		float blur_strength = 1.0f; // blurring strength
		float blur_radius = 0.5f; // blurred radiu
		float blur_decay = 0.2f; // ratio of distance to decay to radius
		float pads[3];
	};
	radial_blur_constants radial_blur_data;

	struct Kusi
	{
		DirectX::XMFLOAT3 pos{ -0.3f, 1.0f, -10.0f };
		DirectX::XMFLOAT3 half_extents{ 0.01f, 0.01f, 0.01f };

		bool kusi_End = false;
		bool move = false;

		bool color_a = false;
	};
	Kusi kusi;

	struct Patty {
		DirectX::XMFLOAT3 pos;          // 中心
		DirectX::XMFLOAT3 vel;
		DirectX::XMFLOAT3 half_extents; // AABB の半径 (hx, hy, hz)
		float mass;
		float restitution;

		bool isColliding = false; // ← 追加
		bool isStuck = false; // ← 串に刺さって固定されているか

		btRigidBody* body = nullptr;          // ★ 追加
		DirectX::XMFLOAT4 rotQuat{ 0,0,0,1 };   // ★ 見た目用の回転
	};
	std::vector<Patty> patties;
	std::vector<int> generatePattern();
	struct Cheese {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel;
		DirectX::XMFLOAT3 half_extents;
		float mass;
		float restitution;

		bool isColliding = false;
		bool isStuck = false;

		btRigidBody* body = nullptr;
		DirectX::XMFLOAT4 rotQuat{ 0,0,0,1 };
	};
	std::vector<Cheese> cheeses; // チーズ一覧

	btBoxShape* m_cheeseShape = nullptr;
	std::vector<btRigidBody*> m_cheeseBodies;

	DirectX::XMFLOAT3 cheese_default_half_extents{ 0.035f, 0.004f, 0.048f };
	float cheese_default_mass = 0.5f;
	float cheese_default_restitution = 0.0f;

	struct Bun {
		DirectX::XMFLOAT3 pos{ -0.3f, 1.3f, 0.0f };
		DirectX::XMFLOAT3 vel{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 half_extents{ 0.055f, 0.008f, 0.058f };
		float mass{ 1.0f };
		float restitution{ 0.0f };
		bool exists{ false };

		btRigidBody* body = nullptr;
		DirectX::XMFLOAT4 rotQuat{ 0, 0, 0, 1 };
	};
	Bun bun;
	btBoxShape* m_bunUpShape = nullptr;
	bool bun_spawned_initial = false;
	bool bun_collider_visible = true;
	DirectX::XMFLOAT4 bun_collider_color{ 0.2f, 1.0f, 0.2f, 0.35f };

	struct BunUnder {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 half_extents;
		DirectX::XMFLOAT4 rotQuat{ 0, 0, 0, 1 }; // 回転
		btRigidBody* body = nullptr;
	};
	BunUnder bunUnder;

	// Bullet用
	btBoxShape* m_bunUnderShape = nullptr;

	std::unique_ptr<bloom> bloomer;

	//	----------------
	//	   マージ部分
	//	----------------
	int gameEnd = 0;

	int kusi_menu[MENU_MAX] = { 0 };
	int karimenu[MENU_MAX] = { 1, 1, 2, 2, 1, 0, 0, 0, 0, 0 };

	bool pate_collider_enabled = true; // 衝突を有効化
	bool patty_collider_visible = true;
	DirectX::XMFLOAT4 patty_collider_color{ 1.0f, 0.2f, 0.2f, 0.35f };
	// コライダー中心は pate_position（既存）を使用
	DirectX::XMFLOAT3 pate_half_extents{ 0.08f, 0.004f, 0.01f }; // X=幅/2, Y=厚み/2, Z=奥行/2
	float pate_restitution = 0.0f; // 反発
	float pate_plane_y = 0.373f; // マウス追従に使う平面の高さ（pate_position.y に反映）
	//////////////////////////
	float patty_spawn_z = -10.0f; // 新規スポーン時に使うZ
	float patties_z_offset = 0.0f; // 全体シフト用の現在値
	float patties_z_offset_prev = 0.0f; // 前フレーム値（差分適用に利用）
	int patty_selected_index = -1; // 選択中（-1: なし）

	std::vector<int> spawnList;  // 0 = patty, 1 = cheese

	float StartTime = 0;

	float StartTimer = 0;

	int spawnIndex = 0;

	float coolTimer = 0.0f;

	const float interval = 0.3f;


	bool isWaitingForPhysics = false;
	float physicsWaitTimer = 0.0f;
	std::vector<int> result;
	bool allSpawn = true;

	btBoxShape* m_pattyShape = nullptr;
	btBoxShape* m_pateShape = nullptr;
	btRigidBody* m_pateBody = nullptr;

	std::vector<btRigidBody*>           m_pattyBodies;

	bool isSpawningPhase = true;
	bool patty_spawned_initial = false;

	btDefaultCollisionConfiguration* m_btConfig = nullptr;
	btCollisionDispatcher* m_btDispatcher = nullptr;
	btBroadphaseInterface* m_btBroadphase = nullptr;
	btSequentialImpulseConstraintSolver* m_btSolver = nullptr;
	btDiscreteDynamicsWorld* m_btWorld = nullptr;

	// グローバル物理パラメータ
	bool patty_sim_enabled = true;
	float patty_gravity_y = -0.3f; // 下向き重力（単位系に応じて微調整）
	float patty_ground_y = 0.235f; // 地面/テーブルの Y（シーンに合わせて）
	DirectX::XMFLOAT3 patty_default_half_extents{ 0.035f, 0.008f, 0.038f };
	float patty_default_mass = 1.0f;
	float patty_default_restitution = 0.0f; // Patty 同士の最小反発係数

	float patty_spawn_x = -0.3f;





	DirectX::XMFLOAT3 pate_position{ -0.4f, 3.0f, 0.0f };
	DirectX::XMFLOAT3 pate_target{ -0.4f, 3.0f, 0.0f };
	float pate_follow_smooth = 0.25f;
	float pate_fixed_x = -0.3f;
	float pate_fixed_z = 0.0f; //
	float pate_control_z = 0.0f; // ヒットするZ平面（固定Z）

	void add_cheese(const DirectX::XMFLOAT3& pos,
		const DirectX::XMFLOAT3& half_extents,
		float mass,
		float restitution);

	void CheckKusiCheeseCollision(); // 串との当たり判定

	void add_patty(const DirectX::XMFLOAT3& p,
		const DirectX::XMFLOAT3& half_extents,
		float mass,
		float restitution);

	void CheckKusiPattyCollision();

	void initBulletWorld();

	void simulate_patties(float dt);

	void resolve_pair(int ia, int ib);

	void simulate_bun(float dt);

	void prepareSpawnList();

	void Spawn(float dt);

	void EnablePhysicsForGameplay();

	void CheckPateCollision();



	void Reset();

private:
	CONST HWND hwnd;

	framework* fw_;

	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	float current_fps{ 0.0f };
	float current_frame_time{ 0.0f };

	void calculate_frame_stats()
	{
		++frames;
		float time = tictoc.time_stamp();
		if (time - elapsed_time >= 1.0f)
		{
			current_fps = static_cast<float>(frames);
			current_frame_time = 1000.0f / current_fps;

			frames = 0;
			elapsed_time += 1.0f;
		}
	}
};
