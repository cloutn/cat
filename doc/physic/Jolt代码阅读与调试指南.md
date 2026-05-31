# Jolt 代码阅读与调试指南

> 适用版本：`free/jolt`（仓库内 `version.txt`，与上游 jrouwe/JoltPhysics 对齐）
>
> 本文档面向「需要在 Jolt 源码里下断点、单步、查变量」的场景，不解释物理学。
> 所有行号都对应仓库中 `free/jolt/Jolt/` 下的源文件，发生上游 sync 后可能会有 ±10 行漂移，请按函数名定位。

---

## 0. 怎么用这份文档

1. 先看 §1 目录速查 → 知道某个模块的文件在哪。
2. 第一次跟一帧仿真：照 §2 的 Update 调用图把断点全打上，按 F5 跑一帧，观察每个 Job 的进入顺序。
3. 后续单点调试某类问题：按 §6「症状 → 断点位置」表查。
4. 阅读 cpp 时配合 `Jolt.natvis`（§7）让 watch 窗口能看清 `BodyID`、`Vec3`、`Mat44`、`Quat`、`SubShapeID` 的真实值。
5. 一些隐藏调试开关在 §8（宏控制），需要重编 Jolt 才生效。

---

## 1. 目录速查（只列调试时常进的文件）

```
free/jolt/Jolt/
├── Core/                          线程/容器/分配器基础设施
│   ├── JobSystem.h / .inl         任务图 (Barrier、JobHandle、依赖计数)
│   ├── JobSystemThreadPool.*      默认线程池实现；卡死 / 死锁先看它
│   ├── TempAllocator.h            每帧用的 stack allocator；OOM 看这里
│   ├── Profiler.*                 JPH_PROFILE_FUNCTION / SCOPE 的实现
│   └── FixedSizeFreeList.*        Body、ContactConstraint 的内存池
├── Geometry/                      纯几何算法（无状态、易单测）
│   ├── GJKClosestPoint.h          GJK 收敛失败时进来看
│   ├── EPAPenetrationDepth.h      穿透深度计算
│   ├── EPAConvexHullBuilder.h     EPA 的 hull 维护
│   ├── ClosestPoint.h             点-三角形/四面体最近点（GJK 子步骤）
│   ├── RayAABox.h / RayTriangle.h 射线测试基础
│   └── ConvexHullBuilder.cpp      ConvexHullShape 构建时跑
├── Physics/
│   ├── PhysicsSystem.{h,cpp}      仿真主入口（§2 整个 Update 在这里）
│   ├── PhysicsUpdateContext.h     一帧的临时数据 / Job handle 总线
│   ├── PhysicsSettings.h          所有可调参数集中地（slop、迭代次数...）
│   ├── IslandBuilder.{h,cpp}      把 active 体 + 约束 + 接触分簇
│   ├── LargeIslandSplitter.*      把大岛拆分给多线程 solver
│   ├── PhysicsLock.h              所有锁的 RAII 包装（断 deadlock 用）
│   ├── DeterminismLog.h           JPH_DET_LOG 宏定义
│   ├── StateRecorder*             SaveState/RestoreState 接口与 impl
│   ├── Body/
│   │   ├── BodyManager.{h,cpp}    Body 池、Active 列表、Body 互斥锁数组
│   │   ├── Body.{h,cpp,inl}       单个刚体；Position / Rotation / Bounds
│   │   ├── BodyID.h               (index | seq) 32-bit 编码
│   │   ├── BodyInterface.cpp      外部 API（AddBody / SetLinearVelocity …）
│   │   ├── MotionProperties.*     线/角速度、阻尼、惯性张量、ApplyForce…
│   │   └── BodyLock*.h            读/写锁辅助（外部代码不要直接 GetBody）
│   ├── Collision/
│   │   ├── BroadPhase/
│   │   │   ├── BroadPhaseQuadTree.* BroadPhase 默认实现入口
│   │   │   ├── QuadTree.{h,cpp}     真正的并发 4-叉 AABB 树
│   │   │   └── BroadPhaseLayer.h    BroadPhaseLayer / ObjectLayer 过滤接口
│   │   ├── Shape/                   所有形状的实现（见 §4.4 子表）
│   │   ├── NarrowPhaseQuery.*       射线、ShapeCast、CollidePoint 等查询
│   │   ├── CollisionDispatch.{h,cpp} Shape×Shape 分派表（GJK / 专用算法）
│   │   ├── CollideShape.h           CollideShapeResult / Settings
│   │   ├── ContactListener.h        外部碰撞回调接口
│   │   ├── ManifoldBetweenTwoFaces.{h,cpp} 两凸面裁剪 → 接触流形
│   │   ├── InternalEdgeRemovingCollector.h 防止网格穿过内边产生跳板
│   │   └── TransformedShape.*       带变换的 Shape，运行时碰撞用
│   ├── Constraints/
│   │   ├── ContactConstraintManager.{h,cpp} 接触约束 + 缓存（最重要的约束文件）
│   │   ├── ConstraintManager.*      非接触约束的全局表
│   │   ├── Constraint.h             基类
│   │   ├── *Constraint.{h,cpp}      具体约束（Hinge/Slider/SixDOF/…）
│   │   ├── CalculateSolverSteps.h   每岛自适应迭代步数计算
│   │   └── ConstraintPart/          单方程基元（Axis/Point/Angle/Spring…）
│   ├── Character/                   CharacterVirtual、Character
│   ├── SoftBody/                    软体（vertex/edge/volume 约束）
│   ├── Ragdoll/、Vehicle/           应用层封装
│   └── ...
└── RegisterTypes.cpp                插入序列化与 ShapeFunctions 注册（启动时一次）
```

