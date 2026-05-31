# 06 — Unity 风格 Component 化的中间方案

> 上级：[README.md](./README.md)
> 关心问题：现在把 `Object` 改成"类 Unity 的 GameObject + Component"结构，并在 task2 推进期间保持这个结构，是否对未来 ECS 改造有帮助？现在做这个改造的代价和必要性有多高？

## 1. 先消歧义："Unity 风格 Component"具体指什么

把"Unity 风格"摊开成三个分立的事实层，避免后面讨论变成大词混战：

| 层 | 事实 | 在 ECS 化里的角色 |
|---|------|---------------------|
| L1 | **新行为通过 `addComponent<T>` 挂到 Object，而不是 Object 加成员或全局 Manager** | 决定"未来组件是否已经是组件"，**减负幅度最大** |
| L2 | **Object 自身瘦身：`m_transform` / `m_mesh` / `m_skin` 也变成组件，Object 只剩 id / parent / children / 组件容器** | 决定"是否保留 Actor-style 节点身份"，影响编辑器面板与序列化的写法 |
| L3 | **Component 拥有独立的生命周期方法**（`onAttach / onDetach / update / lateUpdate`），并由统一的"组件遍历器"驱动 | 决定"是否引入 system 模型雏形"，是 ECS 调度器的前身 |

cat 当前的事实位置：

- **L1**：基础设施有（`m_components` 数组 + `Component` 基类），但实际上没用到 → 半就位；
- **L2**：完全没做（`m_transform` / `m_mesh` / `m_skin` 是 `Object` 直接成员，不在 `m_components` 里）；
- **L3**：完全没做（`Component` 基类是空类，没有 `update()` 接口）。

一个值得关注的事实：`FIX_COMPONENT_TYPE_TRANSFORM = 0` 这个枚举值已经在 `def.h` 里写好，且 `Object::m_fixComponents[FIX_COMPONENT_TYPE_COUNT]` 数组已经声明，**但 `m_transform` 实际上是另一个独立成员**。这是一个明显的"原作者起手做 L2 但搁置了"的痕迹。

## 2. 三档中间方案

按"做到哪一层"切三档，各档独立评估：

| 档位 | 涵盖层 | 类比 | 改造代价 | ECS 减负 | 推荐度 |
|------|--------|------|----------|----------|--------|
| **轻档** | L1 + 护栏 | "现状的 Object 加完整 Component API" | **1~2 周** | 中（节省 ~30%） | ★★★★★ |
| **中档** | L1 + L2 + L3（弱）| **UE Actor + ActorComponent**（保留 Object 身份，Transform 留作"根"槽位）| **3~5 周** | 高（节省 ~50%）| ★★★★☆ |
| **重档** | L1 + L2 + L3（强）| **Unity GameObject + MonoBehaviour**（Object 完全瘦身，连 Transform 也作为普通组件）| **6~8 周** | 高（节省 ~55%）| ★★☆☆☆ |

这三档**不是是非二选**，而是在同一条路径上的三个停车点。下面分别评估。

## 3. 各档详细评估

### 3.1 轻档（L1 + 5 条护栏）

**做什么**：

- 把 04 推荐的"Object 组件 API"补齐：`addComponent<T>` / `getComponent<T>` / `removeComponent<T>` / `hasComponent<T>`；
- `Component` 基类加 `virtual ~Component()` + `virtual int typeID() const` + `virtual void onAttach(Object*)` / `onDetach()`；
- `~Object` 释放 `m_fixComponents` / `m_components` 里的所有组件；
- 修复"`m_fixComponents` / `m_components` 未在构造列表初始化"的 critical bug（CR 已记录）；
- 落 05 的 5 条护栏（新功能默认是组件、Transform 单源、ID 引用、不起 Manager、统一 schema）；
- `m_transform` / `m_mesh` / `m_skin` **保持现状**，不动。

**和 04+05 的关系**：这一档其实就是 04 推荐方案 + 05 护栏的具体落地，**不是新方案**。这里把它独立列出来，是为了和中档 / 重档拉对比基线。

**ECS 减负幅度**：约 30%。原因：

- 新代码（光照 / 阴影 / 物理 / 草地 / 寻路 ...）天然组件化，未来 ECS 化只需要把组件存储从"Object 内"换成"World 内"；
- 但旧代码（Mesh / Skin / Animation / Material）仍然是 OO 内禀，未来 ECS 化时这部分要单独迁移；
- 编辑器仍然要走"Object 的 setter + 组件的 getComponent"两套路径，inspector 写两遍。

