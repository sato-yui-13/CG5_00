#include "IndexBuffer.h"
#include "KamataEngine.h"
#include<d3d12.h>
#include<cassert>

using namespace KamataEngine;

void IndexBuffer::Create(const UINT size, const UINT stride) {// クラス内で取得するために追加
	//strideの値によって、１つのインデックスのフォーマットをきめる
	assert(stride == 2 || stride == 4);
	DXGI_FORMAT format = (stride==2)? DXGI_FORMAT_R16_UINT:DXGI_FORMAT_R32_UINT;
	
	
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// VertexResourceの生成 ------
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファ
	vertexResourceDesc.Width = size;                                // リソースのサイズ。今回はvector4を3頂点分

	// バッファの場合はこれらは１にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合これにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを生成する
	ID3D12Resource* vertexResource = nullptr;
	HRESULT hr =
	    dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	// 生成した頂点リソースを取っておく
	indexBuffer_ = vertexResource;

	// VertexBufferViewを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// リソースの先頭アドレスを使う
	indexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 仕様するリソースのサイズを長短３つ分のサイズ
	indexBufferView.SizeInBytes = size;
	// インデックスのフォーマット
	indexBufferView.Format = format;

	// vertexBufferView_を取っておく
	indexBufferView_ = indexBufferView;
}

//生成したインデックスバッファを繰り返す
ID3D12Resource* IndexBuffer::Get() { return indexBuffer_; }
//用意済みのいんっでくすバッファーを繰り返す
D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return &indexBufferView_; }

//コンストラクタ
IndexBuffer::IndexBuffer() {}

IndexBuffer::~IndexBuffer() {
	if (indexBuffer_)
		indexBuffer_->Release();
	indexBuffer_ = nullptr;
}