---

## 2. 一帧 Update 的完整调用图

主入口：`PhysicsSystem::Update(...)`，位于 `Jolt/Physics/PhysicsSystem.cpp:174`。
该函数把一整帧切成若干 collision step（默认 1），每个 step 创建一组 Job，靠 `JobHandle::RemoveDependency()` 拓扑触发。

### 2.1 Job 依赖图（按依赖关系，从上到下）

```
                Update() entry  PhysicsSystem.cpp:174
                       │
                       ▼
           [BroadPhasePrepare]   PhysicsSystem.cpp:296  (UpdatePrepare on QuadTree)
                       │
   ┌───────────────────┴──────────────────────┐
   ▼                                          ▼
[StepListeners]                          (previous step done)
PhysicsSystem.cpp:390 → JobStepListeners @ 680
   │
   ├─►[DetermineActiveConstraints] 376 → @ 710
   │       (按 batch 把活跃约束扫到 mActiveConstraints[])
   │
   └─►[ApplyGravity]              347 → @ 743
           (对帧首活跃 RigidBody 应用 gravity / 陀螺力)
                       │
                       ▼
        [SetupVelocityConstraints]  357 → @ 790
        [BuildIslandsFromConstraints] 365 → @ 810
                       │
                       ▼
                 [FindCollisions]   314 → @ 890
                  (broad+narrow，body pair queue 见 §4.3)
                       │
                       ▼
        [UpdateBroadphaseFinalize] 278  (QuadTree 新旧树原子切换)
                       │
                       ▼
          [FinalizeIslands]   405 → @1364   IslandBuilder::Finalize
          [BodySetIslandIndex] 431 → @1379  (debug 染色用)
                       │
                       ▼
        [SolveVelocityConstraints] 489 → @1399
           - WarmStart                  ContactConstraintManager.cpp:1542
           - SolveVelocityConstraints   ContactConstraintManager.cpp:1643
           - 非接触约束 part             各 *Constraint::SolveVelocityConstraint
                       │
                       ▼
          [PreIntegrateVelocity] 504 → @1585
          [IntegrateVelocity]    518 → @1602   (写新位置；判定要 CCD 的 body)
          [PostIntegrateVelocity]529 → @1732
                       │  (如果有 CCD body：动态创建 FindCCDContacts job 1750)
                       ▼
          [ResolveCCDContacts]   540 → @2102
                       │
                       ▼
        [SolvePositionConstraints] 553 → @2493  (Baumgarte / Split impulse)
                       │
                       ▼
        [ContactRemovedCallbacks] 421 → @2376
                       │
                       ▼
          [SoftBodyPrepare]     567 → @2637
          [SoftBodyCollide]            → @2718
          [SoftBodySimulate]           → @2738
          [SoftBodyFinalize]           → @2778
                       │
                       ▼
          [StartNextStep] 442  (kick 下一个 collision step)
```

> 想观察单帧执行实际顺序，把这些 `JobXxx` 全部下条件断点 `step_idx == 0 && ioStep->mIsFirst`，再看命中顺序。如果某个 Job 永远不进，去看上一阶段的 `RemoveDependency()` 是否走到（依赖计数初始值见上图括号末尾的常量，例如 `mUpdateBroadphaseFinalize` 是 `num_find_collisions_jobs + 2`，见 PhysicsSystem.cpp:288）。

### 2.2 一帧的全局共享数据：`PhysicsUpdateContext`

文件：`Jolt/Physics/PhysicsUpdateContext.h`

每一帧 `PhysicsSystem::Update` 在栈上建一个 `PhysicsUpdateContext context;`，作为所有 Job 的"参数总线"。调试时把它加入 watch，能看到：

| 字段 | 用途 / 调试观察点 |
|------|-------------------|
| `mSteps[i]` (`Step` 结构) | 一个 collision step 的所有 Job handle + 共享 atomic 计数器 |
| `Step::mNumActiveBodiesAtStepStart` | 本帧应用重力的体数（外部活跃的不算） |
| `Step::mBodyPairQueues[thread]` | broad → narrow 的 body pair 环形队列 |
| `Step::mActiveFindCollisionJobs` | bitmask，哪些 find-collision Job 仍在跑；卡死先看它 |
| `Step::mCCDBodies / mNumCCDBodies` | 本帧待 CCD 的 body 列表 |
| `Step::mSolveVelocityConstraintsNextIsland` | solver 当前消化到第几个 island |
| `mActiveConstraints[]` | 帧首活跃约束快照（DetermineActiveConstraints 写入） |
| `mBodyPairs[]` | broadphase 输出的 pair 池（环形） |
| `mIslandBuilder` | 指向 `PhysicsSystem::mIslandBuilder` |
| `mErrors` | `EPhysicsUpdateError` 位标志：BodyPairsBufferFull / ContactConstraintsBufferFull / ManifoldsBufferFull / StepListeners… |

> 跑完 Update 后，`mErrors != None` 通常意味着 `mMaxInFlightBodyPairs / mMaxBodyPairs / mMaxContactConstraints` 配小了，需要在 `PhysicsSystem::Init` 处调大或者按 §6 跟。

---

## 3. PhysicsSystem 自身

### 3.1 配置入口