**代价**：1~2 周。落到 `task2.md` Phase 0 里完全消化得掉，不打乱节奏。

**与 ECS 的关系**：保留了模式 B（World 旁路）和模式 C（ECS 主干）的全部演进可能，但都没有提前付出。

### 3.2 中档（L1 + L2 + L3 弱，UE Actor + Component 风格）

**做什么**：

在轻档基础上：

- **L2 部分迁移**：
  - `m_mesh` → `MeshComponent`，挂到 `m_fixComponents[FIX_COMPONENT_TYPE_MESH]`（新增枚举值）；
  - `m_skin` → `SkinComponent`，固定槽或可变槽都可（用得不多，建议可变槽）；
  - **`m_transform` 保留为 `Object` 直接成员** —— 对应 UE 的 `AActor::RootComponent` 模式："每个 Actor 都有一个 Transform，且它特殊"；
  - 编辑器入口改：`obj->mesh()` 退化为 `obj->getComponent<MeshComponent>() ? ...->mesh() : NULL`；为了不爆改外部代码，**保留 facade 方法**（`Object::mesh()` 内部转发）。
- **L3 弱版**：
  - `Component::update(double dt)` 接口；
  - `Object::update(dt)` 遍历自己所有 component 的 `update`；
  - `Scene::update(dt)` 遍历 Object；
  - 暂不引入"按组件类型批量 forEach"的系统模型 —— 那是 ECS 的工作。
- **统一序列化 schema**（05 护栏 5）：

  ```yaml
  - id: 42
    name: "DamagedHelmet"
    transform: { move: [...], rotate: [...], scale: [...] }   # facade，从 m_transform 取
    components:
      mesh:   { gltf_path: "...", primitive_count: 5 }
      skin:   { joints: [3, 5, 7, ...] }
      animation: { ... }
  ```

- 编辑器 Inspector 改成"Object 头部固定显示 Transform，下面遍历 components 渲染各自面板"。

**为什么 Transform 留作根（不进 m_components）**：

1. UE / Godot 的 SceneNode 都是这个模型 —— Transform 是节点身份的一部分，不是"可拆卸的组件"。`Object` 没有 Transform 在概念上不成立。
2. 减少改动量：`globalMatrix` / `parentGlobalMatrix` / 设置 setter 全部不动。
3. ECS 化时仍然简单：`m_transform` → `World` 里的 `TransformComponent` 是机械替换，不需要先把 `m_transform` 拆进 `m_fixComponents` 再拆出来。
4. 与"Transform 单源权威"护栏吻合 —— Transform 长在节点身上，物理/阴影/八叉树等谁都不复制。

**ECS 减负幅度**：约 50%。原因：

- L1 + L2 之后，**所有数据除了 Transform 都已经是组件**；
- L3 弱版让"组件 update"成为编辑器和主循环的统一入口；
- ECS 化时主要工作变成：
  - `Object::m_components` 数组 → `World` 内的 SOA 组件数组；
  - `Component*` 裸指针 → `Entity` + 组件 view；
  - 编辑器 / 序列化 / 主循环路径**几乎不动**。

**代价**：3~5 周。可以全部落进 Phase 0（原计划 4 周 + 1~2 周延长），或拆 1~2 周到 Phase 1 早期。

**风险**：

- `m_mesh` 改 `MeshComponent` 时，`Mesh` 自身（`cat/mesh.h`）的 `m_parent` 反向指针、`Skin::generateJointMatrix(...)` 里对 mesh global transform 的依赖、`AnimationChannel::apply()` 的 setter 路径 —— 都要顺一遍。属于"看似简单但坑多"的部分。
- 编辑器 Pick / Gizmo / Save 多处 `m_selectObject->mesh()` 这种链式调用要查一遍并加防御。

### 3.3 重档（L1 + L2 + L3 强，纯 Unity GameObject 风格）

**做什么**：

在中档基础上：

- `m_transform` 也拿掉，变成 `TransformComponent`，挂在 `m_fixComponents[FIX_COMPONENT_TYPE_TRANSFORM]`；
- `Object` 退化为"id + parent + children + components"，不再有任何业务字段；
- L3 强版：引入"按组件类型批量遍历"接口（`Scene::forEach<MeshComponent>(fn)`），开始有"render system 雏形"。

**ECS 减负幅度**：约 55%（比中档只多 5%）。原因：

