// UNIT.10
#include "texture.h"
#include "misc.h"

#include <WICTextureLoader.h>
using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>
#include <map>
using namespace std;

// UNIT.16
#include <sstream>
#include <iomanip>

// UNIT,31
#include <filesystem>
#include <DDSTextureLoader.h>

static map<wstring, ComPtr<ID3D11ShaderResourceView>> resources;
/////////////////////////////////
HRESULT load_texture_from_file(ID3D11Device* device,
	const wchar_t* filename,
	ID3D11ShaderResourceView** srv,
	D3D11_TEXTURE2D_DESC* desc)
{
	using std::filesystem::path;
	using std::filesystem::exists;
	using std::filesystem::is_directory;

	path p(filename);
	if (!exists(p) || is_directory(p)) {
		// ファイルが無い／フォルダだったら白テクスチャ
		return make_dummy_texture(device, srv, 0xFFFFFFFF, 16);
	}

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	// 並存する .dds があれば優先
	path dds = p; dds.replace_extension(L"dds");
	if (exists(dds)) {
		hr = DirectX::CreateDDSTextureFromFile(device, dds.c_str(), resource.GetAddressOf(), srv);
		if (FAILED(hr)) {
			hr = DirectX::CreateWICTextureFromFile(device, p.c_str(), resource.GetAddressOf(), srv);
		}
	}
	else {
		hr = DirectX::CreateWICTextureFromFile(device, p.c_str(), resource.GetAddressOf(), srv);
	}

	if (FAILED(hr)) {
		// 失敗しても落とさずダミー
		return make_dummy_texture(device, srv, 0xFFFFFFFF, 16);
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2d;
	hr = resource->QueryInterface(IID_PPV_ARGS(tex2d.GetAddressOf()));
	if (FAILED(hr)) {
		return make_dummy_texture(device, srv, 0xFFFFFFFF, 16);
	}
	tex2d->GetDesc(desc);
	return S_OK;
}
//////////////////////////////////////
void release_all_textures()
{
	resources.clear();
}

// UNIT.16
HRESULT make_dummy_texture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimension)
{
	HRESULT hr{ S_OK };

	wstringstream keyname;
	keyname << setw(8) << setfill(L'0') << hex << uppercase << value << L"." << dec << dimension;
	map<wstring, ComPtr<ID3D11ShaderResourceView>>::iterator it = resources.find(keyname.str().c_str());
	if (it != resources.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width = dimension;
		texture2d_desc.Height = dimension;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 1;
		texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;

		size_t texels = static_cast<size_t>(dimension) * dimension;
		unique_ptr<DWORD[]> sysmem{ make_unique< DWORD[]>(texels) };
		for (size_t i = 0; i < texels; ++i)
		{
			sysmem[i] = value;
		}

		D3D11_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pSysMem = sysmem.get();
		subresource_data.SysMemPitch = sizeof(DWORD) * dimension;
		subresource_data.SysMemSlicePitch = 0;

		ComPtr<ID3D11Texture2D> texture2d;
		hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
		shader_resource_view_desc.Format = texture2d_desc.Format;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource_view_desc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		resources.insert(std::make_pair(keyname.str().c_str(), *shader_resource_view));
	}
	return hr;
}