- `PhysicsSystem::Init` (`PhysicsSystem.cpp:78`)
  - `inMaxBodies / inNumBodyMutexes / inMaxBodyPairs / inMaxContactConstraints`：这四个上限会在游戏运行时挤爆 → 看 `mErrors`。
  - `BroadPhaseLayerInterface / ObjectVsBroadPhaseLayerFilter / ObjectLayerPairFilter`：碰撞过滤注入点。Layer 不对，所有碰撞都"消失"，断点先打这里看注册的层数。
- `OptimizeBroadPhase` (`:109`)：从 cold start 切到运行态时调用一次，重排 QuadTree 以减少后续 widen。
- `PhysicsSettings`：所有"为什么物理表现是这样"的常量都在 `Jolt/Physics/PhysicsSettings.h`。常用：
  - `mPenetrationSlop`（默认 0.02m）
  - `mNumVelocitySteps / mNumPositionSteps`（默认 10 / 2）
  - `mBaumgarte`（位置纠偏量 0.2）
  - `mSpeculativeContactDistance / mLinearCastThreshold / mLinearCastMaxPenetration`（CCD 阈值）
  - `mTimeBeforeSleep / mPointVelocitySleepThreshold`（睡眠判定）
  - `mAllowSleeping、mDeterministicSimulation` (后者会强制单线程顺序)

### 3.2 关键私有/状态字段（在 `PhysicsSystem.h`）

- `mBodyManager` —— 所有 Body 与活跃列表
- `mBroadPhase` —— 默认为 `BroadPhaseQuadTree`
- `mConstraintManager` —— 非接触约束
- `mContactManager`（`ContactConstraintManager`）—— 接触约束 + manifold cache
- `mIslandBuilder / mLargeIslandSplitter` —— 岛划分
- `mStepListeners` —— `PhysicsStepListener*` 列表
- `mPreviousStepDeltaTime / warm_start_impulse_ratio`（PhysicsSystem.cpp:213-215）—— 控制 warm start impulse 缩放，dt 突变时这里会震

---

## 4. 模块详细调试切入点

### 4.1 BodyManager / Body / BodyID

文件：`Jolt/Physics/Body/BodyManager.{h,cpp}`、`Body/Body.{h,cpp,inl}`、`Body/BodyID.h`

- `BodyID` 是一个 32-bit：低 24 位 index，高 7 位 sequence number，最高位 `cBroadPhaseBit`。
  - watch 表达式：`((id.mID) & 0xFFFFFF)` = index，`((id.mID) >> 24) & 0x7F` = seq。
  - `BodyManager::TryGetBody`（`BodyManager.h:151`）演示了"通过 ID 安全拿 Body"的写法：先取 index，再比对 seq，匹配才返回。**Body 被销毁后 seq 会 +1**，所以悬空 ID 自然失效。
- `mBodies` 是 `BodyVector`，里面可能存"freed 标记"指针，用 `sIsValidBodyPointer`（`BodyManager.h:133`）判断。Crash 时如果 `GetBody(id)` 拿到的 body 指针低位是 1，那是已经释放的槽位。
- 活跃列表：`mActiveBodies[EBodyType::RigidBody / SoftBody]`，通过 `GetActiveBodiesUnsafe / GetNumActiveBodies` 取。
  - `ActivateBodies / DeactivateBodies`（`BodyManager.h:107-111`）必须在持锁状态下调。怀疑"死掉的物体不动"先在这下断。
- Body 互斥锁：`MutexArray`，按 `BodyID.index % N` 散列；外部不要直接 `mBodies[i]`，而要走 `BodyLockRead / BodyLockWrite`（`Body/BodyLock.h`）。
  - 排查死锁顺序：永远是 **先锁 Body → 再锁 BroadPhase**（见 PhysicsSystem.cpp:294 的注释），反过来必死锁。
- `Body::IsActive() / IsInBroadPhase() / IsRigidBody() / IsDynamic()` 都是 inline bit 测试，断点要打到调用方。
- 静态/运动学/动态：`MotionType` 在 `Body/MotionType.h`，`MotionQuality` 在 `Body/MotionQuality.h`（`Discrete` / `LinearCast` → CCD）。
- `MotionProperties`（`Body/MotionProperties.{h,inl}`）：存 linear/angular velocity、惯性张量逆、阻尼、`mIndexInActiveBodies`（活跃数组里位置，sleep 时变 `cInactiveIndex`）。
  - `ApplyForceTorqueAndDragInternal`（`PhysicsSystem.cpp:783` 调用） → `MotionProperties::ApplyForceTorqueAndDragInternal`，是每帧动力学更新点。
  - `ApplyGyroscopicForceInternal` 同样在 ApplyGravity 阶段调用。
- `Body::AddForce / AddImpulse` 仅是把值累加到 `mMotionProperties->mForce / mTorque`，真正消化点在 `ApplyForceTorqueAndDragInternal`。一旦 `Body` 被 Deactivate，残留的 force 不会自动清零；调试"sleep 后施力没生效"看这里。

### 4.2 BroadPhase / QuadTree

文件：`Jolt/Physics/Collision/BroadPhase/*`

- 默认实现：`BroadPhaseQuadTree`，构造一组 `QuadTree`（每个 `BroadPhaseLayer` 一棵）。
- `QuadTree`（`QuadTree.{h,cpp}`）核心特点：
  - 无锁查询（`UpdatePrepare/Finalize` 之间 swap 新旧 root）。
  - 节点 ID：`NodeID`（`QuadTree.h:30`），最高位区分 body / node：
    - `IsBody()` → low bits = `BodyID.GetIndexAndSequenceNumber()`
    - `IsNode()` → low 31 = 节点池索引
  - 修改对象的 AABB 时会**沿父链 widen**，不会立即重建；重建在 `UpdatePrepare`（PhysicsSystem.cpp:299）。
