
#include "KamataEngine.h"
#include "Shader.h"
//#include<d3dcompiler.inl>
#include <Windows.h>
#include <cassert>
using namespace KamataEngine;


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize();

	// DirectCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("Width:{},height:{}\na", w, h).c_str());

	// DirectXCommonクラスが管理している。コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// RootSignature作成
	// 構造体にデータを使用する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatuer{};
	descriptionRootSignatuer.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlog = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignatuer, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, &errorBlog);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendState
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面をかリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//塗りつぶすモードをソリッドにする
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);
	

	//ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	
	//PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()};
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()};
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0]=DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//どのように画面に色を打ち込むかの設定()
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//準備は整った。PSOを生成する
	ID3D12PipelineState* graphicsPipeLineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipeLineState));
	assert(SUCCEEDED(hr));

	//VertexResourceの生成 ------
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//CPUから書き込むヒープ

	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //バッファ
	vertexResourceDesc.Width = sizeof(Vector4) * 3;//リソースのサイズ。今回はvector4を3頂点分

	//バッファの場合はこれらは１にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合これにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを生成する
	ID3D12Resource* vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));


	//VertexBufferViewを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭アドレスを使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//仕様するリソースのサイズを長短３つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	//1つの頂点サイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	//頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};//左上
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};//上
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f}; // 右下

	//頂点リソースのマップを解除
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) 
		{
			break;
		}

		// 描画更新
		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rootSignature);     // RootSignatureの設定
		commandList->SetPipelineState(graphicsPipeLineState);     // PSOの設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVの設定
		// トポロジ
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点数、インデックス数、インデクスの開始位置、インデックスのオフセット
		commandList->DrawInstanced(3, 1, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	//解放処理
	vertexResource->Release();
	graphicsPipeLineState->Release();
	signatureBlob->Release();
	

	rootSignature->Release();
	

	KamataEngine::Finalize();

	return 0;
}