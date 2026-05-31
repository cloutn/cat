# 阶段04-子审查-GLTF-Skin-Animation

子审查 agent：Wegener `019e7e93-4b96-7f13-9d8d-b9767dec78f0`

## Critical

### C1 Skin joint 数组被压缩导致 IBM / JOINTS_0 下标错配

- commit：`3df3ecb`
- 文件：`cat/gltfLoader.cpp:328-338` `_loadSkin`，`cat/skin.cpp:57-73` `Skin::generateJointMatrix`
- 风险：joint node 未映射时直接 `continue` 会压缩 `m_joints`，但 IBM 和顶点 `JOINTS_0` 索引仍按原 `skin.joints` 下标对齐。中间缺一个 joint 后，后续 joint 会配错 IBM，shader 端矩阵数组也和原 joint index 不一致，导致蒙皮严重错乱或读取未上传矩阵。
- 建议：不要压缩 joint 数组。要么保留 NULL 占位并让整个 Skin fail closed；要么构建完整 `jointCount` 矩阵数组，缺失 joint 明确禁用 Skin。`setEnableSkin(true)` 应只在 joint / IBM 全部有效后执行。

### C2 rotation channel 接受 vec3，Release 下读取 f[3] 越界

- commit：`2049f99`
- 文件：`cat/gltfLoader.cpp:405-438` `_loadAnimChannel`
- 风险：校验只允许 output 是 `vec3` 或 `vec4`，但没有按 target path 区分。rotation channel 如果给了 `vec3`，Release 下 `assert(componentCount >= 4)` 失效后会读取 `f[3]` 越界。
- 建议：ROTATE 必须要求 `cgltf_type_vec4`；MOVE / SCALE 必须要求 `cgltf_type_vec3`；不匹配直接 `return false`，不要生成 channel / frame。

### C3 releaseObjectIDMap 后可复活全局 ID map

- commit：`0698ba8`
- 文件：`cat/object.cpp:164-171` `_objectIDMap`，`cat/object.cpp:276-278` `releaseObjectIDMap`
- 风险：release 后如果还有 Object 析构或 `AnimationChannel::apply` 调 `objectByID`，会重新 lazy-new 一个空 map，造成 ID 查询失真、生命周期清理落到错误 map、潜在泄漏。
- 建议：release 后运行时也必须 fail closed。`objectByID` 在 released 后返回 NULL；析构只在 `s_objectIDMap != NULL` 时 del；最好增加 active object count，确保所有 Object 销毁后才能 release。

## Warning

### W1 glTF inverseBindMatrices 可选字段被当成 invalid skin

- commit：`3df3ecb`
- 文件：`cat/gltfLoader.cpp:315-320` `_loadSkin`
- 风险：glTF 的 `inverseBindMatrices` 是可选字段，缺省语义应为 identity matrices。当前当作 invalid skin，且 mesh shader 已按原 jointCount 选择 `SKIN`，最终可能启用蒙皮 shader 但上传 0 个矩阵。
- 建议：accessor 缺失时为每个 joint 生成 identity IBM；accessor 存在但格式/数量非法时才禁用 Skin。

### W2 _loadIBM 忽略 accessor offset / count / stride

- commit：`2049f99`
- 文件：`cat/gltfLoader.cpp:566-583` `_loadIBM`
- 风险：直接按 `buffer_view->offset` 和 `view->size` memcpy，忽略 `accessor->offset`、`accessor->count`、stride / sparse，并要求 `view->size == sizeof(matrix) * jointCount`。合法的共享 bufferView / accessor offset 会被误判无效或读错数据。
- 建议：用 cgltf accessor API 解包，或至少校验 `accessor->count == jointCount`，从 `buffer_view->offset + accessor->offset` 起读，并按 `accessor->stride` 逐元素复制。

### W3 动画 accessor 假设紧密 float 排布

- commit：`2049f99`
- 文件：`cat/gltfLoader.cpp:412-432` `_loadAnimChannel`
- 风险：`times[i]` / `frameDatas[i * componentCount]` 假设动画 accessor 紧密 float 排布，未处理 stride / sparse。合法但非紧密的数据会生成错误关键帧。
- 建议：用 `cgltf_accessor_unpack_floats` / `cgltf_accessor_read_float`，或显式按 accessor stride 读取。

### W4 超大正时间仍可能触发 float 到 uint UB

- commit：`2049f99`
- 文件：`cat/gltfLoader.cpp:428-431` `_loadAnimChannel`
- 风险：NaN / 负值被挡住了，但 `+Inf` 或超大正数仍会进入 `static_cast<uint>(safeTime * 1000)`，float 到 uint 仍可能 UB。
- 建议：增加 finite 和上限检查，例如 `isfinite(rawTime) && rawTime <= UINT_MAX / 1000.0f`，否则 clamp 或拒绝该 channel。

### W5 unsupported target path 仍生成无效 channel

- commit：`2049f99`
- 文件：`cat/gltfLoader.cpp:424-450` `_loadAnimChannel`
- 风险：unsupported target path，例如 glTF 合法的 `weights`，会落到 `KEY_FRAME_TYPE_INVALID`，但函数仍返回 true 并把无效 frames / channel 加进 Animation，半初始化残留未完全消除。
- 建议：`_cgltfType2KeyFrameType` 返回 INVALID 后立刻 log + `return false`；明确标注暂不支持 morph weights。

## Suggestion

### S1 多 scene 共享 node 时 node map 会绑错 Object

- commit：`adca92e`
- 文件：`cat/gltfLoader.cpp:91-119`，`cat/gltfLoader.cpp:516-524`
- 风险：`m_nodeMap` 是整个 `loadFile` 共享的 `cgltf_node* -> ObjectID`。如果多个 scene 引用同一 glTF node，第二次 `_registerNode` 只 assert + return，Release 下静默保留第一份 Object 映射，后续 skin / animation 会绑到错误 scene 的 Object。
- 建议：按 scene 建独立 node map，或复用同一 Object 树；如果暂不支持多 scene 共享 node，加载时显式拒绝并返回失败。

## 残余风险

- 本组只做静态审查，未跑编译或加载样例。
- 本文件为子审查结果快照；最终报告会再按当前最终代码行号做一次交叉复核。