- 调试两类常见问题：
  1. "AddBody 之后 raycast 不到"：`PhysicsSystem::OptimizeBroadPhase` 没被调用？或者 body 加完后还没经过一帧 Update（树还没装上）？断点 `QuadTree::AddBodiesPrepare/Finalize`、`BroadPhaseQuadTree::AddBodiesFinalize`。
  2. "树越来越胖、性能掉"：长时间不 OptimizeBroadPhase，widen 累积。可以打开 `JPH_DUMP_BROADPHASE_TREE`（`QuadTree.h:13`）让它把树 dump 出来。
- BroadPhase 与 Layer 过滤：`ObjectVsBroadPhaseLayerFilter::ShouldCollide` 与 `ObjectLayerPairFilter::ShouldCollide` 是**所有**碰撞过滤的第一道门，碰撞应该有但没有，**先**在这两个函数入口下条件断点。

### 4.3 Narrow phase / Find Collisions

入口：`PhysicsSystem::JobFindCollisions`（`PhysicsSystem.cpp:890`），多线程并发跑。
逻辑：从 active body 列表中拉一批 → BroadPhase 查重叠 → 把 pair 塞到 `Step::mBodyPairQueues` → 消费 pair 调 `ProcessBodyPair`（`:1046`）。

- `ProcessBodyPair`：
  - 先 `mContactManager.GetContactsFromCache`（`ContactConstraintManager.cpp:795`）拿上一帧缓存，命中就跳过 narrow。
  - 不命中则 `CollisionDispatch::sCollideShapeVsShape`，最终调 `Shape::CollideShape` 系列虚函数 → 走 GJK/EPA 或专用算法。
  - 输出 `CollideShapeResult` → `ManifoldBetweenTwoFaces`（`Jolt/Physics/Collision/ManifoldBetweenTwoFaces.cpp`）把两个支撑面裁剪成 ≤ 4 个接触点 → `ContactConstraintManager::AddContactConstraint`（`ContactConstraintManager.cpp:1272`）。
- `TrySpawnJobFindCollisions`（`PhysicsSystem.cpp:827`）：根据当前 pending 数动态再 spawn。`mActiveFindCollisionJobs` 是 bitmask，每个 bit 一个 worker，调试 "活越来越少没结束" 看这里。
- 调试碰撞回调：`ContactListener` 接口在 `Collision/ContactListener.h`。`OnContactValidate / OnContactAdded / OnContactPersisted / OnContactRemoved` 是用户能挂的钩子。注意 OnContactAdded/Persisted 在 narrow phase 线程内调用，**不在主线程**。

### 4.4 Shape 体系

文件：`Jolt/Physics/Collision/Shape/*`

- 基类：`Shape`（`Shape.h:185`），分 `EShapeType`（Convex/Compound/Decorated/Mesh/HeightField/SoftBody/Plane/Empty）与 `EShapeSubType`（具体形状）。
- 工厂表：`ShapeFunctions::sRegistry`（`Shape.h:181`），在 `RegisterTypes.cpp` 里填。注册不全 → 反序列化崩。
- 形状速查：

| Shape | 文件 | 调试关注点 |
|-------|------|----------|
| Sphere | SphereShape.* | inner radius、GetSupport |
| Box | BoxShape.* | convex radius 决定圆角 |
| Capsule | CapsuleShape.* | 主轴沿 Y |
| Cylinder | CylinderShape.* | 端面接触可能不稳定 → InternalEdgeRemovingCollector |
| TaperedCapsule / TaperedCylinder | 同名文件 | 见 .gliffy 图 |
| ConvexHull | ConvexHullShape.{h,cpp}（4w 行级） | 顶点合法性、面是凸的、`PhysicsMaterial` 索引 |
| Triangle | TriangleShape.* | 单三角，主要给 Mesh 内部用 |
| Mesh | MeshShape.{h,cpp}（5w 行） | 内含 BVH；ray/cast 走 visitor 模式 |
| HeightField | HeightFieldShape.{h,cpp}（10w 行） | 量化数据；GetMaterial 通过 sample 找 |
| StaticCompound | StaticCompoundShape.* | 子 shape AABB tree |
| MutableCompound | MutableCompoundShape.* | 可运行时增删 |
| Decorated | DecoratedShape.* | 装饰器基类 |
| Scaled / RotatedTranslated / OffsetCenterOfMass | 同名文件 | 嵌套时调试链路最长 |
| Plane | PlaneShape.* | 半空间，仅静态 |
| Empty | EmptyShape.* | 占位 |

- `SubShapeID`（`Shape/SubShapeID.h`）：从碰撞结果回查"打中复合形状的哪一片"。读取时要把 ID 一层一层 "pop"（按各 Shape 的 `GetSubShapeIDBitsRecursive` 消耗位数），CompoundShape 的实现是好例子。
- 调 mesh / heightfield 碰撞，注意 `ActiveEdgeMode / CollectFacesMode`（`Collision/ActiveEdgeMode.h`、`ActiveEdges.h`），用于抑制内边引发的"鬼跳"。`InternalEdgeRemovingCollector`（`Collision/InternalEdgeRemovingCollector.h`）是 1.x 后引入的更鲁棒的方案。

### 4.5 GJK / EPA

