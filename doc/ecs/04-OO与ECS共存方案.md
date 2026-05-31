# 04 — OO 与 ECS 共存方案

> 上级：[README.md](./README.md)
> 关心问题：在不做整体迁移的前提下，OO 主干和 ECS 旁路如何稳定共存。

## 1. 现状本身就是共存

`testCat/main.cpp` 同时构造 `Client`（OO 主干）和 `World`（ECS 雏形），两者无依赖。`Object` 既是场景树节点（OO），也已经预留 `m_fixComponents` / `m_components` 的"组件容器"槽位。这是 cat **天然的混合架构起点**，并不需要"启动一个共存项目"。

需要做的不是"如何共存"，而是"如何让共存稳定且可演进"。

## 2. 三种共存模式

### 模式 A：OO 主干 + Component as Data（**推荐 / 当前事实路线**）

```
+-- Object (OO 节点) ----------------------+
|  m_id, m_parent, m_childs, m_transform   |
|  m_mesh, m_skin                          |
|  m_fixComponents [TRANSFORM, PHYSICS, ...]
|  m_components [Lighting, Audio, AI, ...] |
+------------------------------------------+
                |
                v
        每个 Component 是 plain data + 行为方法

World 雏形：闲置或仅用于演示
```

特点：

- Object = Entity 替代物；
- 行为通过组件挂载，不通过继承；
- 像 Unity GameObject + MonoBehaviour，或 UE Actor + ActorComponent；
- 组件遍历是 OO 的（`for (auto* c : object->m_components) c->update()`），不是 SOA。

**适合现阶段 cat。** 业界主流引擎都是这条路，task2 全部 phase 都可以承载。

### 模式 B：OO 主干 + ECS 旁路系统（**未来扩展路径**）

```
+-- Object (主干) -----+      +-- World (旁路) ---+
|  m_id, m_transform   |<---->|  Entity = ObjectID|
|  m_mesh, m_skin      |      |  ParticleComponent|
|  m_components        |      |  VegetationInst   |
+----------------------+      |  AIBlackboard     |
                              +-------------------+
        ↑                              ↑
   主线渲染 / 编辑器 / 物理       批量同质实体的 SOA 更新
```

特点：

- `Object` 仍是身份载体；
- `World` 承担"批量同质实体"（粒子、植被、子弹、群体 AI 等）；
- 桥接方式：让 `Entity` 和 `Object::id()` 共用同一编号空间（`Entity = ObjectID`），由 `Object::objectByID(entity)` 反查到 OO 节点；反方向 `Object::entity()` 直接返回 `m_id`；
- 不是所有 Entity 都对应 Object —— 粒子 / 草 这类高密度同质实体可以**纯 ECS**，没有 OO 镜像；
- 编辑器看到的"对象"仅是有 OO 镜像的那一部分。

**适合未来出现粒子系统 / 植被 / 群体 AI 时启动。**

### 模式 C：ECS 主干 + Object 作为 Facade（**不推荐**）

```
World 是真数据源
Object 退化成 Facade：
  Object* obj = scene->find(...);
  obj->setMove(v);   // 内部转发到 world->get<TransformComponent>(entity).move = v
```

特点：

- 全量迁移，工作量见 `02-改造难度评估.md`；
- Facade 让"现有外部代码不爆改"；但 Facade 本身是个长期债务，最终还是要让外部直接用 ECS 接口；
- Unity DOTS 的 baking / conversion workflow 走的是这条路，但需要完整工具链支撑（cat 没有）。

**不推荐**：若决心走 ECS 主干，不如直接用模式 B 渐进；模式 C 的代价和模式 B 终态相同，但前期投入大得多。

## 3. 推荐方案：模式 A 为主，预留模式 B 接口

### 3.1 主干（不动）

- `Scene` / `Object` / `Mesh` / `Skin` / `Animation` / `Material` / `Shader` / `Env` 全保持 OO 现状；
- 加 task2 Phase 0 规划的 dirty flag / 视锥剔除 / opaque handle 等基础设施。

### 3.2 Object 内组件容器收紧（短期 1~2 天工作量）

实际做法：

1. **构造列表初始化**修复（CR 已记录）：
   ```cpp
   Object::Object(Object* parent) :
       m_id            (...),
       m_parent        (parent),
       m_mesh          (NULL),
       m_skin          (NULL),
       m_transform     (NULL),
       m_fixComponents (),       // 显式 zero-init
       m_components    (),
       ...
   {
       _objectIDMap().add(this);
   }
   ```

2. 补齐 `Object::addComponent<T>()` / `Object::getComponent<T>()` / `Object::removeComponent<T>()`：

   ```cpp
   template <typename T>
   T*  addComponent    ();             // new T()，挂到 m_components 或固定槽
   template <typename T>
   T*  getComponent    () const;       // 按 type_id 查
   template <typename T>
   bool removeComponent();
   ```

   - 内部用 `scl::type_id<T>()` 做 key；
   - 固定槽（如 `FIX_COMPONENT_TYPE_TRANSFORM`、未来的 `FIX_COMPONENT_TYPE_PHYSICS`）走 `m_fixComponents[idx]`；其他走 `m_components` 数组。

3. `Component` 基类加最小接口：

   ```cpp
   class Component
   {
   public:
       virtual ~Component() = default;
       virtual int typeID() const = 0;     // 或 scl::type_id<Self>()
   };
   ```

