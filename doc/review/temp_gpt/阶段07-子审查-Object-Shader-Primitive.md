# 阶段07-子审查-Object-Shader-Primitive

子审查 agent：Poincare `019e7e93-73d4-7723-82f3-dd46a506c42a`

## Critical

### C1 shader reload 失败会返回已释放的 m_deviceShader

- commit：`99ac00f`
- 文件：`cat/shader.cpp:29`
- 风险：当前先 `releaseShader(m_deviceShader)`，随后 `_loadfile()` 失败时跳过 `createShader()`，但没有置空，也把 `m_dirty=false`。下一帧 draw 会使用悬垂 `svkShaderProgram*`，析构还可能二次释放。
- 建议：先加载源码并创建 `newDeviceShader` 到局部变量，成功后再释放旧 shader 并 swap；失败时保留旧 shader，保持 dirty 或记录失败状态。

### C2 cgltf_get_accessor_buffer 可返回 NULL 后，index/vertex 路径未同步判空

- commit：`99ac00f`
- 文件：`cat/gltfLoader.cpp:252`，`cat/gltfLoader.cpp:537`
- 风险：`cgltf_get_accessor_buffer()` 改成可返回 `NULL` 后，primitive index / vertex 路径没有同步判空。`setIndices(NULL, ...)` 会继续写 GPU buffer，`_flattenVertexAttrs` 还会对 `NULL` 做指针偏移和 `memcpy`。外部 `.bin` 缺失或 buffer 未加载时仍会崩。
- 建议：每个 accessor buffer 取出后立即判空；任一 index / attr buffer 缺失时丢弃该 primitive 或让 glTF 加载失败返回。

## Warning

### W1 addChild 不从旧 parent 摘除，也不防重复添加

- commit：`0698ba8`
- 文件：`cat/object.h:49`
- 风险：`addChild()` 直接把 `c->m_parent = this` 并 push，但不从旧 parent 摘除，也不防重复添加。对象重挂或重复 add 时，旧 parent 仍持有同一 child，析构阶段会出现重复 delete / UAF。
- 建议：`addChild` 先处理 `c->m_parent != NULL && c->m_parent != this` 的 detach；若已在当前 `m_childs` 内则直接返回。

### W2 Primitive::release 不是幂等，手动 release 后析构会二次释放

- commit：`bbbbaa1`
- 文件：`cat/primitive.cpp:131`
- 风险：`Primitive::release()` 是 public 且析构会再调一次，但释放 `m_deviceVertexBuffers` 后没有置 NULL。手动 `release()` 后析构会再次 `delete[]` 同一指针。
- 建议：释放后设置 `m_deviceVertexBuffers = NULL`，并把 buffer 释放路径做成可重复调用安全。

### W3 yaml::node 复制 document 但 NodeRef 仍引用原 Tree

- commit：`bbbbaa1`
- 文件：`catbase/cat/yaml.cpp:70`
- 风险：`yaml::node` 拷贝了 `document`，但 `m_node` 仍引用原 `Tree`。一旦 node 逃出原 `document` 生命周期，或 iterator 使用 `m_document` 副本，`NodeRef` 与 Tree 生命周期错位，存在悬垂引用风险。
- 建议：`node` 持有原 `document*` / 共享 owner，不要复制 `Tree`；或复制后把 `NodeRef` 重新绑定到副本 Tree 的同 id 节点。

### W4 非 TEST_VULKAN 分支仍引用已注释成员

- commit：`756ba36`
- 文件：`testCat/client.cpp:290`，`testCat/client.h:112`
- 风险：draw API 改签后，非 `TEST_VULKAN` 分支仍调用 `m_gridPrimitive->draw(...)`，但 `client.h` 里成员已被注释。OpenGL / 非 Vulkan PC 配置会编译失败。
- 建议：删除该死分支或改成当前 `m_grid` 路径；至少静态检查 `TEST_VULKAN` 开 / 关两套宏配置。

## Suggestion

### S1 PipelineKey 仍依赖整结构体字节 hash / compare

- commit：`72c90b3`
- 文件：`catvulkan/cat/pipelineKey.cpp:9`
- 风险：`memset` 已止住当前 padding UB，但 `hash()` / `operator==` 仍依赖整结构体字节。后续加字段或非平凡成员会重新踩坑。
- 建议：改为逐字段 hash / compare，并加 `static_assert(std::is_trivially_copyable<PipelineKey>::value)` 作为过渡保护。

### S2 pick 路径可能使用未设置的 m_env

- commit：`756ba36`
- 文件：`cat/primitive.cpp:77`
- 风险：`Primitive::draw()` 现在只检查 `m_render` 和 shader，但 pick 路径还会用 `m_env`；`setShaderWithPick(shader, env)` 接收 env 却不写 `m_env`。新调用者按注释只调 `setRender + setShaderWithPick` 仍可能 pick 崩溃。
- 建议：`setShaderWithPick()` 内同步 `m_env = env`，或 draw 的 pick path 显式检查 `m_env`。