文件：`Jolt/Geometry/GJKClosestPoint.h`、`EPAPenetrationDepth.h`、`EPAConvexHullBuilder.h`、`ClosestPoint.h`

- 入口宏：`JPH_ENABLE_GJK_STATS / JPH_ENABLE_EPA_STATS / JPH_EPA_PENETRATION_DEPTH_DEBUG`（搜索头部宏开关）会插额外 `JPH_TRACE` 日志。
- GJK 不收敛的迹象：迭代到 `cMaxIterations` 仍未停。在 `GJKClosestPoint::GetClosestPoints` 的 for 循环里下断，监 `m_Y / m_P / m_Q` 的 simplex 状态。
- EPA 失败：法线方向退化、convex hull 构建失败。`EPAConvexHullBuilder.h` 里有 `JPH_EPA_CONVEX_BUILDER_DUMP` / `JPH_EPA_CONVEX_BUILDER_VALIDATE` 等调试宏。

### 4.6 IslandBuilder

文件：`Jolt/Physics/IslandBuilder.{h,cpp}`

调用时序：

1. `PrepareContactConstraints` —— 每帧开始（PhysicsSystem.cpp:341 / 465）
2. `PrepareNonContactConstraints` —— BuildIslandsFromConstraints 内（PhysicsSystem.cpp:821）
3. `LinkBodies / LinkConstraint / LinkContact` —— 多线程并行调，所有 link 都用 `mBodyLinks[i].mLinkedTo` atomic 做 union-find（路径压缩）。
4. `Finalize` —— FinalizeIslands Job 调（PhysicsSystem.cpp:1364）→ 内部按 lowest body index 给每个 body 分配 `mIslandIndex`，再 bucket sort 出 `mBodyIslands / mConstraintIslands / mContactIslands` 和它们的 `*Ends[]`。
5. solver 通过 `GetBodiesInIsland / GetConstraintsInIsland / GetContactsInIsland`（`IslandBuilder.h:49-51`）拿到第 i 个岛的范围。

调试技巧：

- 打开 `JPH_VALIDATE_ISLAND_BUILDER`（`IslandBuilder.h:15`），会启用 `mLinkValidation`，每次 Finalize 时校验所有 link 是否真的在同一岛内。
- 看每岛大小：watch `mBodyIslandEnds[i] - (i==0?0:mBodyIslandEnds[i-1])`。
- 大岛会被 `LargeIslandSplitter`（`LargeIslandSplitter.{h,cpp}`）切成 batch 给多线程 solver；性能调试 / 不确定行为时先关掉它对比（直接早 return 看是否复现）。

### 4.7 ContactConstraintManager（最复杂的一档）

文件：`Jolt/Physics/Constraints/ContactConstraintManager.{h,cpp}`，**72k 字节**，包含：

- **Manifold Cache**（`ManifoldCache`，`:250` 起）：上一帧 body-pair / manifold 哈希表，用于 warm start 和 OnContactPersisted 判定。哈希基于 `BodyPair`（按 ID 排序）+ `SubShapeIDPair`。`Prepare/Finalize/Clear/SaveState/RestoreState` 都在这里。
- **PrepareConstraintBuffer**（`:698`）—— 每帧开头分配本帧的接触约束池。`mErrors |= ContactConstraintsBufferFull` 由这里抛。
- **GetContactsFromCache**（`:795`）—— 接触缓存命中流程，决定是否跳过 narrow。
- **AddContactConstraint**（`:1272`）—— narrow phase 产出 manifold 后真正写入。内部按 body 配对组合 `MotionType` 分派到 `TemplatedAddContactConstraint`（`:1044`，按模板特化优化数学）。`OnContactValidate / OnContactAdded / OnContactPersisted` 在这两个函数里被调。
- **WarmStartVelocityConstraints**（`:1542`）—— 用上一帧 impulse × `mWarmStartImpulseRatio` 预热。`mWarmStartImpulseRatio = dt_now / dt_prev`，**dt 跳变会让物体"跳"**，调试震动看 `PhysicsSystem.cpp:214`。
- **SolveVelocityConstraints**（`:1643`）—— 每个 solver 迭代调 1 次，内层用 `AxisConstraintPart`（接触法线 + 两条摩擦切向）做 Sequential Impulses。
- **SolvePositionConstraints**（`:1721`）—— Baumgarte 位置纠偏（split-impulse）。
- **OnCCDContactAdded**（`:1366`）—— 给 CCD 路径的接触特别处理。
- **FinalizeContactCacheAndCallContactPointRemovedCallbacks**（`:1475`）—— 帧尾扫缓存，没续上的 manifold 触发 `OnContactRemoved` 回调，注意此函数**仅在主线程**调，回调里再加/删 body 在这里是安全的。
- **绘制开关（运行时 bool）**（`:26-29`）：
  - `sDrawContactPoint`、`sDrawSupportingFaces`、`sDrawContactPointReduction`、`sDrawContactManifolds`，直接在调试器里改 true 就能可视化。

### 4.8 非接触约束 / ConstraintPart

文件：`Jolt/Physics/Constraints/*Constraint.{h,cpp}` 与 `Constraints/ConstraintPart/*`

- 每种 Constraint = 若干 `*ConstraintPart` 复合：
  - `AxisConstraintPart`（最常用，单方向 1D 阻塞）
  - `PointConstraintPart`（3D 位置等式）
  - `AngleConstraintPart` / `HingeRotationConstraintPart` / `RotationEulerConstraintPart` / `RotationQuatConstraintPart`
  - `SwingTwistConstraintPart`、`DualAxisConstraintPart`、`IndependentAxisConstraintPart`
  - `SpringPart`（与 AxisConstraintPart 组合实现 soft constraint）
  - `GearConstraintPart` / `RackAndPinionConstraintPart`
