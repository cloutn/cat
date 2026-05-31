# cat 引擎路线图：对照 UE 的模块清单与优先级排序

> 定稿时间：2026-05-31
> 视角：以 UE4/UE5 模块全景为参照，重新评估 cat 引擎"先做什么、后做什么"
> 关系：本文件**不替代** `task2.md`，只提供另一种取景框
> 4 个评分维度：
> - **M（Modern）** 现代引擎设计的必备性
> - **L（Learn）** 作为学习材料的价值
> - **A（Android）** 移动平台适用性（你日常 UE 项目对口的设备：Snapdragon 660 ~ 8 Gen 3）
> - **D（Demo）** 做独立游戏 demo 的必要性（MMO / ARPG / 文明 / RTS / 戴森球 / FPS）
>
> 评分 0~5；★=必做、☆=可选、✗=不做
>
> 评分原则：
> - 单人开发，资源 = 时间
> - cat 已有：scl、glTF、ImGui、ImGuizmo、Vulkan、Jolt 接入计划、Recast 接入计划
> - 写在前面：UE 一定不是一个人能写出来的，本文档只对"概念模块"对齐，不奢求 cat 在每个模块上达到 UE 的完成度

---

## 目录

- [一、UE 模块全景清单](#一ue-模块全景清单)
- [二、4 维度打分总表](#二4-维度打分总表)
- [三、按"必做 / 应做 / 可选 / 不做"分桶](#三按必做--应做--可选--不做分桶)
- [四、cat 推荐开发路线图（与 task2 对照）](#四cat-推荐开发路线图与-task2-对照)
- [五、独立游戏品类驱动的能力反推表](#五独立游戏品类驱动的能力反推表)
- [六、与 task2.md 的关键差异说明](#六与-task2md-的关键差异说明)
- [七、单人开发的"非技术"约束](#七单人开发的非技术约束)

---

## 一、UE 模块全景清单

> 目标：穷尽地列出 UE 主要模块，**先看见全貌**，再做减法。
> 来源：`Engine/Source/Runtime/*` + `Engine/Source/Editor/*` + `Engine/Plugins/*`。

### 1. 基础设施（Foundation / Core）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| F-01 | `Core` | 容器、字符串、数学、时间、文件 IO |
| F-02 | `CoreUObject` | `UObject`、反射、GC、序列化、`UClass` |
| F-03 | `ApplicationCore` | 平台抽象（Win/Mac/Linux/Android/iOS） |
| F-04 | `Json` / `Xml` | 配置 / 数据交换 |
| F-05 | `Slate` / `SlateCore` | 编辑器与游戏 UI 框架 |
| F-06 | `UMG` | 设计师友好的运行时 UI |
| F-07 | `MessageLog` / `Logging` | 分类日志、宏 `UE_LOG` |
| F-08 | `Stats` / `Insights` / `Trace` | 性能采集 + 离线分析 |
| F-09 | `TaskGraph` / `Async` | 任务系统、`ParallelFor` |
| F-10 | `DerivedDataCache (DDC)` | 编译产物全局缓存（shader / lightmap / mesh） |
| F-11 | `IO Store` / `Pak` | 资产打包与运行时加载 |
| F-12 | `AssetRegistry` | 资产元数据索引 |
| F-13 | `Localization` / `FText` | 本地化、字符串表 |
| F-14 | `Cmd` / `IConsoleVariable` (CVar) | 命令行 / 控制台开关 |
| F-15 | `UnrealBuildTool / UnrealHeaderTool` | 元构建 + 反射代码生成 |

### 2. 渲染（Rendering）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| R-01 | `RHI` | 抽象 Vulkan/DX12/Metal/GLES |
| R-02 | `RenderCore` | 着色器编译、参数绑定、`FRenderResource` |
| R-03 | `Renderer` | 实际渲染管线（Deferred / Mobile） |
| R-04 | `MeshDrawPipeline` (`FMeshPassProcessor`) | MeshBatch → MeshDrawCommand |
| R-05 | `GPU Scene` | Per-instance 数据集中到 GPU buffer |
| R-06 | `RDG`（RenderDependencyGraph） | 帧级有向图，自动管理资源生命周期 |
| R-07 | `Materials` + `MaterialEditor` | 节点图 → HLSL → SPIR-V |
| R-08 | `Material Instance` (MIC/MID) | 参数覆盖、避免 shader 爆炸 |
| R-09 | `Lighting` | DirLight / PointLight / SpotLight / RectLight / SkyLight |
| R-10 | `ShadowMap` | Basic / PCF / VSM / CSM / DistanceField / VSM(UE5) |
| R-11 | `IBL` / `Reflection Captures` | Sphere / Box / Planar / SkyLight |
| R-12 | `SSAO` / `GTAO` | 屏幕空间 AO |
| R-13 | `SSR` / `Lumen` | 屏幕空间反射 / 软光线追踪 GI |
| R-14 | `Nanite` | Cluster + LOD + Visibility Buffer |
| R-15 | `Virtual Shadow Map (VSM)` | UE5 高分辨率阴影 |
| R-16 | `Volumetric Fog` / `Cloud` / `SkyAtmosphere` | 大气与体积 |
| R-17 | `Hair / Groom` | Strand 渲染 |
| R-18 | `Niagara` / `Cascade` | GPU 粒子 / VFX |
| R-19 | `Landscape` | 地形 + LOD + GrassType |
| R-20 | `Foliage` | 草、灌木、树的撒点 + Instance |
| R-21 | `Water` | 水体 + SingleLayerWater 材质 |
| R-22 | `Decals` | 投影贴花 |
| R-23 | `Post Process Chain` | Bloom / Tonemap / TAA/TSR / DoF / MB / FXAA / SMAA / Vignette / LUT |
| R-24 | `TAA` / `TSR` / `DLSS` / `FSR` / `XeSS` | 时域抗锯齿 + 上采样 |
| R-25 | `HZB` + `Occlusion Query` | 遮挡剔除 |
| R-26 | `Distance Field` | DFAO / DFShadow / Mesh Distance Field |
| R-27 | `Virtual Texture` (RVT / SVT) | 大世界纹理 |
| R-28 | `Texture Streaming` | mip 按需调入 |
| R-29 | `Skeletal Mesh / Static Mesh` | 资产类型 |
| R-30 | `LOD / HLOD` | 距离 / 屏占比切换 |
| R-31 | `Mobile Renderer` | 单 pass forward、subpass、tile shading |
| R-32 | `Path Tracer` / `Ray Tracing` | 离线参考 / 实时硬件 RT |
| R-33 | `Substrate` (UE5.4+) | 新材质框架 |
| R-34 | `Movie Render Queue` | 高质量离线渲染 |
| R-35 | `Pixel Streaming` | 云渲染推流 |

### 3. 动画（Animation）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| AN-01 | `Skeleton / SkeletalMesh` | 骨架 + 蒙皮 mesh |
| AN-02 | `AnimSequence` | 关键帧动画 |
| AN-03 | `AnimBlueprint` / `AnimGraph` | 节点图驱动姿态 |
| AN-04 | `BlendSpace` | N 维插值动画 |
| AN-05 | `StateMachine` | Idle / Walk / Run / Jump 状态切换 |
| AN-06 | `Animation Montage` | 一次性动作（攻击 / 受击） |
| AN-07 | `Control Rig` | 程序化绑定（IK / FK） |
| AN-08 | `IK` (TwoBone / FullBody / FABRIK / CCD) | 反向动力学 |
| AN-09 | `MorphTarget` (BlendShape) | 表情 / 形状变形 |
| AN-10 | `Animation Compression` | ACL / Bitwise / Curve |
| AN-11 | `Animation Notify` | 帧事件（脚步声、出手判定） |
| AN-12 | `Sequencer` | 关卡序列、Cinematic |
| AN-13 | `Live Link` | DCC 实时联动 |
| AN-14 | `Chaos Cloth` | 布料 |
| AN-15 | `Skinning` (CPU / GPU) | 蒙皮计算 |
| AN-16 | `RootMotion` | 动画驱动位移 |

### 4. 物理（Physics）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| P-01 | `Chaos Physics` (前身 PhysX) | 刚体 + 约束 |
| P-02 | `Collision Query` | RayCast / SweepTest / OverlapTest |
| P-03 | `CharacterMovementComponent` | 角色控制器（network-aware） |
| P-04 | `Chaos Destruction` | 几何破碎 |
| P-05 | `Chaos Vehicle` | 载具 |
| P-06 | `Chaos Cloth / Flesh` | 软体 |
| P-07 | `Trigger / Overlap` | 触发体回调 |
| P-08 | `Async Physics` | 物理与渲染异步 |

### 5. 音频（Audio）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| A-01 | `AudioMixer` | 新一代音频引擎 |
| A-02 | `MetaSound` | 节点图程序化音频 |
| A-03 | `SoundCue` / `SoundClass` / `SoundMix` | 资产 + 路由 |
| A-04 | `Audio Spatialization` | 3D 音效 / HRTF |
| A-05 | `Reverb` / `Submix Effects` | 后处理 |
| A-06 | `MediaFramework` | 视频 / 流媒体播放 |

### 6. 输入（Input）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| I-01 | `Enhanced Input` | Mapping Context + Action + Modifier |
| I-02 | `Gamepad / KBM / Touch` | 各类输入设备 |
| I-03 | `Input Replay` | 录制回放 |

### 7. 网络（Networking）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| N-01 | `Replication` (Property / RPC) | Actor 状态同步 |
| N-02 | `Iris` (UE5.3+) | 新一代复制框架 |
| N-03 | `Network Prediction` | 客户端预测 + 服务器仲裁 |
| N-04 | `Dedicated Server` | 无头服务端 |
| N-05 | `Online Subsystem` (Steam / EOS / GameCenter) | 平台账号 / 大厅 |
| N-06 | `Voice Chat` | 语音通道 |

### 8. AI / 行为（AI）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| AI-01 | `NavigationSystem` (Recast & Detour) | NavMesh + 寻路 |
| AI-02 | `Behavior Tree` + `Blackboard` | 节点行为决策 |
| AI-03 | `EQS`（Environment Query System） | 场景查询打分 |
| AI-04 | `StateTree` (UE5) | 状态树 |
| AI-05 | `AI Perception` | 视/听感知 |
| AI-06 | `Mass Entity / Mass AI` (UE5) | ECS 风格群体 |
| AI-07 | `Crowd Manager` | 人群避让 |
| AI-08 | `Smart Object` | 互动点 |

### 9. Gameplay 框架

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| G-01 | `UWorld / ULevel / AActor / UActorComponent` | 世界与对象模型 |
| G-02 | `GameInstance / GameMode / GameState / PlayerController / PlayerState / HUD` | 七件套 |
| G-03 | `GameplayAbilitySystem (GAS)` | 技能 + 效果 + Tag |
| G-04 | `GameplayTag` | 层级标签 |
| G-05 | `GameplayCue` | 表现层联动 |
| G-06 | `DataAsset / DataTable / DataRegistry` | 配置型资产 |
| G-07 | `Blueprint / VM` | 可视化脚本 + 字节码 VM |
| G-08 | `Subsystem` (Engine / GameInstance / World / LocalPlayer) | 模块化单例 |
| G-09 | `Save Game` | 存档序列化 |

### 10. 世界 / 场景（World）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| W-01 | `LevelStreaming` | 关卡按需加载 |
| W-02 | `World Composition` (legacy) | 老大世界拼接 |
| W-03 | `World Partition` (UE5) | 网格 + Streaming Source + Data Layer |
| W-04 | `HLOD` | 远景代理 mesh |
| W-05 | `OneFilePerActor (OFPA)` | git friendly 资产拆分 |
| W-06 | `External Actors / External Objects` | 同上 |

### 11. 编辑器（Editor）

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| E-01 | `Editor Framework` (Slate 化的) | 主框架 |
| E-02 | `Content Browser` + `AssetRegistry` | 资产浏览 |
| E-03 | `Details Panel` + `IPropertyTypeCustomization` | 反射驱动属性面板 |
| E-04 | `World Outliner` | 场景树 |
| E-05 | `Viewport` + `Gizmo` | 3D 视口 + 操控 |
| E-06 | `Blueprint Editor` | 可视化脚本 IDE |
| E-07 | `Material Editor` | 节点图 → shader |
| E-08 | `Niagara Editor` | 粒子 IDE |
| E-09 | `Animation / AnimBP Editor` | 动画工具 |
| E-10 | `Sequencer Editor` | 序列编辑 |
| E-11 | `Sound Editor` (MetaSound) | 音频节点图 |
| E-12 | `Landscape Editor` | 地形刷子 |
| E-13 | `Foliage Editor` | 撒点工具 |
| E-14 | `Source Control` (Perforce / Git / Plastic) | 版本控制集成 |
| E-15 | `Cook + Stage + Package` | 打包流水线 |
| E-16 | `PIE`（Play In Editor）/ `SIE` | 编辑器内试玩 |
| E-17 | `Plugin System` | 模块化扩展 |
| E-18 | `Asset Import` (FBX / OBJ / glTF / PNG / WAV / …) | 资产导入 |

### 12. 工具链与跨平台

| # | UE 模块 / 概念 | 一句话定位 |
|---|----------------|------------|
| T-01 | `UBT` / `UHT` | 元构建 + 反射 codegen |
| T-02 | `AutomationTool` | 打包 / 测试 自动化 |
| T-03 | `Cross-Compile to Android / iOS` | NDK / Xcode 集成 |
| T-04 | `Crash Reporter` | 崩溃上报 |
| T-05 | `Profiler` (Insights / Stat / GPU Profile) | 性能分析 |
| T-06 | `MemReport` / `LLM` (Low Level Mem Tracker) | 内存追踪 |

---

## 二、4 维度打分总表

> 说明：维度满分 5，**总分仅作参考**，不是简单相加（学习价值高但很难做的，单独标注）。
> 同一类别内按"推荐优先级"从高到低排序。

### 基础设施 / 反射 / 资产

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| F-02 | 反射 + GC + 序列化（UObject 等价） | 5 | 5 | 4 | 5 | cat 现有 `Object::save/load`，但缺反射，是**所有后续编辑器与序列化的核心地基** |
| F-09 | 任务系统 / 线程池 | 5 | 5 | 4 | 4 | 现在还是单线程，等 RDG 之前必须先做 |
| F-07 | 分类日志（已有 `scl::log`） | 4 | 3 | 5 | 4 | 已有，做加分项：category + level + 文件 ring buffer |
| F-14 | CVar / 控制台 | 4 | 4 | 5 | 4 | ImGui 已能凑合，应做成正式 CVar 注册表 |
| F-08 | Trace / Stats（已有 ImGui FrameStats 雏形） | 4 | 5 | 5 | 4 | 移动端 GPU profile 至关重要 |
| F-10 | DDC（shader / mesh / lightmap） | 4 | 5 | 4 | 3 | 学 UE DDC 的设计很值，先做 SPIR-V 磁盘缓存即可 |
| F-12 | AssetRegistry（资产元数据索引） | 4 | 4 | 4 | 4 | 先做轻量：`*.cat_asset` 旁路索引文件 |
| F-11 | Pak / IO Store | 3 | 3 | 4 | 3 | demo 阶段直接读散文件即可，shipping 再说 |
| F-13 | 本地化 | 2 | 2 | 3 | 2 | demo 阶段一律英文 / 中文硬编码即可 |
| F-15 | UHT 等价（codegen 反射） | 3 | 5 | 2 | 2 | **学习价值满分**，但单人维护代价大；折中：写个简易 `cat-reflect` 头扫描器 |
| F-05/06 | Slate / UMG | 2 | 3 | 3 | 4 | cat 用 ImGui 已绕开，"游戏 UI"另立体系（轻量 retained UI） |

### 渲染

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| R-01 | RHI 抽象（cat::IRender） | 5 | 5 | 5 | 5 | 已有雏形，**M0.3 中要做的 typed handle 是关键升级** |
| R-09/10/11 | DirLight + Shadow + IBL | 5 | 5 | 5 | 5 | task2 已规划，方向正确 |
| R-07/08 | 材质 + 材质实例 | 5 | 5 | 5 | 5 | **缺**：cat 目前 material 体系还很弱，必须早做 |
| R-04/05 | MeshDrawPipeline + GPU Scene | 5 | 5 | 5 | 4 | 把 per-object UB 收敛到 GPU buffer，移动端尤其受益 |
| R-23 | Post Process chain | 5 | 4 | 4 | 5 | Bloom / Tonemap / FXAA 是 demo 的"门面"，必做 |
| R-31 | Mobile Renderer（Forward / Subpass / Tile） | 5 | 5 | 5 | 4 | 移动平台核心，**应升级到 task2 主线** |
| R-06 | RDG（帧图） | 4 | 5 | 4 | 3 | 学习价值满分，但**应等 pass 数量 ≥ 10 再上**，否则过度工程 |
| R-29/30 | Static / Skeletal Mesh + LOD | 5 | 4 | 5 | 5 | cat 已支持 glTF，差 LOD |
| R-19/20 | Landscape + Foliage | 4 | 4 | 4 | 5 | 戴森球 / 文明 / RTS 没地形/植被不像样 |
| R-22 | Decals | 4 | 3 | 3 | 4 | ARPG / FPS 弹孔脚印必备，体量小，性价比高 |
| R-21 | Water | 3 | 4 | 3 | 3 | task2 给了 5 周，对**学习**值，对**多数 demo** 不必须，建议**压缩** |
| R-18 | Particles / VFX（Niagara 等价） | 5 | 4 | 4 | 5 | **task2 几乎没排**，是缺口 |
| R-16 | Volumetric Fog / Sky | 3 | 4 | 2 | 3 | 移动端慎用 |
| R-25 | HZB / Occlusion | 4 | 4 | 3 | 4 | 大场景必备，可与 GPU Driven 一起做 |
| R-28 | Texture Streaming | 4 | 3 | 5 | 3 | 移动端内存压力大时再做 |
| R-26 | Distance Field | 3 | 4 | 1 | 2 | 移动不友好，**跳过** |
| R-12 | SSAO / GTAO | 3 | 4 | 3 | 3 | 入门 SS 后处理算法，可在 post 章节顺手做 |
| R-13 | SSR / Lumen | 3 | 5 | 1 | 2 | Lumen 移动不友好，SSR 可做学习 |
| R-14 | Nanite | 2 | 5 | 0 | 1 | 学概念即可，**不实现** |
| R-15 | Virtual Shadow Map | 2 | 4 | 0 | 1 | 同上 |
| R-17 | Hair / Groom | 1 | 3 | 0 | 2 | 不做 |
| R-24 | TAA / TSR / DLSS | 3 | 5 | 3 | 3 | TAA v0 必做，DLSS/TSR 跳过 |
| R-32 | Ray Tracing | 1 | 4 | 0 | 1 | 不做 |
| R-33 | Substrate | 1 | 3 | 0 | 1 | 不做 |
| R-34 | Movie Render Queue | 1 | 2 | 0 | 1 | 不做 |
| R-35 | Pixel Streaming | 0 | 1 | 0 | 0 | 不做 |
| R-27 | Virtual Texture | 2 | 4 | 2 | 2 | 草稿即可，task2 已标 |

### 动画

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| AN-01/02/15 | Skeleton + Skinning + Sequence | 5 | 5 | 5 | 5 | cat 已能加载 glTF，差**运行时 AnimInstance** |
| AN-05 | StateMachine（最小可用版） | 5 | 4 | 5 | 5 | ARPG/MMO/FPS 必备 |
| AN-04 | BlendSpace | 4 | 4 | 5 | 5 | 双轴移动必备 |
| AN-06 | Montage | 4 | 4 | 4 | 5 | 出招、技能必备 |
| AN-11 | Animation Notify | 4 | 3 | 4 | 5 | 脚步 / 受击 / 出手判定 |
| AN-16 | RootMotion | 4 | 4 | 4 | 4 | 第三人称必备 |
| AN-08 | IK（TwoBone / FABRIK） | 4 | 5 | 3 | 4 | 脚 IK / 手 IK，FPS 拿枪 |
| AN-03 | AnimGraph（节点图） | 3 | 5 | 3 | 3 | 工具化代价大，**先用代码 hand-write**，编辑器后置 |
| AN-09 | MorphTarget | 3 | 3 | 3 | 3 | 表情；非必需 |
| AN-07 | Control Rig | 2 | 5 | 2 | 2 | 学习价值高，工程量大，**跳过** |
| AN-10 | Animation Compression | 3 | 4 | 5 | 2 | 移动端有用，但 glTF 已带 |
| AN-12 | Sequencer | 2 | 3 | 2 | 3 | cinematic 用得到，先不做 |
| AN-13 | Live Link | 1 | 2 | 0 | 1 | 不做 |
| AN-14 | Cloth | 2 | 4 | 1 | 2 | 移动不友好，跳过 |

### 物理

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| P-01/02 | 刚体 + Query | 5 | 5 | 5 | 5 | task2 已规划接 Jolt，方向对 |
| P-03 | CharacterController | 5 | 5 | 5 | 5 | Jolt CharacterVirtual，task2 已排 |
| P-07 | Trigger / Overlap | 5 | 4 | 5 | 5 | 必做 |
| P-04 | Destruction | 2 | 4 | 1 | 2 | 跳过 |
| P-05 | Vehicle | 2 | 3 | 2 | 2 | 看 demo 类型；跳过 |
| P-06 | Soft Body / Cloth | 2 | 4 | 1 | 2 | 跳过 |
| P-08 | Async Physics | 3 | 4 | 3 | 2 | 多线程化后再考虑 |

### 音频

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| A-01 | 基础混音 + 通道 | 5 | 4 | 5 | 5 | **task2 完全没排**，是大缺口；至少接一个 miniaudio / SoLoud |
| A-04 | 3D Spatial | 4 | 4 | 5 | 5 | 距离衰减 + 简单 panning 即可 |
| A-03 | SoundCue / Mix | 3 | 3 | 4 | 4 | 表驱动即可，先不上节点图 |
| A-05 | Reverb / Submix | 3 | 4 | 3 | 3 | 学一遍 freeverb |
| A-02 | MetaSound | 1 | 4 | 1 | 1 | 不做 |
| A-06 | MediaFramework | 2 | 2 | 2 | 2 | 看需要再说 |

### 输入

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| I-01 | Enhanced Input（Mapping Context） | 4 | 4 | 5 | 5 | **必做**，cat 当前是裸 Windows 消息；做一个 Action/Axis 抽象层 |
| I-02 | Gamepad / Touch | 4 | 3 | 5 | 5 | 移动端必备 |
| I-03 | Replay | 2 | 4 | 2 | 2 | 跳过 |

### 网络

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| N-01/03 | Replication + Prediction | 4 | 5 | 3 | **MMO=5,其他=2** | **极难自研**；建议**单独成一个长期 phase**，先不混入主线 |
| N-04 | Dedicated Server | 3 | 4 | 3 | MMO=5 | 同上 |
| N-05 | Online Subsystem | 2 | 3 | 4 | MMO=4 | 平台账号，shipping 才需要 |
| N-02 | Iris | 2 | 5 | 2 | 1 | 学概念即可 |

### AI

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| AI-01 | NavMesh + Detour | 5 | 5 | 5 | 5 | task2 已排 Recast，方向正确 |
| AI-02 | Behavior Tree + Blackboard | 4 | 5 | 4 | 5 | ARPG/RTS/MMO 都用，必做 |
| AI-03 | EQS | 3 | 5 | 3 | 4 | 学一次很值，可后期做 |
| AI-04 | StateTree | 3 | 4 | 3 | 3 | BT 之后再说 |
| AI-05 | Perception | 4 | 4 | 4 | 5 | 视/听感知，敌人必备 |
| AI-06 | Mass / ECS | 4 | 5 | 4 | 4 | 戴森球 / RTS 单位海量时必备；cat 已有 ECS 探索 doc |
| AI-07 | Crowd | 3 | 4 | 3 | 3 | RVO，看 demo |
| AI-08 | Smart Object | 2 | 3 | 2 | 2 | 跳过 |

### Gameplay 框架

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| G-01 | World / Actor / Component | 5 | 5 | 5 | 5 | cat 已有 Object，需要**正名**为正式 Actor+Component 体系 |
| G-02 | Game 七件套 | 4 | 4 | 5 | 5 | 至少要 GameMode + PlayerController + Pawn 三件 |
| G-08 | Subsystem | 4 | 4 | 4 | 4 | 替代散乱单例，是设计成熟度的标志 |
| G-09 | SaveGame | 4 | 3 | 5 | 5 | 文明/戴森球/MMO 必备 |
| G-04 | GameplayTag | 4 | 4 | 4 | 5 | 层级 tag 是写干净逻辑的银弹 |
| G-03 | GAS（Ability System） | 4 | 5 | 3 | ARPG=5 | 单独成 phase，体量大 |
| G-06 | DataAsset / DataTable | 4 | 4 | 5 | 5 | 配表系统，必做 |
| G-07 | Blueprint / VM | 2 | 5 | 3 | 3 | **学习价值满分但工程量大**；折中：接 Lua 或 AngelScript |

### 世界

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| W-01 | LevelStreaming | 4 | 5 | 5 | 5 | MMO/ARPG/戴森球必备 |
| W-03 | World Partition | 4 | 5 | 4 | 4 | 大世界，学概念即可，简化实现 |
| W-04 | HLOD | 3 | 4 | 3 | 3 | 看场景规模 |
| W-05/06 | OFPA | 2 | 3 | 3 | 2 | 单人不必 |

### 编辑器

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| E-04 | Outliner | 5 | 3 | - | 5 | 已有 |
| E-03 | Details Panel | 5 | 4 | - | 5 | 已有但依赖反射；F-02 不到位则一直是硬编码 |
| E-02 | Content Browser | 4 | 4 | - | 5 | 缺，需要做 |
| E-05 | Viewport + Gizmo | 5 | 3 | - | 5 | 已有 |
| E-12 | Landscape Editor | 4 | 4 | - | 4 | task2 已排（Phase 5） |
| E-13 | Foliage Editor | 4 | 3 | - | 4 | task2 已排 |
| E-07 | Material Editor（节点） | 3 | 5 | - | 3 | 工程量极大，**长期跳过**；用 yaml 文本材质即可 |
| E-06 | Blueprint Editor | 1 | 5 | - | 2 | 不做 |
| E-08 | Niagara Editor | 2 | 4 | - | 3 | 不做 |
| E-09 | AnimBP Editor | 2 | 4 | - | 3 | 不做 |
| E-10 | Sequencer | 2 | 3 | - | 3 | 不做 |
| E-11 | Sound Editor | 1 | 3 | - | 2 | 不做 |
| E-14 | Source Control | 3 | 2 | - | 2 | 用 git 即可 |
| E-15 | Cook / Package | 3 | 3 | 5 | 4 | shipping 才做 |
| E-16 | PIE | 4 | 4 | 3 | 4 | 编辑器与运行态隔离，**中期必做** |
| E-17 | Plugin System | 3 | 4 | 3 | 3 | 长期 |
| E-18 | Asset Import | 4 | 3 | 4 | 5 | 已有 glTF，加 PNG/HDR/WAV |

### 工具链

| ID | 模块 | M | L | A | D | 备注 |
|----|------|---|---|---|---|------|
| T-04 | Crash Reporter | 4 | 4 | 5 | 3 | shipping 才必须，开发期可做 minidump |
| T-05 | GPU Profile（Tracy / vkCmdWriteTimestamp） | 5 | 5 | 5 | 4 | **task2 在 M0 末就排了，正确** |
| T-06 | 内存追踪（LLM） | 4 | 5 | 5 | 3 | 移动端必修 |
| T-01 | UBT/UHT 等价 | 2 | 5 | 2 | 2 | 学习价值满分，工程不值 |
| T-03 | Android 构建 | 3 | 4 | 5 | 3 | task2 把跨平台暂缓，**长远看必须回来**（你日常 UE 项目是 Android） |

---

## 三、按"必做 / 应做 / 可选 / 不做"分桶

### A. 必做（不做就不像现代引擎，或不做就没法出 demo）

**渲染**
- R-01 RHI / typed handle
- R-04/05 MeshDrawPipeline + GPU Scene（移动端必备）
- R-07/08 Material + MIC
- R-09/10/11 Light + Shadow + IBL
- R-23 Post FX 基本链（Tonemap / Bloom / FXAA）
- R-29/30 Mesh + LOD
- R-31 Mobile Renderer 路径（forward + subpass）
- R-18 Particles / VFX（最小可用）

**动画**
- AN-01/02/15 Skeleton + Skinning + Sequence 运行时
- AN-04/05/06 BlendSpace + StateMachine + Montage
- AN-11/16 Notify + RootMotion

**物理**
- P-01/02/03/07 Jolt 刚体 + Query + Character + Trigger

**音频** ← **task2 没排，必须补**
- A-01/04 基础混音 + 3D 衰减

**输入** ← **task2 没排，必须补**
- I-01/02 Action/Axis 抽象 + Gamepad/Touch

**AI**
- AI-01 NavMesh
- AI-02 Behavior Tree + Blackboard
- AI-05 Perception

**Gameplay**
- G-01 Actor / Component 正名
- G-02 GameMode + PlayerController + Pawn
- G-06 DataAsset / DataTable
- G-08 Subsystem
- G-09 SaveGame

**世界**
- W-01 LevelStreaming（即使简化实现）

**基础设施**
- F-02 反射 + 序列化（哪怕弱反射）
- F-09 任务系统
- F-08 Trace / FrameStats
- F-14 CVar
- F-10 DDC（SPIR-V 磁盘缓存起步）

**工具**
- T-05 GPU Profile
- T-06 内存追踪

---

### B. 应做（学习价值高，做了一档明显进步）

- R-06 RDG（>10 pass 之后）
- R-19/20 Landscape + Foliage
- R-22 Decals
- R-12 SSAO/GTAO
- R-24 TAA v0
- R-25 HZB Occlusion
- AN-08 IK（TwoBone + FABRIK）
- AI-03 EQS
- AI-06 Mass / ECS
- G-03 GAS（仅当做 ARPG/MMO demo 时）
- G-04 GameplayTag
- W-03 World Partition（简化）
- E-02 Content Browser
- E-16 PIE 隔离
- T-03 Android 构建恢复（与日常 UE 工作对齐）

---

### C. 可选（learning-only，或特定 demo 才做）

- R-13 SSR
- R-16 Volumetric Fog / Cloud
- R-21 Water 完整实现（task2 给了 5 周，**建议压到 1.5 周**做湖泊 + 反射，FFT 海洋单独标 learning）
- R-27 Virtual Texture（草稿）
- R-28 Texture Streaming
- AN-03 AnimGraph（编辑器化）
- AN-09 MorphTarget
- A-03/05 SoundCue + Reverb
- G-07 脚本 VM（Lua / AS）
- W-04 HLOD
- F-12 AssetRegistry
- F-11 Pak / IO Store
- T-04 Crash Reporter
- 网络（N-*）整体作为**独立长期 phase**

---

### D. 不做（投入产出不值 / 与移动平台冲突 / 与单人开发量级冲突）

- R-14 Nanite（只学，不实现）
- R-15 Virtual Shadow Map
- R-17 Hair / Groom
- R-26 Distance Field
- R-32 Ray Tracing
- R-33 Substrate
- R-34 Movie Render Queue
- R-35 Pixel Streaming
- AN-07 Control Rig
- AN-13 Live Link
- AN-14 Cloth
- P-04 Destruction
- P-05 Vehicle
- P-06 Soft Body
- A-02 MetaSound
- E-06 Blueprint Editor
- E-07 Material Editor（节点图 IDE）
- E-08 Niagara Editor
- E-09 AnimBP Editor
- E-10 Sequencer
- F-13 完整本地化
- F-15 UHT 完整 codegen（折中用扫描器）

---

## 四、cat 推荐开发路线图（与 task2 对照）

> 本路线图**不是替代 task2.md**，而是从"对照 UE 模块全景 + 4 维度评估"重新排序的版本。
> 时间不写周数（单人节奏不稳定），只保留**严格的先后依赖**与**期望的学习产出**。

### 阶段定义

```
Phase A：地基（Foundation, 必须，最先）
Phase B：现代渲染核心
Phase C：游戏对象与生命周期
Phase D：交互三件套（输入 / 音频 / UI）
Phase E：动画运行时
Phase F：物理与导航
Phase G：内容（地形 / 植被 / 后处理 / VFX）
Phase H：移动平台主动验证
Phase I：编辑器深化
Phase J：选学（GAS / RDG / GPU Driven / 网络）
```

下面给出**每个阶段必须输出的能力**与"什么时候做"的依据。

### Phase A — 地基（与 task2 Phase 0 对齐 + 升级）

| 能力 | 与 task2 对照 | 升级点 |
|------|---------------|--------|
| Critical 13 项修复 | 同（已完成） | — |
| Transform dirty + 视锥剔除 | M0.4 | 同 |
| 资源 ownership + opaque handle | M0.3 | 同 |
| SPIR-V 磁盘缓存（DDC v0） | M0.3 含 | 同 |
| **反射 + 序列化 v1**（弱反射） | **task2 未单列** | **新增**：用 X-macro 或 codegen 做最小 reflection，给 Details Panel / Save 系统打底 |
| **任务系统 / 线程池** | 暂未排 | **新增**：为 RDG、async load、动画 BlendPose 铺路 |
| **CVar 注册表** | 暂未排 | **新增**：替代散落的 ImGui 开关 |
| **GPU Profile（Tracy）** | 横切任务 | **提前到 Phase A** |

### Phase B — 现代渲染核心（约等于 task2 Phase 1+2，但顺序更紧）

| 能力 | 与 task2 对照 | 升级点 |
|------|---------------|--------|
| Phong → Blinn-Phong → PBR → IBL | task2 Phase 1 | 同 |
| 材质系统 v1（数据驱动） | M1.5 | **提前到 PBR 前**：先有 Material struct 再写 PBR shader |
| ShadowMap → PCF → CSM | task2 Phase 2 | 同；VSM/ESM 可降为选学 |
| **MeshDrawPipeline 雏形** | 暂未排 | **新增**：把 per-object UB 收敛到 GPU buffer（GPU Scene v0） |

### Phase C — 游戏对象与生命周期（task2 缺）

| 能力 | 与 task2 对照 | 升级点 |
|------|---------------|--------|
| Actor / Component 正名 | 隐含在 cat::Object | **明确语义**：Actor = 可放入世界，Component = 行为/数据；删 ID 散乱 |
| GameMode / PlayerController / Pawn | 无 | **新增** |
| Subsystem（Engine / World） | 无 | **新增**：替代 `Env` / 散乱单例 |
| DataAsset / DataTable | 无 | **新增**：rapidyaml 表驱动 |
| SaveGame | 散落在 Object::save/load | **新增**：成体系的 archive |

### Phase D — 交互三件套（task2 缺）

| 能力 | 与 task2 对照 | 升级点 |
|------|---------------|--------|
| Input Action/Axis 抽象 | 无 | **新增**：Windows + Android 共用一套 |
| Gamepad + Touch | 无 | **新增** |
| Audio 基础（miniaudio / SoLoud） | 无 | **新增**：2D / 3D / 衰减 / 简单 reverb |
| 游戏 UI 体系 | 暂只有 ImGui | **新增**：与 ImGui（编辑器）分桶；游戏 UI 走自己的 retained tree，配 RmlUi / 自写极简 |

### Phase E — 动画运行时（task2 几乎没排）

| 能力 | 与 task2 对照 | 升级点 |
|------|---------------|--------|
| Sequence 播放（已有） | 已有 | 整理为 AnimInstance |
| BlendSpace 1D / 2D | 无 | **新增** |
| StateMachine（代码定义） | 无 | **新增** |
| Montage | 无 | **新增** |
| Notify | 无 | **新增** |
| RootMotion | 无 | **新增** |
| IK（TwoBone + FABRIK） | 无 | **选学** |

### Phase F — 物理与导航（与 task2 Phase 3 + Phase 9 对齐）

- Jolt 接入：与 task2 一致
- Recast/Detour：与 task2 一致
- 顺手做：BT + Blackboard 最小可用版

### Phase G — 内容（task2 Phase 4~8 重排）

> 重点变化：**水体从 5 周压到 1.5 周（湖泊 + Fresnel + 反射）**；FFT 海洋单独留作"selectable learning"。**新增** VFX / Particles 1.5 周。

| 子阶段 | 包含 | 与 task2 |
|--------|------|----------|
| G-Terrain | Heightmap + Splat + LOD + Brush + Grass | 同 Phase 5（保留） |
| G-Water  | 湖泊（Fresnel + 反射） + 河流 | **压缩** Phase 6（FFT 海洋移到 J） |
| G-Sky    | Cubemap + Preetham/Hosek | 同 Phase 4 |
| G-Post   | Bloom + Tonemap + FXAA + TAA v0 + LUT | 同 Phase 8 |
| **G-VFX**| **粒子系统最小可用（CPU 发射 + GPU 模拟可选）** | **新增** |
| G-Decal  | Box decal | **新增**（小） |

### Phase H — 移动平台主动验证（task2 暂缓的部分）

> 触发条件：Phase B 完结即开。**不要等到 Phase 5 后**——你 UE 日常项目就在 Android，cat 早一天上 Android 早一天反哺。

- Android 构建脚本恢复
- Vulkan on Mali / Adreno 实测（subpass / tile shading 验证）
- 触屏输入 + Gamepad 兼容
- Frame Pacing / Choreographer
- 内存上限（移动端 < 2 GB heap）
- 跨平台编译警告清理（`-Wreorder` / `-Wshadow` / case-sensitive include）

### Phase I — 编辑器深化

- Content Browser（资产浏览）
- Details Panel 真反射化
- PIE（编辑器 / 运行态隔离）
- Asset Import 扩展（PNG/HDR/WAV）

### Phase J — 选学

- GAS 自研 mini 版（仅当做 ARPG demo 时）
- RDG（pass ≥ 10 时引入）
- GPU Driven / Meshlet（task2 Phase 10）
- 网络（Replication + Prediction，单独长期 phase）
- FFT 海洋 / 体积云 / SSR / SVOGI 等"风景级"渲染特性

---

## 五、独立游戏品类驱动的能力反推表

> 反向思考：**做这个 demo 至少要哪些子系统？**
> 哪些品类对 cat 路线图的"额外需求"最大？

| 品类 | 关键依赖（cat 缺/有） | 路线图启示 |
|------|------------------------|------------|
| **ARPG**（黑魂 / 暗黑 like） | Animation 全套 / GAS / VFX / Decals / 简单关卡流送 / 音频 | 完成 Phase A~G 即可启动；GAS 决定是否选学 J |
| **FPS** | Animation（拿枪/换弹 IK） / VFX（枪口火花 / 弹道 / 弹孔 decal） / 音频 3D / 网络（可选） | 同 ARPG，强依赖 IK + Decal |
| **MMO** | 网络 Replication + Prediction / 大世界 Streaming / SaveGame / 服务端 / 数据库 | **网络是单独大坑**，建议先做 PVE 单机版 |
| **文明 类 4X** | UI 极重（UMG 等价） / 大量配表 / 存档系统 / AI 决策 / 6 面体 mesh / hex grid | UI + DataTable + SaveGame + BT 即可启动，**不依赖现代渲染** |
| **RTS**（星际 / 红警 like） | Pathfinding 大量代理 / RVO / Mass ECS / Fog of War / RTS 摄像机 / 单位选择框 | 强依赖 AI-01 + AI-06 + AI-07 |
| **戴森球 / Factorio like** | 大量 Instance 渲染 / 巨型 SaveGame / UI 极重 / 简单 mesh / 物流模拟 / Mass ECS | **不依赖高阶渲染**，依赖 ECS + SaveGame + UI + Instance |
| **塔防 / 卡牌** | UI + 表 + 简单 VFX + 音频 | 入门最简单 |
| **横版 / Roguelike 2D** | 2D 渲染 / Tilemap | cat 现在偏 3D，2D 走轻量另起 |

**关键洞察**：
1. **UI + 配表 + 存档**对所有品类是**通用基础设施**，task2 严重缺。
2. **Animation 运行时**是 ARPG/FPS/MMO 三大品类的共同关键路径，task2 也没排。
3. **Mass ECS / Instance**是戴森球 / RTS 的关键，cat 已有 ECS 探索 doc，可对接。
4. **网络**是 MMO 唯一硬门槛，体量巨大；建议**先做 PVE 单机品类**练手。
5. **PBR / IBL / CSM**这些 task2 主线的"渲染奢华件"，**对文明 / 戴森球 / 塔防类几乎不必要**。

---

## 六、与 task2.md 的关键差异说明

> 不是说 task2 错，而是从"对照 UE + 出 demo"角度补的视角。

### 6.1 task2 没正面规划，但对**任何 demo** 都必须的（红色缺口）

- **Audio 子系统**（A-01/04）
- **Input 抽象层**（I-01/02）
- **Actor / Component / GameMode / Subsystem 正名**（G-01/02/08）
- **DataAsset / DataTable**（G-06）
- **完整 SaveGame 体系**（G-09）
- **Animation 运行时**（AN-01~16 中的核心七项）
- **Particles / VFX**（R-18）
- **任务系统 / 线程池**（F-09）—— 不是为了多线程性能，是为了未来的 RDG / async load 不返工

### 6.2 task2 占比偏重、可压缩的

- **Water 5 周** → 压缩到 1.5 周（湖泊 + Fresnel + 反射）；FFT 海洋移到 J 选学
- **VSM / ESM**（M2.3）→ 知道概念即可，PCF + CSM 已够 demo；时间换给 IK + Montage
- **天空 + 体积云**（Phase 4）→ Cubemap + 简单大气 1 周足够；体积云移到 J

### 6.3 task2 排得对，但**应提前**的

- **Trace / FrameStats / GPU Profile**：task2 在 M0.4 末有；建议**Phase A 开端就上 Tracy**，越早越好
- **typed handle**：M0.3 已排，**应在 PBR 之前完成**，否则材质系统建在沙上
- **任务系统**：单独提前到 Phase A

### 6.4 task2 暂缓的，建议**早些回来**的

- **跨平台编译 / Android 构建**：task2 M0.2 推到 Phase 5 后。我建议**Phase B 完成（PBR 跑通）就回来**——你的 UE 日常项目在 Android，cat 也尽早上 Android 才能反哺工作中的判断。

### 6.5 task2 已有但本文档进一步分级的

- **Editor 项目**：task2 把编辑器作为"横切任务"。本文档把它拆为 Phase I，**显式承认 Details Panel 反射化、PIE 隔离、Content Browser 是中期才能做"完"**。

---

## 七、单人开发的"非技术"约束

> 这部分是给自己看的，不是给 UE 比的。

1. **学习闭环优先于工程闭环**
   每个 phase 出口必须有 1 篇学习笔记 + 1 个可跑 demo 场景。否则不算完，直接进入下一 phase 会逐渐失去反馈。

2. **品类锚定一个练手项目**
   建议在 Phase E 完结（Animation 跑通）后，选一个 ARPG 小 demo（一个角色、一种武器、一种敌人、一种技能、一张地图）作为长期"压舱石"。所有后续渲染 / 物理 / VFX 改进都喂给它。

3. **拒绝"全像 UE"的诱惑**
   - Blueprint VM、Material Editor、Niagara Editor、Sequencer 这四件大编辑器，**单人写完整版基本=放弃引擎本体**。
   - Material 用 YAML + 预制 shader template；脚本接 Lua / AngelScript；VFX 用代码定义 + 数据表。

4. **"看一遍 UE"比"抄一遍 UE"重要**
   每写一个子系统，强制读 UE 对应模块的入口文件（如 `FSceneRenderer::Render` / `FMeshDrawCommand` / `UAnimInstance::TickAnimation` / `UCharacterMovementComponent::PerformMovement`）一次，记笔记，然后**用 cat 的命名方式重写**，不要抄类名结构。这样既懂 UE，又不会被 UE 复杂度淹没。

5. **承认有些坑这辈子绕不开 UE**
   - 网络 Replication + Prediction、Editor 大型工具、Cook/Stage/Package、跨平台 shader 编译矩阵——这些 cat 自研到 UE 30% 水平就已经超额完成学习目的，再投入是浪费。

---

> 修订建议：本文件与 `task2.md` 并行存在。
> - `task2.md` 仍是"周级排期"主表；
> - 本文件作为**取舍依据 + 长期愿景**，每次完成一个 task2 phase 后回头校准本表的优先级。