- 把 Transform 也拆成组件，对 ECS 化来说是把"Transform 单源权威"从 `Object::m_transform` 变成 `Object 持的 TransformComponent` —— **这一步本身在 ECS 化里也只是机械替换**，提前做 vs 后做差别不大；
- 真正减负的是 L1 + L2，L2 把 Transform 也拆开是边际收益；
- L3 强版"按类型批量遍历"是 World 的 forEach 的雏形，但这个抽象写在 `Scene` 上还是 `World` 上区别不大。

**代价**：6~8 周。

**ROI 不划算的原因**：

- 把"Transform 是 Object 的一部分"这件事拆开，会让所有"`obj->position()` / `obj->setMove()` 这种 0.5 秒能写完的代码"变成 `obj->getComponent<TransformComponent>()->setMove()`；要么写 facade（白做），要么爆改全部调用点（高风险）；
- ECS 减负只比中档多 5%；
- 与 `task2.md` 学习路线对照的是 UE，UE 不走纯 Unity 模型，走中档；
- 单人项目，6~8 周是 Phase 0 的 1.5~2 倍预算。

## 4. 三档对未来 ECS 改造的具体帮助

把 02 难度评估里的"硬伤"逐条对照，看每档帮你解决了哪些：

| 02 中的硬伤 | 轻档（L1） | 中档（L1+L2+L3 弱） | 重档（L1+L2+L3 强） |
|------------|-----------|----------------------|----------------------|
| 场景图层级 vs ECS 平铺 | 不解决 | 不解决 | 不解决（这是 ECS 化本身的事） |
| 跨实体引用太多（Skin → Object*）| 部分（护栏 3 强制 ID）| 大部分（SkinComponent 持 ID）| 大部分 |
| 资产对象 vs ECS 组件 | 不解决 | **解决**（MeshComponent 持 Mesh 句柄）| 解决 |
| 编辑器 / Gizmo / Pick / Save 全是 Object*| 部分（统一序列化 schema）| **大部分**（inspector 走 components） | 大部分 |
| 风格规则与 ECS 范式有摩擦 | 不解决 | 不解决 | 不解决（这是 scl 容器的事） |
| 析构顺序与全局静态 | 部分（统一 ID 池）| 部分 | 部分 |

**结论**：
- **轻档**主要是"立护栏不让事情变更糟"；
- **中档**实质性减少了"未来 ECS 化时要啃的硬伤"约一半；
- **重档**相对中档边际收益小。

## 5. 现在做 Unity 风格改造的必要性

| 维度 | 轻档 | 中档 | 重档 |
|------|------|------|------|
| 修复 critical bug（构造列表初始化）| **必要** | 必要 | 必要 |
| 让 task2 后续 phase 有统一组件挂载入口 | **必要** | 必要 | 必要 |
| 与 UE 学习对照线对齐 | 中 | **强**（UE Actor+Component 同款）| 弱（不是 UE 模型） |
| ECS 化未来收益的"门票"| 部分 | **多数** | 多数 |
| 投入产出比（ROI） | **极高** | 高 | 中 |
| 是否影响 task2 节奏 | 不影响（Phase 0 内消化）| 拖长 Phase 0 ~1~2 周 | 拖长 Phase 0 ~2~4 周 |

**必要性结论**：

- **轻档：高必要**。它和 04 + 05 的推荐**重合度 100%**，不需要单独立项 —— Phase 0 顺手做。这是任何方案都建议做的最小集。
- **中档：中~高必要**。如果你**确实倾向于未来某天会走向 ECS 或更结构化的引擎设计**，中档值得投。它是 cat 学习路线（对照 UE）最匹配的形态：UE Actor + ActorComponent 是 cat 想长成的样子。
- **重档：低必要**。除非有明确的"全量 ECS 迁移"目标，否则边际收益不够支付边际成本。

## 6. 推荐方案：中档，分三段嵌入 Phase 0~1

> 推荐**中档（UE Actor + Component 风格）**，因为：
> 1. 它是 04 + 05 的自然延伸（轻档已经必做）；
> 2. ECS 减负幅度从 30% 跃升到 50%，是性价比最高的一档；
> 3. 与 task2 的"对照 UE 学习"主线吻合；
> 4. 总投入 3~5 周，可消化在 Phase 0 内或 Phase 0→1 衔接处；
> 5. 即便未来不做 ECS，这个结构本身也是更干净的引擎；
> 6. 算是把原作者起手 L2 但搁置的工作做完。

### 推荐里程碑

放进 `task2.md` Phase 0 之内或紧接其后：

#### M-Component.1（~1 周，与 Phase 0 并行）