- 每个 Part 三段套路：
  1. `CalculateConstraintProperties` —— 算雅可比、有效质量。
  2. `WarmStart` —— 把上一帧 lambda 转成 impulse 应用。
  3. `SolveVelocityConstraint` / `SolvePositionConstraint` —— 单次迭代。
- 约束行为不对，直接在对应 `*Constraint::SolveVelocityConstraint` 入口断点，watch `mLambda`、`mTotalLambda`、`mEffectiveMass`。
- `ConstraintManager`（`ConstraintManager.{h,cpp}`）维护全局 `mConstraints` 数组。`sBuildIslands` / `sSetupVelocityConstraints` / `sWarmStartVelocityConstraints` / `sSolveVelocityConstraints` / `sSolvePositionConstraints` 是静态批处理入口，PhysicsSystem 直接调它们。
- 自适应迭代步数：`CalculateSolverSteps.h`，按岛聚合每个约束 `GetNumVelocityStepsOverride / GetNumPositionStepsOverride` 取 max（每个约束可单独覆盖默认值）。

### 4.9 Solver Loop（Velocity + Position）

文件：`PhysicsSystem.cpp` 中 `JobSolveVelocityConstraints`（`:1399`）/ `JobSolvePositionConstraints`（`:2493`）

每个 worker：

1. 从 `Step::mSolveVelocityConstraintsNextIsland.fetch_add(1)` 拿下一个岛。
2. 若是大岛 → 转交 `LargeIslandSplitter`（拆 batch）。
3. 否则按 `mNumVelocitySteps[island]` 次：
   - `ContactConstraintManager::WarmStartVelocityConstraints`（第一次迭代）
   - `ConstraintManager::sSolveVelocityConstraints`
   - `ContactConstraintManager::SolveVelocityConstraints`
4. position 阶段类似，按 `mNumPositionSteps[island]` 次。

调试 "穿透 / 抖动 / 链条松弛"：

- 调高 `mNumVelocitySteps` 看是否收敛；不收敛就是 LCP 病态（质量比悬殊 / 长链）。
- 在 `*::SolveVelocityConstraint` 里 watch `lambda` 累积值看是否 clamp 频繁。

### 4.10 Integrate / CCD

- `JobIntegrateVelocity`（`PhysicsSystem.cpp:1602`）：积分位置 + 角度。对 `MotionQuality::LinearCast` 的 body 检查 `delta_pos² >= mLinearCastThresholdSq`，是则塞进 `Step::mCCDBodies`。
- `JobFindCCDContacts`（`:1801`）：对每个 CCD body 做 ShapeCast → 找最早接触 → 写 `mFraction / mFractionPlusSlop / mContactNormal / mContactPointOn2`。
- `JobResolveCCDContacts`（`:2102`）：回滚位置到 `mFractionPlusSlop`，调用 `ContactConstraintManager::OnCCDContactAdded` 注册接触；再次 solve。
- CCD 失效场景：`mFraction` 始终 1（cast 没命中）/ body 不是 LinearCast / `mLinearCastThreshold` 配太大。

### 4.11 Sleeping

- 入口：`PhysicsSystem::CheckSleepAndUpdateBounds`（`:2438`），在 SolvePositionConstraints 末尾按岛触发。
- 判定使用 `Body::UpdateSleepStateInternal`（`Body.cpp` 内），核心：维护 `MotionProperties::mSleepTestSpheres[3]`，三个体上的"参考点"位移连续 `mTimeBeforeSleep` 没超过 `mPointVelocitySleepThreshold`，整岛一起睡。
- 调试 "永远不睡 / 不该睡时睡了":
  - 在 `Body::ResetSleepTimer / AddPositionStep` 处下断（一个动作就重置计时）。
  - watch `mSleepTestTimer` 与 `mSleepTestSpheres[i].mCenter`。

### 4.12 软体（SoftBody）

文件：`Jolt/Physics/SoftBody/*`

入口 Job 已在 §2 图末尾。`SoftBodyMotionProperties` 里维护 vertices/edges/volumes/skin/rod 等约束数组。
要调试软体可视化：`SoftBodyMotionProperties::sDrawXxx` 静态 bool（在头文件搜 `sDraw`），调试器里改为 true。

---

## 5. JobSystem & 锁（卡死先看这一节）

文件：`Jolt/Core/JobSystem*.{h,cpp,inl}`，`Physics/PhysicsLock.h`

- `JobHandle` 内部就是 `Job *`（refcount + dependency counter）。`RemoveDependency()` 把计数减 1；到 0 才入 ready 队列。
- 卡死排查步骤：
  1. 把所有线程 break，逐 worker 看栈顶 → 大概率卡在 `Semaphore::Wait` 或某个 `Mutex::lock`。
  2. 查阻塞的 mutex 是 Body mutex 还是 BroadPhase 锁。Body mutex 数组在 `BodyManager::mBodyMutexes`，BroadPhase 锁见 `BroadPhase::LockModifications`。
  3. 确认调用顺序：**Body 锁先**于 **BroadPhase 锁**（PhysicsSystem.cpp:294 注释）。外部业务代码若反着调（先 ChangeBroadPhase 再 LockBody）就会死锁。
  4. `JobSystem::WaitForJobs` 在 PhysicsSystem.cpp:625，所有 barrier 都加进去了。Barrier 没等到 → 说明某个 Job 创建后 `mNumDependencies` 永远没到 0，回 §2 检查是哪条边没 RemoveDependency。
