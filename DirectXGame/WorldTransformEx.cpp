#include "WorldTransformEx.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

// そして定数バッファへの転送も行う
void WorldTransformEx::UpdateMatrix() {

	// World変更行列を計算し、natWorld_に格納する
	matWorld_ = MakeAffineMatrix();
	// 定数バッファへ転送する
	TransferMatrix();
}

// アフィン変更行列をつくる
KamataEngine::Matrix4x4 WorldTransformEx::MakeAffineMatrix() {
	// Scale Matrix
	Matrix4x4 matScale = MakeScaleMatrix(scale_);

	// Rotation Matrix
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;

	// Translate
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	// World	Matrix
	Matrix4x4 matWorld = matScale * matRot * matTrans;

	return matWorld;
}