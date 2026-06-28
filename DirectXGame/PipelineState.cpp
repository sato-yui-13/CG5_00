#include "PipelineState.h"
#include <KamataEngine.h>

using namespace KamataEngine;
void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc) {
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 準備は整った。PSOを生成する
	ID3D12PipelineState* graphicsPipeLineState = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipeLineState));
	assert(SUCCEEDED(hr));

	//生成した　PipelineState をとっておく
	pipelineState_ = graphicsPipeLineState;

}

//生成したpipelineState_を繰り返す
ID3D12PipelineState* PipelineState::Get() { return pipelineState_; }

//コンストラクタ
PipelineState::PipelineState() {}

PipelineState::~PipelineState() { 
	if (pipelineState_) {
	
	pipelineState_->Release();
		pipelineState_ = nullptr;
	
	}
}