- `PhysicsLock`（`PhysicsLock.h`）封装 enable-asserts 版本的锁，能告诉你哪段代码以哪种顺序拿了锁。打开 `JPH_ENABLE_ASSERTS` 编译时调试效果最好。

---

## 6. 症状 → 断点速查表

| 现象 | 第一断点 | 重点观察 |
|------|---------|---------|
| AddBody 后无任何碰撞、ray cast 不到 | `BroadPhaseQuadTree::AddBodiesFinalize` / `ObjectVsBroadPhaseLayerFilter::ShouldCollide` | Layer 注册是否齐；是否过了一帧 Update |
| 碰撞回调不调用 | `ContactConstraintManager::AddContactConstraint`（`:1272`） | manifold 是否有点；`OnContactValidate` 是否 reject |
| 碰撞但物体穿透 | `JobSolveVelocityConstraints` + `JobSolvePositionConstraints` | `mNumVelocitySteps/PositionSteps`、`mPenetrationSlop`、`mBaumgarte`；质量比 |
| 高速物体穿墙 | `JobIntegrateVelocity`（`:1602`）入口 | body 是否 `MotionQuality::LinearCast`；`mLinearCastThreshold(Sq)`；`Step::mNumCCDBodies` 是否 > 0；`JobFindCCDContacts` 是否被 spawn |
| 长链 / 多约束抖动 | `*Constraint::SolveVelocityConstraint` | lambda 振荡、warm start impulse 是否合理；试增 velocity steps |
| dt 突变导致跳 | `PhysicsSystem.cpp:214`（`warm_start_impulse_ratio`） | 看 ratio 值；考虑禁用 warm start |
| 物体不睡眠 | `CheckSleepAndUpdateBounds`（`:2438`），`Body::UpdateSleepStateInternal` | `mSleepTestTimer / mSleepTestSpheres` 移动量、`mPointVelocitySleepThreshold`、`mTimeBeforeSleep` |
| 物体过早睡 | 同上 | 业务侧是否每帧 `ApplyForce` 但未 `ActivateBody`；force 在睡眠时被清 |
| `EPhysicsUpdateError::BodyPairsBufferFull` | `JobFindCollisions` 内 push 处；`PhysicsSystem::Init` | 调大 `inMaxBodyPairs / mMaxInFlightBodyPairs` |
| `ContactConstraintsBufferFull` / `ManifoldsBufferFull` | `ContactConstraintManager::AddContactConstraint`、`PrepareConstraintBuffer` | 调大 `inMaxContactConstraints` |
| 死锁、`WaitForJobs` 不返回 | `JobSystem::WaitForJobs`、各 `JobXxx` 末尾的 `RemoveDependency()` | 是否漏减依赖；外部锁顺序 |
| 重放不确定 | `PhysicsSettings::mDeterministicSimulation = true`；`JPH_DET_LOG` | 打开 DeterminismLog（`DeterminismLog.h`），对比两次 trace |
| GJK / EPA 退化 | `GJKClosestPoint::GetClosestPoints`、`EPAPenetrationDepth::GetPenetrationDepth` | simplex 状态、convex radius |
| Compound / Mesh 内边跳 | `InternalEdgeRemovingCollector::OnContact*`、`ActiveEdges.h` | `ActiveEdgeMode`、`mActiveEdgeMovementDirection` |
| 反序列化 / 工厂崩 | `ShapeFunctions::sRegistry`（`Shape.h:181`），`RegisterTypes.cpp` | 是否调了 `RegisterDefaultAllocator() + Factory::sInstance + RegisterTypes()` |

---

## 7. 工具与可视化

### 7.1 Profiler

`Jolt/Core/Profiler.{h,inl,cpp}`：
- 宏 `JPH_PROFILE_FUNCTION()`、`JPH_PROFILE("name")`，需要编译时打开 `JPH_PROFILE_ENABLED`。
- `Profiler::sInstance->StartFrame()` / `EndFrame()` 在 PhysicsSystem 内部已加。
- 用 `Profiler::sInstance->Dump("frame.txt")` 把一帧时间线导出（chrome://tracing 兼容格式）。

### 7.2 DeterminismLog

`DeterminismLog.{h,cpp}` + 宏 `JPH_DET_LOG(x)`（已在 PhysicsSystem.cpp:178 用过）。
- 需要 `JPH_ENABLE_DETERMINISM_LOG` 才生效。
- 不确定性排查：两次跑同样输入，导出两份 log diff，第一处不同就是发散点。

### 7.3 StateRecorder

`Physics/StateRecorder.h` + `StateRecorderImpl.{h,cpp}`：
- `PhysicsSystem::SaveState / RestoreState`（`PhysicsSystem.cpp:2835`）。
- 复现现场神器：游戏卡死前一帧 SaveState，单测里 RestoreState 然后单步 Update。
- `EStateRecorderState` 位标志决定保哪些（Bodies / Constraints / Contacts / Global）。

### 7.4 .natvis

`Jolt/Jolt.natvis` 已配置了：
- `Vec3 / Vec4 / Quat / Mat44 / DMat44 / RVec3`：展示 x/y/z 浮点而不是 `__m128`。
- `BodyID`：展示 `Index/Sequence/IsInvalid`。
- `Color`、`Array<T>`、`StaticArray<T,N>`、`Ref<T>`：友好展开。
- `SubShapeID`：可拆位段。
- 在 VS 中 `Tools → Options → Debugging → 自动加载 natvis` 默认就够；如果是 CMake out-of-source 编译，确保 `Jolt.natvis` 进了项目。