4. `~Object` 释放组件：

   ```cpp
   for (int i = 0; i < FIX_COMPONENT_TYPE_COUNT; ++i)
           safe_delete(m_fixComponents[i]);
   for (int i = 0; i < m_components.size(); ++i)
           safe_delete(m_components[i]);
   ```

> 说明：`m_transform` 当前是独立成员（`Transform* m_transform`），与 `m_fixComponents[FIX_COMPONENT_TYPE_TRANSFORM]` 实际上**不是同一份**。短期建议保留这种分离（避免改 transform 牵动整套 setter）；长期规划是把 Transform 迁到固定槽，但属于另一个独立工作项。

### 3.3 World 雏形保留 + 接口扩展（按需，不立即做）

只在出现真实 ECS 需求时再扩，扩的方向是：

```cpp
class World
{
public:
    Entity              createEntity   ();
    void                destroyEntity  (Entity e);
    bool                isAlive        (Entity e) const;

    template <typename T> T*    addComponent    (Entity e);
    template <typename T> T*    getComponent    (Entity e);
    template <typename T> bool  hasComponent    (Entity e) const;
    template <typename T> bool  removeComponent (Entity e);

    // 批量遍历入口（只需要单组件视图就够走粒子 / 植被）
    template <typename T, typename Fn>
    void                forEach        (Fn&& fn);
};
```

- 加 generation：`Entity` 改成 `{ uint32 index : 24; uint32 generation : 8; }`，避免 stale handle；
- 加 `EntityIndex` 稀疏集，让 `getComponent` 走 O(1)；
- archetype 暂不引入（cat 短期不会有"多组件交集 join"的需求）。

### 3.4 ID 空间统一约定

让 `cat::Entity` 与 `Object::id()` 共用同一池子，**这是把模式 A 升到模式 B 的关键**：

- `typedef int Entity;` 与 `m_id` 同类型；
- `Object` 创建时 `m_id = world->createEntity()` 而不是用全局静态 `ObjectIDMap::alloc_id()`；
- 全局 `Object::objectByID(id)` 退化为 `world->getObject(entity)`；
- 这一步之后，未来任何"Entity 同时挂 OO 镜像"的需求都能 0 改动桥接。

> 这一步与 `Object::releaseObjectIDMap()` 未置 NULL 的 critical bug 修复**正好可以一起做**：把全局静态 map 退役为 World 的实例字段。

### 3.5 数据所有权红线

明确把权威数据源标住，任何时候只能有一个：

| 数据 | 权威所有者 | 备注 |
|------|------------|------|
| Transform（local / global matrix） | `Object::m_transform` | World **不**持 Transform 组件 |
| Mesh / Primitive / Material / Shader | `Env` 共享池 + `Object` 强引用 | World 不 own |
| Skin / 关节引用 | `Object::m_skin` | 关节引用是 Object* / Entity |
| Animation 时间线 | `Animation` / `Scene` | 同上 |
| 粒子 / 植被实例 / 群体 AI | World（未来） | 没有 OO 镜像 |
| Physics body 句柄 | `PhysicsComponent` 挂 `Object::m_components` | 已经在物理设计文档定下 |

红线：**World 永远不要复制 Transform 组件**。一旦 Transform 出现两份，同步 bug 会成为长期债务。

## 4. 共存的最小验证 demo

可以挑一个低风险场景把模式 B 跑通，作为可行性验证（不是必须做，但比写一堆文档更能定型）：

- 在 `testCat/main.cpp` 不删 `world.addComponent(e1, TestComponent(77))`；
- 把 `TestComponent` 改成 `ParticleComponent { vector3 pos; vector3 vel; float life; }`；
- 写 `World::forEach<ParticleComponent>(fn)`，在 `Client::run` 单帧主循环里调用一次更新，把 `pos += vel * dt`；
- 渲染端复用 `IRender::draw2`，但走批量 push constant；
- 保留 Object/Scene 主线渲染不动。

跑通这个 demo 之后，你会拿到三个有用的产物：

1. World 的 forEach / generation / sparse set 实现完成度初步评估；
2. 粒子和场景共渲染的接口边界（`Client::run` 该怎么编排）；
3. 一个未来粒子 / 植被 / Boid 直接复用的脚手架。

## 5. 共存模式的非目标（明确不做）

为了避免共存方案越做越大，**明确以下事情不做**：

- ❌ 不把 `Object` 改成 ECS 实体的语法糖；
- ❌ 不把 `Mesh` / `Primitive` 拆成组件；
- ❌ 不引入 archetype / chunk allocator；
- ❌ 不为 `World` 写 system 调度框架（手动在主循环里 forEach 即可）；
- ❌ 不用 ECS 包办物理 / 动画 / 编辑器（这些都还在 OO 路径上）；
- ❌ 不引入第三方 ECS 库（EnTT / flecs）—— 与 `scl` / 风格约束冲突。

## 6. 共存可能性结论

| 维度 | 评级 |
|------|------|
| 共存技术可行性 | **极高**（现状已是雏形混合体） |
| 共存对当前主线的侵入 | **极低**（只需补 Object 组件 API + 收紧 World 接口） |
| 推荐共存模式 | **A（OO 主干 + Object 内组件） + 预留 B（World 旁路）** |
| 启动模式 B 的触发条件 | 出现"千+同质实体"的真实需求（粒子 / 植被 / Boid） |
| 长期演进上限 | 完全可以走到模式 B 终态，不需要切到模式 C |