- 修复 `m_fixComponents` / `m_components` 未初始化（CR 已记录）；
- 补 `addComponent<T>` / `getComponent<T>` / `removeComponent<T>` / `hasComponent<T>`；
- `Component` 加 `virtual ~Component() = default` + `virtual int typeID() const = 0` + `onAttach(Object*) / onDetach()`；
- `~Object` 显式 `safe_delete` 所有组件；
- 在 `testCat/main.cpp` 把 `World::addComponent` 的 demo 也接入 `getComponent` / `forEach`，验证 World 接口扩展是否合理（可选，与 ECS 评估并行）；
- **此时仍未引入任何业务组件**，纯接口层。

#### M-Component.2（~2~3 周）

- `MeshComponent`（包装 `Mesh*`，持有所有权）：
  - `Object::loadNode` 加载 mesh 时改成 `obj->addComponent<MeshComponent>()->load(...)`；
  - `Object::mesh()` 保留为 facade：`return getComponent<MeshComponent>() ? ...->mesh() : NULL`；
  - `Object::draw` 内部走 component；或者更彻底地把 draw 也搬进 `MeshComponent::draw`，由 `Object::draw` 遍历组件分发。
- `SkinComponent`（包装 `Skin*`）：
  - `m_skin` 退役；
  - `SkinComponent::m_joints` 是 `scl::varray<int>`（Entity / ObjectID），不是 `Object*` —— **顺手吃掉护栏 3 的债务**；
  - `SkinComponent::generateJointMatrix(...)` 里通过 `Object::objectByID` 反查 joint。
- `Animation` 保持现状（不是组件），但 `AnimationChannel::apply()` 的 setter 路径走 facade，等中档完成。
- 关键回归测试：DamagedHelmet / Sponza 跑通。

#### M-Component.3（~1~2 周）

- 序列化按统一 component schema 重写（`Object::save`、`Scene::save` / `load`），见 §3.2 的 yaml 示例；
- 编辑器 Inspector 改成"头部 Transform 区 + 组件循环渲染"；
- 把"新功能默认是组件"写进项目规范（`.cursor/rules` 或 `doc/代码审查-规范.md`）；
- Phase 1 之后所有新行为（光照 / 阴影 / 物理 / 地形 / 草地 / 寻路 ...）默认组件化，没有例外审批。

### 不在中档里做的事

- 不动 `m_transform`（它在 `Object` 直接成员，是 UE Actor RootComponent 模式）；
- 不动 `Mesh / Primitive` 内部结构（仍然 OO，仍然 `Mesh::draw → Primitive::draw → IRender::draw2`）；
- 不引入 `forEach<T>` 跨 Object 批量遍历（那是 World 的活）；
- 不重写 `Animation` / `AnimationChannel`（它们是"时间线对象"，不是组件，类似 UE 的 `UAnimSequence`）。

## 7. 风险

| 风险 | 概率 | 缓解 |
|------|------|------|
| `m_mesh` / `m_skin` 改组件后回归出 bug | 中 | facade 方法保留旧 API，外部代码无需爆改 |
| 序列化 schema 变化导致 yaml 不兼容 | 高 | cat 现在没有真在用的 yaml 资产，破坏成本低；早做早收益 |
| 单人开发期间分支管理 | 中 | 中档 3~5 周可以单分支推进，不需要长期 feature flag |
| Phase 0 总预算被吃满 | 中 | M-Component.3 可以拆到 Phase 1 早期，不是硬约束 |
| "做了组件化但 task2 走着走着又退化" | 中 | 把 05 的 5 条护栏写进 `代码审查-规范.md`，每个 PR 自检 |

## 8. 一段话结论

> **轻档（≈ 04+05 的推荐）任何情况都建议立刻做，1~2 周顺手解决，是 ECS 友好纪律的最小集。**
>
> **中档（UE Actor + ActorComponent 风格）是最值得投资的中间方案 —— 3~5 周拿到 50% 的 ECS 减负 + 干净的引擎结构 + 与 UE 学习路线对齐 + 把原作者起手搁置的工作做完。强烈推荐落到 Phase 0 末或 Phase 1 初。**
>
> **重档（纯 Unity GameObject 模型）边际收益不够，不推荐。**
>
> 中档完成后，未来如果要做 ECS，成本曲线大约是 12~25 周（vs 不做的 24~50 周）；如果未来不做 ECS，这个结构本身也是 cat 长成"成熟小引擎"的合理形态，不是沉没成本。