### 7.5 调试绘制

需要 `JPH_DEBUG_RENDERER` 宏 + 自行实现 `DebugRenderer` 子类。
- `BodyManager::Draw(DrawSettings, ...)`（`BodyManager.h:272`）：体本身（shape / bounds / COM / velocity / mass / sleep）。
- `ContactConstraintManager::sDrawXxx`（§4.7）：接触点 / manifold。
- `Shape::sDrawSubmergedVolumes`、`CharacterVirtual::sDrawConstraints` 等运行时 bool（注释见 BodyManager.h:239-242）。
- 软体绘制：`SoftBodyMotionProperties::sDrawXxx` 系列。

---

## 8. 关键编译开关（影响调试体验）

下面这些定义在 `Jolt/Jolt.cmake` / `Core/Core.h` 中，改了要重编 Jolt：

| 宏 | 作用 |
|----|------|
| `JPH_ENABLE_ASSERTS` | 打开所有 `JPH_ASSERT`，**调试必开** |
| `JPH_PROFILE_ENABLED` | 启用 Profiler |
| `JPH_DEBUG_RENDERER` | 启用 `DrawXxx` 接口与 `sDraw*` 静态开关 |
| `JPH_ENABLE_DETERMINISM_LOG` | 启用 `JPH_DET_LOG` |
| `JPH_DOUBLE_PRECISION` | 把世界坐标改用 double（`RVec3 / DMat44`）。大世界场景需要 |
| `JPH_TRACK_NARROWPHASE_STATS` | 在 narrow phase 累统计，可看到每个 Shape×Shape 组合的耗时 |
| `JPH_TRACK_SIMULATION_STATS` | 每岛收集 solver 耗时（IslandBuilder.h:57） |
| `JPH_VALIDATE_ISLAND_BUILDER` | IslandBuilder 验证（IslandBuilder.h:15） |
| `JPH_DUMP_BROADPHASE_TREE` | QuadTree 重建时把整树 dump（QuadTree.h:13） |
| `JPH_ENABLE_GJK_STATS / JPH_EPA_PENETRATION_DEPTH_DEBUG` | GJK/EPA 内部 trace |
| `JPH_FLOATING_POINT_EXCEPTIONS_ENABLED` | 让 NaN/Inf 直接抛硬件异常 |
| `JPH_OBJECT_STREAM` | ObjectStream 序列化（保存场景 / 反序列化形状） |
| `JPH_DISABLE_TEMP_ALLOCATOR` | 强制走 malloc，方便外部检测器（asan）抓越界 |

---

## 9. 首次跟一帧仿真的建议步骤

1. 编译时开 `JPH_ENABLE_ASSERTS + JPH_PROFILE_ENABLED + JPH_DEBUG_RENDERER`，至少前两个。
2. 在以下函数入口下断点：
   - `PhysicsSystem::Update`（`PhysicsSystem.cpp:174`）
   - `PhysicsSystem::JobApplyGravity`（`:743`）
   - `PhysicsSystem::JobFindCollisions`（`:890`）
   - `PhysicsSystem::ProcessBodyPair`（`:1046`）
   - `ContactConstraintManager::AddContactConstraint`（`ContactConstraintManager.cpp:1272`）
   - `PhysicsSystem::JobBuildIslandsFromConstraints`（`:810`）→ `IslandBuilder::Finalize`
   - `PhysicsSystem::JobSolveVelocityConstraints`（`:1399`）
   - `PhysicsSystem::JobIntegrateVelocity`（`:1602`）
   - `PhysicsSystem::JobSolvePositionConstraints`（`:2493`）
3. 准备一个最小用例：1 个大地（静态 BoxShape）+ 1 个动态盒子从空中掉下来。`mNumVelocitySteps = 1` 临时降低让接触不一次性解掉，方便观察。
4. Continue 进各 Job，按 §2 的顺序确认走通；watch `context.mSteps[0]` 一路看 atomic 计数器的变化。
5. 跑 30 帧后把 Profiler dump 出来，用 chrome://tracing 打开，对照 §2 图能直观看到并发。

---

## 10. 在 cat 工程里要怎么找入口

cat 侧封装见 `doc/physic/Jolt接入实施步骤.md` 与 `doc/physic/物理引擎接入设计与实施计划.md`（与本文档同目录）。

调试时如果只关心"我们调用 Jolt 哪里出了问题"，建议双向插桩：

1. cat 侧封装的 `Tick` / `AddBody` / `SetTransform` 等先加 log + `JPH_PROFILE("cat::xxx")`。
2. 用本文 §6 的症状表，沿调用栈下沉到 Jolt 内部。
3. 复现稳定后，用 `PhysicsSystem::SaveState` 把现场存盘，写一个最小 Jolt-only 单测（不依赖 cat 渲染 / ECS）能复现，再交给 §7.3 的工具拆解。

---

## 11. 维护说明

- 上游 Jolt 同步后，所有行号需要重新校对（仅函数名不变）。
- 新增模块（如未来的 Vehicle 调试）请在本文 §4 下新增小节，沿用 "文件 / 入口函数 / 关键字段 / 调试断点" 的四段式写法。
- 不要在本文档塞物理学公式或入门概念，那是另一篇文档的事。
