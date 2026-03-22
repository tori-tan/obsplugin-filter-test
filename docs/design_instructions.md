# 作業指示書：OBS 色調補正フィルタプラグイン（最小実装）

## 1. 目的

OBS Studio向けのフィルタプラグインを作成する。  
ソースに適用する「フィルタ」として動作し、リアルタイムで色調補正（明るさ・コントラスト・彩度）を行う。

* * *

## 2. 前提環境

### 対応環境

- OS: Windows 10/11
- OBS: 最新安定版（29以降を想定）
- ビルド: CMake + Visual Studio 2022

### 依存

- OBS Studioの開発ヘッダ（obs-studio repo）
- graphics API: OBS標準の `libobs`（DirectX経由）

* * *

## 3. プラグイン仕様

### 種別

- Source Filter（フィルタ）

### フィルタ名

- 表示名: `Simple Color Correction`

### 適用対象

- すべての映像ソース

* * *

## 4. 機能仕様

### パラメータ

以下をOBSのプロパティとして提供：

| パラメータ | 型 | 範囲 | デフォルト |
| --- | --- | --- | --- |
| brightness | float | -1.0 ～ 1.0 | 0.0 |
| contrast | float | 0.0 ～ 2.0 | 1.0 |
| saturation | float | 0.0 ～ 2.0 | 1.0 |

* * *

## 5. 処理仕様

### 描画処理

- OBSのフィルタレンダリングパイプラインにフック
- テクスチャを受け取り、シェーダで色変換

### シェーダ処理（HLSL/GLSL互換）

以下ロジックを実装：

    color.rgb += brightnesscolor.rgb = (color.rgb - 0.5) * contrast + 0.5float gray = dot(color.rgb, float3(0.299, 0.587, 0.114))color.rgb = lerp(float3(gray, gray, gray), color.rgb, saturation)

* * *

## 6. 実装構成

### ディレクトリ構成

    simple-color-filter/ ├─ CMakeLists.txt ├─ src/ │ ├─ plugin-main.c │ ├─ color-filter.c │ └─ color-filter.h └─ data/ └─ color_filter.effect

* * *

## 7. 実装詳細

### 7.1 plugin-main.c

- `obs_module_load` を実装
- フィルタを登録

    OBS_DECLARE_MODULE()OBS_MODULE_USE_DEFAULT_LOCALE("simple-color-filter", "en-US")bool obs_module_load(void){ obs_register_source(&color_filter_info); return true;}

* * *

### 7.2 フィルタ定義（color-filter.c）

#### 必須構造体

- `obs_source_info`

設定内容：

- `.id = "simple_color_filter"`
- `.type = OBS_SOURCE_TYPE_FILTER`
- `.output_flags = OBS_SOURCE_VIDEO`

* * *

### 7.3 ライフサイクル

#### create

- 設定値初期化
- effect（シェーダ）読み込み

#### destroy

- effect解放

#### update

- パラメータ更新

#### video\_render

- シェーダ適用
- パラメータをuniformとして渡す

* * *

### 7.4 プロパティUI

    obs_properties_t *props = obs_properties_create();obs_properties_add_float_slider(props, "brightness", "Brightness", -1.0, 1.0, 0.01);obs_properties_add_float_slider(props, "contrast", "Contrast", 0.0, 2.0, 0.01);obs_properties_add_float_slider(props, "saturation", "Saturation", 0.0, 2.0, 0.01);

* * *

### 7.5 シェーダ（color\_filter.effect）

    uniform float brightness;uniform float contrast;uniform float saturation;texture2d image;sampler_state textureSampler { Filter = Linear; AddressU = Clamp; AddressV = Clamp;};struct VertData { float4 pos : POSITION; float2 uv : TEXCOORD0;};float4 mainImage(VertData v) : TARGET{ float4 color = image.Sample(textureSampler, v.uv); color.rgb += brightness; color.rgb = (color.rgb - 0.5) * contrast + 0.5; float gray = dot(color.rgb, float3(0.299, 0.587, 0.114)); color.rgb = lerp(float3(gray, gray, gray), color.rgb, saturation); return color;}technique Draw { pass { pixel_shader = mainImage(v); }}

* * *

## 8. CMake設定

最低限：

    cmake_minimum_required(VERSION 3.16)project(simple-color-filter)find_package(libobs REQUIRED)add_library(simple-color-filter MODULE src/plugin-main.c src/color-filter.c)target_link_libraries(simple-color-filter libobs)install(TARGETS simple-color-filter DESTINATION obs-plugins/64bit)install(DIRECTORY data/ DESTINATION data/obs-plugins/simple-color-filter)

* * *

## 9. 動作確認

1. ビルド
2. OBSのプラグインディレクトリへ配置
3. OBS起動
4. 任意ソースにフィルタ追加
5. 「Simple Color Correction」が表示されること
6. スライダー操作でリアルタイム変化すること

* * *

## 10. 完了条件

- フィルタとして選択可能
- UIから3パラメータ変更可能
- 映像に即時反映
- クラッシュなし

* * *

## 11. 制約

- 外部ライブラリ追加禁止
- GPU処理のみ（CPUピクセル処理禁止）
- 単一effectファイルで完結

* * *

## 12. 拡張余地（今回は未実装）

- LUT対応
- RGB分離
- ガンマ補正
- プリセット保存
- 他フィルタとの合成順制御
