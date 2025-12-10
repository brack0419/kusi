#pragma once

#include"System/Sprite.h"
#include "Scene.h"
#include "framework.h"

//タイトルシーン
class SceneEnd : public Scene
{
public:
	SceneEnd(framework* fw);
	~SceneEnd()override {}

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

	void SceneEnd::DrawNumber(int number, float x, float y, ID3D11DeviceContext* ctx);

	std::unique_ptr<skinned_mesh> skinned_meshes[8];

	std::unique_ptr<sprite_batch> sprite_batches[8];

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

	DirectX::XMFLOAT3 translation_object3{ -0.254f, 0.0f, -9.391f };

	DirectX::XMFLOAT3 rotation_object3{ 0.0f, 3.115f, 0.0f };

	DirectX::XMFLOAT4 material_color{ 1 ,1, 1, 1 };

	bool flat_shading = false;

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
};