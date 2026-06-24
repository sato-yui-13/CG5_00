#include "Shader.h"
#include <cassert>
#include <d3dcompiler.h>

void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// wstring=>string　文字列変換
	std::string mbShaderModel = std::string(shaderModel.begin(), shaderModel.end());

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダーファイル名
	    nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", mbShaderModel.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shaderBlob, &errorBlob);

	// エラーが発生した場合、止める
	if (FAILED(hr)) {

		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}

		assert(false);
	}
	// 生成したshaderBlobをとっておく
	blob_ = shaderBlob;
}

void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel) {
	// DXC(DirectX shader Compiler)を初期化
	static IDxcUtils* dxcUtils = nullptr;
	static IDxcCompiler3* dxcCompiler = nullptr;
	static IDxcIncludeHandler* includeHandeler = nullptr;
	HRESULT hr;

	if (dxcUtils == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		assert(SUCCEEDED(hr));
	}

	if (dxcCompiler == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		assert(SUCCEEDED(hr));
	}

	if (includeHandeler == nullptr) {
		hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandeler);
		assert(SUCCEEDED(hr));
	}

	// 1. hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容をDxuBufferに設定する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2. Compileする
	//    Compilerに必要なコンパイルオプションの準備
	LPCWSTR arguments[] = {
	    filePath.c_str(), L"-E", L"main", L"-T", shaderModel.c_str(), L"-zi", L"-Qembed_debug", L"-Od", L"-Zpr",
	};
	IDxcResult* shaderResult = nullptr;

	hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments), includeHandeler, IID_PPV_ARGS(&shaderResult));
	// コンパイルエラーではなくdxcが起動できないなどの致命的な状況
	assert(SUCCEEDED(hr));

	// 3. 警告・エラーが出てないか確認する
	IDxcBlobUtf8* shaderError = nullptr;
	IDxcBlobWide* nameBlob = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), &nameBlob);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		OutputDebugStringA(shaderError->GetStringPointer());
		assert(false);
	}

	// 4. Compile結果を受け取る
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &nameBlob);
	assert(SUCCEEDED(hr));

	// もう使わないresourceを解放
	shaderSource->Release();
	shaderResult->Release();

	// 実行用のバイナリを取っておく
	dxcBlob_ = shaderBlob;
}

// コンパイル済みのシェーダーデーターを繰り返す
ID3DBlob* Shader::GetBlob() { return blob_; }

IDxcBlob* Shader::GetDxcBlob() { return dxcBlob_; }

// コンストラクタ
Shader::Shader() {}

// デストラクタ
Shader::~Shader() {

	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}

	if (dxcBlob_ != nullptr) {
		dxcBlob_->Release();
		dxcBlob_ = nullptr;
	}
}
