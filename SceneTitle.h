#pragma once

#include"System/Sprite.h"
#include "Scene.h"
#include "framework.h"

//タイトルシーン
class SceneTitle : public Scene
{
public:
	AudioSource* bgmTitle;

	SceneTitle(framework* fw);
	~SceneTitle()override {}

	//初期化
	void Initialize()override;

	//終了化
	void Finalize()override;

	//更新処理
	void Update(float elaspedTime)override;

	//描画処理
	void Render(float elapsedTime)override;

	//GUI描画
	void DrawGUI()override;

	std::unique_ptr<skinned_mesh> skinned_meshes[8];

	Microsoft::WRL::ComPtr<ID3D11PixelShader> effect_shaders[2];

	enum class SAMPLER_STATE { POINT, LINEAR, ANISOTROPIC, LINEAR_BORDER_BLACK, LINEAR_BORDER_WHITE, LINEAR_CLAMP };
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[6];

	enum class DEPTH_STATE { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states[4];

	enum class BLEND_STATE { NONE, ALPHA, ADD, MULTIPLY };
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states[4];

	enum class RASTER_STATE { SOLID, WIREFRAME, CULL_NONE, WIREFRAME_CULL_NONE };
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[4];

	std::unique_ptr<bloom> bloomer;

	DirectX::XMFLOAT3 translation_object0{ -0.254f, 0.0f, -9.391f };
	DirectX::XMFLOAT3 translation_object2{ -0.254f, 0.0f, -9.391f };
	DirectX::XMFLOAT3 translation_object3{ -0.254f, 0.0f, -9.391f };
	DirectX::XMFLOAT3 translation_object4{ -0.254f, 0.0f, -9.391f };
	DirectX::XMFLOAT3 translation_object5{ -0.254f, 0.0f, -9.391f };
	DirectX::XMFLOAT3 rotation_object0{ 0.0f, 3.115f, 0.0f };
	DirectX::XMFLOAT3 rotation_object2{ 0.0f, 3.115f, 0.0f };
	DirectX::XMFLOAT3 rotation_object3{ 0.0f, 3.115f, 0.0f };
	DirectX::XMFLOAT3 rotation_object4{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 rotation_object5{ 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4 material_color{ 1 ,1, 1, 1 };
	DirectX::XMFLOAT4 material_color1{ 1 ,1, 1, 1 };
	DirectX::XMFLOAT4 material_color2{ 1 ,1, 1, 1 };
	DirectX::XMFLOAT4 material_color3{ 1 ,1, 1, 1 };

	DirectX::XMFLOAT4 material_color_title6 = { 1,1,1,1 }; // mesh[6]専用

	bool flat_shading = false;

	float title0_brightness = 1.0f;
	float title2_brightness = 1.0f;
	float title5_brightness = 1.0f;
	float title6_brightness = 16.0f;   // 既存

private:
	Sprite* sprite = nullptr;
	framework* fw_;
	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	float current_fps{ 0.0f };
	float current_frame_time{ 0.0f };
	// framework.h に追加
	float camera_distance = 5.0f;   // 初期距離（好きな値）
	float anim_time_ = 0.0f;
	DirectX::XMFLOAT2 click_min = { 58.134f, 638.063f };
	DirectX::XMFLOAT2 click_max = { 542.062f, 849.329f };
	// タイトル出現演出用
	float titleAppearTimer_ = 0.0f;
	bool titleAppearing_ = true;

	DirectX::XMFLOAT3 titleStartPos_;
	DirectX::XMFLOAT3 titleTargetPos_;
	float fadeTimer = 0.0f;
	float fadeDuration = 2.0f;   // フェード完了までの秒数
	float fadeAlpha = 0.0f;      // 0.0 = 真っ暗, 1.0 = 完全表示
	bool  fadeCompleted = false;

	bool isHover = false;
	float defaultLightX = 0.0f;
	DirectX::XMFLOAT4 defaultTitle6Color = { 0,0,0,0 };

	float neonFlickerTimer3 = 0.0f;
	float neonFlickerIntensity3 = 1.0f;
	float neonTargetIntensity3 = 1.0f;

	float neonSubChannelDelay3 = 0.0f;   // 一部遅延用
	float neonNoisePhase3 = 0.0f;        // 微細ノイズ

	bool  neonPowerDrop3 = false;
	float neonShakeTime3 = 0.0f;
};