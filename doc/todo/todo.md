# todo

> 本文件由旧 `todo.txt` 整理而来；`todo.txt` 原始编码为 GBK，迁入后统一保存为 UTF-8 无 BOM。

## 当前补充

- **ensure 多线程去重**（`free/scl/assert.h` 宏内 `static bool s_logged`）：scl 进多线程项目且要严格 once 时换 `volatile int + scl::compare_and_swap`；当前 race 是良性退化（多几次 log/break），非必修。详见 `doc/architect/错误处理-哲学与工具集.md`。
- **PipelineKey 逐字段 hash / compare**（`catvulkan/cat/pipelineKey.cpp:1-31`）：当前 `XXH32(this, sizeof) + memcmp` 依赖 ctor `memset` 清 padding，结果正确但易碎——加字段时若忘了走 `memset` 路径会复现 cache miss + 每帧泄漏。等下次给 PipelineKey 加字段时一起改：逐字段 hash + 逐字段 `operator==`，彻底脱离 padding 依赖。文件内 TODO 注释已标。`static_assert(std::is_trivially_copyable<PipelineKey>)` 是 placebo，**不要加**——它只保证 memcpy/memcmp 合法，不防 padding 未初始化。
- **Pick pass RT 缩小为 1×1 + scissor**（`catvulkan/cat/vulkanRender.cpp:111` `_createRenderTarget` 处）：当前 pick 用全屏 RT，CPU buffer 已经是 1×1 = 4 字节。pick 已经只在点击事件触发（`Client::_clickSelectObject`），不是每帧成本，桌面端无压力。**移动端编辑器落地时再优化**：1×1 offscreen RT + 投影矩阵把点击点偏移到 RT 中心；或者 scissor 限制 + 调整 viewport。优化属独立改造，涉及 pick 投影路径变更，不要顺手做。

## 2025年8月20日

- 点击物体的时候，选中的是 mesh；如果 mesh 有骨骼动画，那么移动 mesh 并不会真正移动 mesh，而是移动 mesh 所在节点，mesh 的位置由骨骼动画根节点决定。
- 原始测试占位文本：
  - `test_linux_git2`
  - `testgog`

## 2025年7月2日

- 目前的大目标：完成编辑器状态下透视矩阵向正交矩阵的过渡。
- 这个过渡需要给出一个合理的正交矩阵范围，需要计算所有物体的包围盒。
- 子问题：如何计算 primitive 的包围盒；旧记录里提出需要从 GPU 中获取 vertex buffer 中的 position 来计算包围盒。

## 2024年7月8日

- 正在进行中的内容：
  - 地形基础。
  - 类似 Unity 的左上角坐标轴标识完善，需要修改 ImGui 的代码。

## 整体规划

### 模型

- [x] 原生数据 render（基本完成）。
- [x] mesh 加载（已完成）。
- [x] 基础材质纹理加载（已完成）。
- [x] 场景组织（已完成）。
- [x] skinned mesh 动画（基本完成）。
- [ ] 整理一下 render 的代码。
- [ ] FBX Windows 加载：非必要功能，延后；先在 glTF 基础上做后续开发。如重启，使用 SDK。
- [ ] FBX Android 加载：非必要功能，延后；考虑使用 openfbx 或者 assimp 代码。

### 基础光照

- [ ] 冯氏光照模型。
- [ ] Blinn-Phong。

### PBR 渲染

- [ ] 基础 PBR 光照。
- [ ] IBL 实现。

### 阴影

- [ ] 生成阴影图。
- [ ] 使用阴影图。
- [ ] 场景阴影 bake（使用 Unreal 的 LightMass）。

### 地形

- [ ] 刷高度图（地形刷子需要编辑器的支持）。
- [ ] 地形纹理。
- [ ] 植被 GPU Instance。
- [ ] 雪地沙地脚印，用 TS。

### 天空

- [ ] 实现最简单的天空盒。

### 水面

- [ ] 河流。
- [ ] 大海（FFT Fast Fourier Transform 快速傅里叶变换）。
- [ ] 瀑布。

### 场景编辑器

- [ ] 待补充。

## 2022年04月20日 星期三

### 渲染器

- 旧记录仅保留标题，未写具体内容。

### 光照

- [ ] 基础光照实现。
- [ ] PBR 与光照实现。

### 地形

- [ ] 地形高度图渲染和纹理。
- [ ] DrawInstance 画植被。
- [ ] 进阶：RVT 实现。

### 水体

- [ ] 静态水。
- [ ] 动态水。
- [ ] 涟漪：Unity 实现参考 <https://blog.csdn.net/ak47007tiger/article/details/115270022>。

### 阴影

- [ ] 基础 shadowmap 实现。

### 场景管理

- [ ] 八叉树空间管理。

### 资源管理

- [ ] 参考 catui 的资源管理方式。

### 物理

- [ ] 接入 PhysX 4.0。

### AI

- [ ] 寻路，接入 Recast and Detour navmesh。

### 其他

- 旧记录为“其他？？”占位，未写具体内容。

## 2021年09月16日 星期四

- [x] 全屏和最小化之后的恢复。
- [ ] 暂缓：接入 catui。现在接入了 ImGui，可以保证一些基本的信息显示与调试了。
- [ ] 暂缓：接入 HLSL 支持。该功能不是非常紧急。

## 2021年09月02日 星期四

- Vulkan uniform 的管理过于混乱，能不能抽象一下，简化对上结构。

## 2021年06月08日 星期二

- [ ] 干掉 global，只留下 env。
- [ ] 使用 `shaderCache` 类管理重复加载 shader。
- [ ] 明明只有一个 vertex shader 和一个 frag shader，但是 `_createShaderFromCode` 被调用了 4 次。
- [ ] 正在改造 `svkCreateShaderProgramFromCode`，支持从 shader 中直接读取 uniform 信息（旧记录写作 `unifrom`）。
- [ ] 纹理的 wrap 方式默认是 repeat，现在没问题，但是怎么设置？
- [ ] Vulkan 方式下，索引的 index 是 int16 还是 int32 无法指定。
- [ ] attr 在 shader 中的 layout 位置现在是写死的，考虑怎么改成可配置的。

```cpp
m_attrs[i].index		= attrIndex;
if (attrIndex < 0)
{
	printf("attr index = %s\n", attr.name);
	continue;
}
```

- [ ] 不支持的 attr 怎么处理：强制 render 跳过，或者先扫一遍有效的 attr？
- [ ] render 代码清理（现在基本只有一个 draw，没用的函数了）。
- [ ] PBR 材质支持，用 DamagedHelmet 调试。
- [ ] Sponza 模型显示不正确，感觉是摄像机裁剪问题：`D:\glTF-Sample-Viewer\assets\models\2.0\Sponza`。
- [ ] 鼠标移动手感像屎一样。
- [ ] 渲染状态切换，类似 Unity 的 SubShader / Pass 属性，是否开启深度之类的。

