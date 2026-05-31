# cat 引擎开发计划 v2

> 定稿时间：2026-05-31
> 执行者：单人开发
> 核心目标：通过项目"详细学习引擎各个细节"，**学习驱动 > 功能驱动**
> 节奏：以"周"为最小颗粒度，每个里程碑 1~4 周，结尾产出一篇 `doc/学习笔记/<主题>.md`

---

## 目录

- [一、总体原则](#一总体原则)
- [二、Phase 总览](#二phase-总览)
- [三、Phase 0：代码健康度修复 + 基础设施扎根](#三phase-0代码健康度修复--基础设施扎根-4-周不加新功能)
- [四、Phase 1：光照系统](#四phase-1光照系统-8-周)
- [五、Phase 2：阴影系统](#五phase-2阴影系统-6-周)
- [六、Phase 3：Jolt 物理接入](#六phase-3jolt-物理接入-5-周)
- [七、Phase 4：天空与大气](#七phase-4天空与大气-3-周)
- [八、Phase 5：地形系统](#八phase-5地形系统-6-周)
- [九、Phase 6：水体](#九phase-6水体-5-周)
- [十、Phase 7：场景管理](#十phase-7场景管理-3-周)
- [十一、Phase 8：后处理与色彩管线](#十一phase-8后处理与色彩管线-3-周)
- [十二、Phase 9：AI 寻路](#十二phase-9ai-寻路-2-周)
- [十三、Phase 10：GPU Driven 实验](#十三phase-10gpu-driven-实验开放期)
- [十四、贯穿全程的横切任务](#十四贯穿全程的横切任务)
- [十五、风险与对策](#十五风险与对策)
- [十六、出口判定](#十六出口判定)

---

## 一、总体原则

> 这 8 条原则在开始之前已经过一次自我确认，写在最前作为后续所有决策的"宪法"。

1. **学习驱动 > 功能驱动**
   每个里程碑明确"学到什么"，对照 UE / Unity / RTR4 / GPU Gems / GPU Pro 的概念点，不仅仅是列功能。

2. **先稳后扩**
   代码审查发现的 13 条必崩、文件大小写、Vulkan 退出 wait-idle 等问题必须先清。地基不稳，后面每加一个新模块都在踩同一类坑。

3. **基础设施一次性扎深**
   dirty flag、`globalMatrix` 缓存、frustum culling、八叉树、资源 ownership、opaque handle、引用计数纹理共享 —— 一开始就把账还掉，后面所有渲染特性都受益。

4. **物理与渲染按序推进**
   主线先走渲染（学习 RTR4 第 9 章为目标）。物理 Jolt 在光照、阴影主线之后接入，不阻塞光照学习。

5. **渲染按 RTR4 章节顺序展开**
   Forward Phong → Blinn-Phong → PBR (Cook-Torrance + GGX) → IBL (split-sum) → ShadowMap (Basic / PCF / VSM / CSM) → 后处理 → 全局光照实验 → GPU Driven。每一步做一遍 UE 等价物对照笔记。

6. **编辑器与序列化贯穿全程**
   每加一类资产/组件，同步落 `Object::save/load`、属性面板、Undo/Redo。UE 编辑器里很多"真正的难点"都在序列化与 Undo/Redo。

7. **工具链与 CI 必修**
   `clang-format` / `clang-tidy` / case-sensitive include 检查 / `-Wreorder -Werror` / SPIR-V 磁盘缓存 / `setup.py` 完善。

8. **学习笔记必须交付**
   每个里程碑结束写一篇 `doc/学习笔记/<主题>.md`，包括：
   - 实现踩坑
   - UE 对照（找出 UE 中对应模块路径与设计取舍）
   - 参考资料链接
   - 概念图 / 时序图（Mermaid 即可）

---

## 二、Phase 总览

| Phase | 主题 | 预估 | 备注 |
|-------|------|------|------|
| 0  | 代码健康度修复 + 基础设施扎根 | ~4 周 | **不加新功能** |
| 1  | 光照（Phong → PBR → IBL） | ~8 周 | RTR4 第 9 章 |
| 2  | 阴影（Basic → CSM） | ~6 周 | |
| 3  | Jolt 物理接入 | ~5 周 | |
| 4  | 天空与大气 | ~3 周 | |
| 5  | 地形系统 | ~6 周 | |
| 6  | 水体（含 FFT 海洋） | ~5 周 | |
| 7  | 八叉树 + 剔除 + 线程化 | ~3 周 | |
| 8  | 后处理与色彩管线 | ~3 周 | |
| 9  | Recast & Detour 寻路 | ~2 周 | |
| 10 | GPU Driven 实验 | 开放 | 学习性质 |

合计纸面排期：**~45 周（约一年）**。单人节奏宽松，允许溢出 30%。

---

## 三、Phase 0：代码健康度修复 + 基础设施扎根  (~4 周，不加新功能)

> **目标**：把代码审查发现的必崩级问题、跨平台编译陷阱、ownership 混乱清理干净，
> 同时把后续所有 phase 都依赖的基础设施一次扎深。

### M0.1  必崩级 Critical 修复（~1 周）

对照 `doc/review/代码审查-崩溃安全.md` 中标红的 13 项：

- [ ] `Object::skinRoot()` 自递归（栈溢出 → 必崩）
- [ ] `ShaderMacroArray::assign` 变量名拼错（写错成员 → 行为错误）
- [ ] `delete (void*)` —— 改为类型化 delete 或 deleter 回调
- [ ] `_loadfile` 返回 `false` 给 `char*` —— 修返回类型与调用点
- [ ] `svkCopyImageToData` dataSize 计算顺序 —— 修先 size 后 alloc
- [ ] `releaseObjectIDMap` 未置 NULL —— 二次访问悬空指针
- [ ] `~Object` 不解父引用 —— 父节点 children 列表悬空
- [ ] `cgltf_util.c` 链式解引用无判空 —— 任意一段缺失即崩
- [ ] terrain 硬编码 3 —— 明确含义，改成具名常量并加注释
- [ ] Vulkan 退出时未 `vkDeviceWaitIdle` —— 进程退出偶发崩
- [ ] 全局对象析构顺序未定（Env / Shader Cache / Render） —— 改为显式 shutdown
- [ ] union 误用（POD vs 非 POD） —— 改 `std::variant` 或显式构造
- [ ] `void*` 句柄类型不安全 —— 用 opaque handle struct（typed handle）

**交付**：
- 所有 critical 项以 PR 形式提交（即使单人也走 PR 形式，留 review 痕迹）
- `doc/学习笔记/崩溃安全修复总结.md`：每条问题 = 根因 + 修复方案 + 类似坑的 UE 防御写法

### M0.2  跨平台编译与构建清理（~3~5 天）

对照 `doc/review/代码审查-跨平台编译.md`：

- [ ] 所有 include 改为 case-sensitive（Linux/macOS 必崩根源）
- [ ] 修复 ctor 初始化列表顺序（`-Wreorder`）
- [ ] 启用 `-Wall -Wextra -Wreorder -Wshadow -Werror`（先逐模块开，避免一次爆 1000 个）
- [ ] `clang-format` 配置入仓，pre-commit hook 强制
- [ ] `clang-tidy` 至少开 `cppcoreguidelines / bugprone / performance / modernize` 子集
- [ ] `setup.py` 完善：一键拉依赖 + 配 cmake + 装 hook
- [ ] CI 草稿：本地 `act` 跑 `build_windows + build_linux`（GitHub Actions 形式）

**交付**：
- `.clang-format` / `.clang-tidy` / `.editorconfig` 入仓
- `scripts/setup.py` 或扩展现有 `setup.py`
- `doc/学习笔记/跨平台与工具链.md`
  - 大小写敏感的 include 治理过程
  - clang-tidy 误报与豁免策略
  - 比较 UE 的 `Build.cs` / `CompileEnvironment` 隔离设计

### M0.3  资源 ownership 与 opaque handle（~1 周）

对照 `doc/review/代码审查-系统级.md` 中 ownership 相关条目：

- [ ] 明确每种资源（Texture / Mesh / Shader / Material）的 ownership 模型：
  - unique（Mesh data）
  - shared + ref-count（Texture / Shader Module / Pipeline）
  - non-owning view（场景中的 reference）
- [ ] 引入 typed opaque handle（替换 `void*` GPU 句柄）：

```cpp
struct TextureHandle { uint32_t id; };
struct BufferHandle  { uint32_t id; };
struct ShaderHandle  { uint32_t id; };
```

`IRender` 接口的所有 `void*` 改为 typed handle。

- [ ] Texture 共享：相同 path/hash 命中既有资源，引用计数；release 到 0 才真销毁。
- [ ] Shader 模块缓存：用**结构化 key**（不再拼字符串），key 含 `{sourcePath, stage, macroSetHash, includeMtime}`。
- [ ] SPIR-V 磁盘缓存：以 key 哈希为文件名，落 `.cache/spv/`；命中即跳过 `shaderc`。

**交付**：
- `IRender` 接口与所有实现统一改造
- `doc/学习笔记/资源-ownership-与-handle.md`
  - UE 中 `FRHITexture` / `TRefCountPtr` 的设计对照
  - opaque handle vs 强类型指针的取舍
  - SPIR-V 磁盘缓存与 UE DDC 的对照（哲学相同）

### M0.4  Transform / Box / Visibility 基础设施（~1 周）

对照 `doc/review/代码审查-性能.md`：

- [ ] **Transform dirty flag**
  - `localMatrix` dirty
  - `globalMatrix` dirty（父链 dirty 时下传）
  - 替换现有"每次访问都乘一遍父链"的实现
- [ ] **BoundingBox CPU 缓存**
  - `Mesh::getBoundingBox()` 改为读取首次构建期算好的 CPU 版本
  - 严禁 readback GPU vertex buffer 做包围盒（CR 中明确点名）
- [ ] **视锥剔除**
  - 提取 6 个 frustum plane
  - 每个 `Primitive` 在 Render 之前做 AABB vs frustum 测试
  - 输出统计：`drawn / culled`，挂 ImGui 调试面板
- [ ] **Uniform buffer 治理**
  - 48MB 常驻 uniform buffer 调查并拆分（per-frame / per-object 分桶）
  - 引入 ring buffer 的 dynamic uniform 写入路径（pre-step，为后续 GPU Driven 铺路）

**交付**：
- ImGui 调试面板：`FrameStats`（draw count / culled / triangles / passes）
- `doc/学习笔记/Transform-与-视锥剔除.md`
  - dirty flag 传播算法（与 UE `FSceneProxy::UpdateTransform` 对照）
  - frustum 提取的两种方式（plane extraction vs Gribb–Hartmann）
  - UE 中 `FViewInfo::HiddenPrimitiveMap` 与 `OcclusionQuery` 的接力

### M0.5  死代码与组织清理（~3 天）

- [ ] 清掉散落的 `.txt` 残留代码片段（CR 中多处提到）
- [ ] 统一文件命名规范（驼峰 vs snake_case 二选一，写进 `doc/review/代码审查-规范.md`）
- [ ] `doc/` 下中文文件名乱码问题排查（Windows GBK vs UTF-8 兼容）
- [ ] 把 `doc/` 中"实验性"标记的内容整理为 `doc/sketches/` 子目录

### Phase 0 出口准入条件

- [ ] 13 项 critical 全部清掉
- [ ] 三平台（Windows / Linux / Android stub）能编过
- [ ] FrameStats 面板可用
- [ ] 视锥剔除生效（场景外物体不进 draw call）
- [ ] M0.1 / M0.2 / M0.3 / M0.4 四篇学习笔记齐

---

## 四、Phase 1：光照系统  (~8 周)

> 主目标：完整走一遍 **RTR4 第 9 章（Physically Based Shading）**。
> 对照模型：**Sponza + DamagedHelmet**（glTF Sample Model 标准用例）。

### M1.1  Phong 经典光照（~1 周）

- [ ] 重整光源数据结构：`DirectionalLight / PointLight / SpotLight`
- [ ] `LightingComponent`，挂在 `Object/World` 上，序列化
- [ ] Shader：`phong.vert` / `phong.frag`
  - ambient + diffuse `(N·L)` + specular `(R·V)^n`
  - 多光源用 for 循环（不要 `#define MAX_LIGHTS` 硬编码，用 SSBO 或 dynamic uniform）
- [ ] 法线变换矩阵：inverse-transpose（注意 dirty flag 触发）
- [ ] 编辑器：光源 Gizmo
  - DirectionalLight：一根箭头
  - PointLight：一个线框球
  - SpotLight：一个圆锥
- [ ] 调试可视化：法线 / 切线 / 光照分量分别可见

**学习笔记**：`doc/学习笔记/Phong-光照.md`
- Lambert 与 Phong 的物理意义（不是物理正确但概念清晰）
- normal matrix 为什么是 inverse-transpose
- gamma 与 linear 空间（提前布点，下个 milestone 真正切换）

### M1.2  Blinn-Phong + 线性空间渲染（~1 周）

- [ ] Half vector：`H = normalize(L + V)`，specular = `(N·H)^n`
- [ ] 全管线切到线性空间：
  - sRGB texture sampling 用 `VK_FORMAT_*_SRGB`
  - swapchain 用 sRGB format，由 GPU 做 OETF
  - 手工 `pow(2.2)` 全部干掉（如果有）
- [ ] HDR 中间缓冲：`renderTarget = R16G16B16A16_SFLOAT`
- [ ] 最小 Tonemap：Reinhard（先占位，Phase 8 详做）

**学习笔记**：`doc/学习笔记/Blinn-Phong-与-Gamma.md`
- Blinn-Phong 性能与外观优势（特别是 large angle 时的差异）
- sRGB 与 gamma 的历史与硬件支持
- HDR pipeline 的"必要性而不是炫技"

### M1.3  Cook-Torrance + GGX 基础 PBR（~2 周）

- [ ] BRDF 拆解：
  $$f = k_d \cdot \text{Lambert} + k_s \cdot \frac{D \cdot F \cdot G}{4 \cdot (N \cdot L) \cdot (N \cdot V)}$$
- [ ] **D**：GGX (Trowbridge-Reitz)
- [ ] **F**：Schlick approximation
- [ ] **G**：Smith 联合（GGX 对应的 Lambda）
- [ ] roughness / metallic / baseColor 工作流（glTF 默认的 MR）
- [ ] 让 DamagedHelmet 显示与 glTF Sample Viewer 一致（视觉比对）

**学习笔记**：`doc/学习笔记/PBR-Cook-Torrance-GGX.md`
- 各项分量推导：能量守恒 / Fresnel / Microfacet 几何遮蔽
- GGX vs Beckmann vs Blinn 各项异性
- glTF metallic-roughness 与 specular-glossiness 两套工作流
- UE 中 BasePass shader 的 BRDF 入口路径

### M1.4  IBL：split-sum 近似（~3 周）

- [ ] 输入：HDR equirectangular `.hdr` / `.exr`
- [ ] 转 Cubemap：irradiance + prefiltered specular + BRDF LUT
  - cubemap mip 链：`roughness ↔ mip level`
  - prefilter：importance sampling GGX
  - BRDF LUT：预计算 `NdotV / roughness` 的二维 LUT（R16G16）
- [ ] 落地：
  - Compute shader 离线生成（不用每次启动算）
  - 落盘 `.ktx2`
- [ ] Shader 中 IBL evaluation：

```glsl
vec3 diffuse_indirect  = kd * irradiance(N);
vec3 specular_indirect = ks * prefiltered(R, roughness)
                          * (F0 * brdfLUT.r + brdfLUT.g);
```

**学习笔记**：`doc/学习笔记/IBL-split-sum.md`
- split-sum 的两阶段近似数学推导
- Karis 在 SIGGRAPH 2013 的原始 talk 重点
- prefilter 时 importance sampling vs uniform sampling 的差异
- 与 UE `FReflectionEnvironmentCubemapArray` 的对照

### M1.5  材质系统整理（~1 周，穿插）

- [ ] `Material` 数据结构正式立项：
  - `baseColorTexture / metallicRoughnessTexture / normalTexture / occlusionTexture / emissiveTexture`
  - factor 与纹理乘法（glTF 标准）
- [ ] 材质参数 ImGui 编辑面板
- [ ] 序列化：材质引用纹理用 GUID/path 而非裸指针
- [ ] 材质资产 vs 材质实例（先不上 UE 那套 MIC，先打基础）

**学习笔记**：`doc/学习笔记/材质系统-v1.md`
- glTF 材质模型完整规范
- UE Material 与 MaterialInstance 的关系，本项目以后怎么收敛

### Phase 1 出口准入

- [ ] DamagedHelmet 与官方 viewer 视觉无明显偏差
- [ ] Sponza 在 IBL 下高光与漫反射都合理
- [ ] 五篇学习笔记齐

---

## 五、Phase 2：阴影系统  (~6 周)

> 主目标：把 ShadowMap 的整条链路走一遍，对照 UE `FProjectedShadowInfo`。

### M2.1  Basic Shadow Map（~1.5 周）

- [ ] DirectionalLight 的 light view + ortho proj 计算
- [ ] depth-only pass：写一张 depth attachment
- [ ] 主光照 shader 中采样阴影图，做 depth compare
- [ ] 解决经典伪影：
  - shadow acne：constant bias + slope-scaled bias
  - peter panning：bias 调小、front-face culling 时观察
- [ ] 调试：把 shadow map 渲染到屏幕角落

**学习笔记**：`doc/学习笔记/ShadowMap-基础.md`
- shadow acne / peter panning / shimmer 三大伪影根因
- 深度比较与 `sampler2DShadow` 的硬件 PCF
- light space 矩阵计算的几个坑

### M2.2  PCF 与 PCSS（~1 周）

- [ ] PCF：3x3 / 5x5 / Poisson disk 三档
- [ ] PCSS：blocker search → penumbra estimate → variable PCF
- [ ] 切换可在 ImGui 实时对比

**学习笔记**：`doc/学习笔记/PCF-PCSS.md`

### M2.3  VSM / ESM（~1.5 周）

- [ ] VSM：写入 depth + depth²，光照时用 Chebyshev 不等式
- [ ] light bleeding 的处理（reduction）
- [ ] ESM 备选，与 VSM 对比开销与质量

**学习笔记**：`doc/学习笔记/VSM-ESM.md`
- 概率统计视角：Chebyshev 推导
- 与 PCSS 的对比矩阵

### M2.4  CSM 级联阴影（~2 周）

- [ ] frustum 切分（log / uniform / 混合）
- [ ] 各 cascade 独立 view+proj
- [ ] cascade 选择 + 边界过渡
- [ ] cascade 抖动稳定（snap to texel）
- [ ] PointLight 改 cubemap shadow（顺手做掉）
- [ ] SpotLight perspective shadow

**学习笔记**：`doc/学习笔记/CSM.md`
- frustum 切分策略
- UE WholeSceneShadow 与 CSM 的入口
- 与 Virtual Shadow Map（VSM, UE5）的差距与未来路线

### Phase 2 出口准入

- [ ] Sponza 在 directional + 多 point light 下阴影一致
- [ ] CSM 摄像机移动稳定，无 shimmer
- [ ] 四篇学习笔记齐

---

## 六、Phase 3：Jolt 物理接入  (~5 周)

> 依据：`doc/physic/物理引擎接入设计与实施计划.md` / `doc/physic/Jolt接入实施步骤.md`

### M3.1  Jolt 编译进项目（~3 天）

- [ ] `free/jolt` 通过 CMake 子项目编入
- [ ] `catphysics` 模块脚手架
- [ ] 与 `scl` 数学的转换层（`vector3 ↔ Vec3`、`quaternion ↔ Quat`）

### M3.2  PhysicsWorld + 静态碰撞（~1 周）

- [ ] `PhysicsScene`：BroadPhaseLayer / ObjectLayer / ContactListener 配置
- [ ] `CollisionComponent`：Box / Sphere / Capsule / ConvexHull / MeshShape
- [ ] glTF mesh → Jolt MeshShape 转换
- [ ] 调试可视化：Jolt `DebugRenderer` 桥接到现有 line/triangle 渲染

### M3.3  射线拾取替换 GPU Pick（~3 天）

- [ ] 编辑器鼠标射线 → Jolt `CastRay`
- [ ] 与现有 Pick Pass 并存，提供"切换"按钮，方便对照
- [ ] 验证：选择速度、精度对比

**学习笔记**：`doc/学习笔记/Jolt-接入与射线拾取.md`
- Broad / Narrow phase 与 BVH 学习
- GJK / EPA 简介
- 与 PhysX / Bullet 的对比

### M3.4  刚体动力学（~1 周）

- [ ] `RigidBodyComponent`：mass / friction / restitution / linear/angular velocity
- [ ] 编辑器中拖拽生效（运行态 vs 编辑态隔离 —— 引入 PIE 概念草稿）
- [ ] 简单场景 demo：堆叠盒子 / 弹球 / 倒下的多米诺

### M3.5  碰撞回调与触发器（~1 周）

- [ ] Trigger volume（OverlapPair 回调）
- [ ] OnHit / OnOverlap / OnEndOverlap 三类回调路由到 Component
- [ ] 与脚本/C++ 行为系统的接口（先 C++ 虚函数，脚本暂不接）

### M3.6  约束与角色控制器（~1 周）

- [ ] Distance / Hinge / Slider 约束 demo
- [ ] `CharacterVirtual`：第一人称漫游 demo（与编辑器摄像机并存）

**学习笔记**：`doc/学习笔记/Jolt-刚体与角色.md`
- 角色控制器的两套范式（rigid body 驱动 vs kinematic + capsule cast）
- UE Chaos 与 Jolt 的对照

### Phase 3 出口准入

- [ ] 编辑器拾取走 Jolt，速度 / 精度可与 GPU Pick 对比
- [ ] 角色 demo 可在 Sponza 中走动
- [ ] 两篇学习笔记齐

---

## 七、Phase 4：天空与大气  (~3 周)

### M4.1  Cubemap SkyBox（~3 天）

- [ ] 立方体盒子，深度测试 `LessEqual`，最后绘制
- [ ] HDR cubemap，可与 IBL 共享同一张

### M4.2  程序化天空（~1.5 周）

- [ ] Preetham / Hosek-Wilkie 二选一
- [ ] 太阳位置实时控制
- [ ] 与 DirectionalLight 联动（太阳的方向、颜色驱动主光）

**学习笔记**：`doc/学习笔记/程序化天空.md`
- Rayleigh / Mie 散射基本概念
- UE SkyAtmosphere 的简化路径

### M4.3  体积云（草稿，~1 周）

- [ ] Worley + Perlin noise 3D 纹理生成
- [ ] Raymarching shader（成本控制：低分辨率半屏 + upscale）
- [ ] 标记为"草稿"，不强求质量，重在学习 raymarching

**学习笔记**：`doc/学习笔记/体积云草稿.md`

---

## 八、Phase 5：地形系统  (~6 周)

### M5.1  高度图地形（~1 周）

- [ ] `HeightmapTerrainComponent`
- [ ] tessellation 或者 mesh shader 暂不上，先 CPU 生成 vertex grid
- [ ] LOD：四叉树 quad-tree LOD（geomipmap 简化版）

### M5.2  地形多层混合（~1 周）

- [ ] splat map（4 通道控制 4 层）
- [ ] triplanar 备选（陡坡）

### M5.3  编辑器地形刷子（~1 周）

- [ ] 高度刷（升 / 降 / 平 / 平滑）
- [ ] 纹理刷（写 splat map）
- [ ] Undo/Redo（地形数据快照）

### M5.4  GPU Instance 草地（~1 周）

- [ ] 在地形上撒草：density map + 风扰动
- [ ] DrawInstance / DrawIndirect

### M5.5  雪/沙脚印（~1 周）

- [ ] RenderTarget 跟随角色，写入凹陷
- [ ] 衰减恢复

### M5.6  RVT（草稿，~1 周）

- [ ] Runtime Virtual Texture 概念实现
- [ ] 标记草稿，不追完整度

**学习笔记**：每个 milestone 单独一篇短笔记，最后总成一篇 `doc/学习笔记/地形系统.md`

---

## 九、Phase 6：水体  (~5 周)

| 子任务 | 内容 | 预估 |
|--------|------|------|
| M6.1 | 湖泊（静态平面 + 反射 + 折射 + Fresnel） | ~3 天 |
| M6.2 | 河流（flow map） | ~3 天 |
| M6.3 | 涟漪（鼠标点击在 RT 上扩散） | ~3 天 |
| M6.4 | FFT 海洋（compute IFFT + Phillips spectrum + 多频带叠加 + 与 SkyAtmosphere 联动） | ~2.5 周 |
| M6.5 | 瀑布（贴图滚动 + 粒子顶部 + 底部水雾） | ~1 周 |

**学习笔记**：
- `doc/学习笔记/水体反射折射.md`
- `doc/学习笔记/FFT-海洋.md`（重头戏，对照 Tessendorf 2001 原文）

---

## 十、Phase 7：场景管理  (~3 周)

### M7.1  八叉树空间索引（~1 周）

- [ ] 静态 build / 动态 update
- [ ] 视锥剔除替换 Phase 0 的线性版本（性能对比要做表）

### M7.2  遮挡剔除：HZB 草稿（~1 周）

- [ ] mip 链 reduce
- [ ] HZB occlusion test compute pass

### M7.3  渲染线程化草稿（~1 周）

- [ ] 主线程：scene update；render 线程：draw submit
- [ ] 双缓冲 frame data

**学习笔记**：`doc/学习笔记/场景管理与剔除.md`
- 八叉树 / BVH / grid 三选一的取舍
- UE `FScene / FViewInfo / OcclusionQueue` 的对照
- 多线程渲染的同步语义

---

## 十一、Phase 8：后处理与色彩管线  (~3 周)

- [ ] HDR pipeline 收口（之前在 M1.2 占位的部分正式做）
- [ ] Tonemap：Reinhard / ACES Filmic / AgX 三选可切换
- [ ] Bloom：dual filter blur 或 13-tap (Karis)
- [ ] FXAA / SMAA 至少一种
- [ ] TAA 草稿（reproject + history clamp，标记 v0）
- [ ] 颜色分级（LUT）

**学习笔记**：`doc/学习笔记/后处理与色彩管线.md`
- HDR / SDR / Display Referred / Scene Referred 概念
- ACES 与 AgX 的工程差异
- TAA 的鬼影来源与基本处理

---

## 十二、Phase 9：AI 寻路  (~2 周)

- [ ] Recast & Detour 接入（`free/recast` 拉源码）
- [ ] 从静态地形 + 静态碰撞体 build navmesh
- [ ] Detour pathfinding：起点 → 终点
- [ ] 编辑器可视化（绘制 navmesh polygon）
- [ ] 与 Jolt 角色控制器联动：右键设目标，角色寻路过去

**学习笔记**：`doc/学习笔记/Recast-Detour-寻路.md`

---

## 十三、Phase 10：GPU Driven 实验（开放期）

> 定位：**学习性质，不追完整度**。

- [ ] 间接绘制 `DrawIndirect` / `DrawIndexedIndirect`
- [ ] GPU culling compute pass（参考 Niagara / Nanite 的入门级实现）
- [ ] Mesh Cluster：Meshlet 分块（`meshoptimizer` 库辅助）
- [ ] Visibility Buffer 实验（先小场景）

**学习笔记**：`doc/学习笔记/GPU-Driven-入门.md`
- UE5 Nanite 体系总览（不实现，只学）
- 《A Deep Dive into Nanite Virtualized Geometry》笔记

---

## 十四、贯穿全程的横切任务

### 编辑器

- 每加一个 Component / Asset 类型，同步落属性面板 + Outliner 显示
- 每加一个可编辑数值，同步落 Undo/Redo
- 编辑器 Gizmo 持续打磨（M0 的左上角坐标轴改进、imguizmo 不丢焦点）

### 序列化

- 每加一类资产，立刻设计 save/load
- 用 `rapidyaml` 做主格式；二进制资产（贴图 / lightmap / navmesh）旁路落盘
- 资产引用统一用 path/GUID，**禁止裸指针入文件**

### 调试与可视化

- `FrameStats` 面板持续扩展（draw / culled / shadow / postfx / GC）
- GPU Profiling：接入 Tracy 或者用 `vkCmdWriteTimestamp` 自己采（M0 末期开始）
- ImGui 中所有"切换开关"集中到一个 Debug 面板

### 测试

- `testCat` 中按 phase 增量加 demo 场景：
  - Phase 1 → `sponza_pbr_demo`
  - Phase 2 → `sponza_csm_demo`
  - Phase 3 → `jolt_playground`
  - ……
- 关键算法（dirty flag 传播、八叉树 build、frustum 提取）写单测（用 catch2 或自写极简框架）

### 文档

- 每个 phase 结束更新 `doc/architect/架构总览.md`
- `doc/architect/子系统-XXX.md` 在引入新模块时同步更新
- 学习笔记入 `doc/学习笔记/` 子目录（如目录不存在则建）

---

## 十五、风险与对策

| # | 风险 | 对策 |
|---|------|------|
| 1 | Phase 0 拖到 6~8 周（修问题修出新问题） | 每个 critical 修复必须有"最小复现 demo + 修复后回归"。第 4 周末若仍未清完，剩余 critical 推到 Phase 1 穿插，但 frustum culling / dirty flag / opaque handle 必须 4 周内完成。 |
| 2 | PBR + IBL 学不动，被数学卡住 | M1.3 / M1.4 允许溢出到 4 周。先看 LearnOpenGL PBR 章节 + Joey de Vries 的 GitHub 实现，再回头啃 RTR4。绝不跳过笔记，写不出笔记说明没学懂。 |
| 3 | Jolt 接入与渲染 ownership 改造冲突 | M3.1 之前确认 M0.3 的 ownership 改造已完成。Jolt 的物理体 ↔ 渲染体的双向引用，用 ComponentID 而非指针。 |
| 4 | 单人开发动力衰减 | 严格保留学习笔记交付；每个 phase 结尾自己 demo 一遍（录屏或写 demo doc）形成正反馈；允许中间穿插一个"业余小特性周"。 |
| 5 | 渲染路径越改越复杂，`IRender` 抽象层跟不上 | `IRender` 在 M0.3 同步做一次大改造；Phase 2 末期再做一次"中期审查"，若需要则插入一个 Phase 1.5 的中期里程碑（参考 UE 的 RHI vs RDG 分层）。 |

---

## 十六、出口判定

> "task2 完成"的判定 = 下列条件全部 ✅

- [ ] Phase 0 ~ Phase 9 全部 milestone 勾完
- [ ] `doc/学习笔记/` 下不少于 **25 篇**笔记
- [ ] `testCat` 中至少 **6 个 demo 场景**能跑（PBR / CSM / Jolt / Terrain / Water / Nav）
- [ ] 引擎在 **Windows + Linux** 上都能编过且 demo 跑通
- [ ] 自己能用**一句话**讲清每个子系统的设计取舍和"为什么这么做"

---

> 任务卡片可在执行中追加到本文件末尾，保留时间戳。
> 每完成一个 milestone，在该 milestone 行前打勾 `- [x]`，并写一条简短回顾。
