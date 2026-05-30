# cat 项目代码审查 — Shader 维度

> 子 agent：Shader 专家
> 项目类型：自研 C++ Vulkan/GLES 渲染引擎（非 UE）
> Shader 语言：GLSL / Vulkan GLSL
> 目标平台：移动端（Adreno / Mali GLES 3.x）+ PC Vulkan
> 性能目标：1080p / 60fps

---

## 第一节：Shader 本体审查（10 个文件）

### 1.1 OpenGL ES：`color.vert` / `color.frag`

#### Suggestion
- [Shader] `testCat/shader/opengles/color.vert:12` — `vertex_color.rgba = color.bgra;` 在顶点着色器里做颜色通道重排。如果是为了适配 BGRA 顶点 buffer 格式，建议在 C++ 端上传顶点时就规范化为 RGBA，避免每个 vertex 多 4 个 swizzle 指令；如果只是历史习惯，应该直接 `vertex_color = color;`。

#### Suggestion
- [Shader] `testCat/shader/opengles/color.vert:1` — 文件头有大量 tab/空格尾部空白（全文件都有），不影响功能但建议清理；同时 `#version 300 es` 后面跟一长串 tab 不规范，将来用某些 GLSL preprocessor / shader minifier 时可能出错。

#### Suggestion
- [Shader] `testCat/shader/opengles/color.vert:11` — `position` 已经声明为 `vec4`，但又写 `vec4(position.xyz, 1.0)`，等价于强制把 `w` 置 1，丢弃 attribute 的 `w`。如果 C++ 端确实只上传 xyz（w 未定义），写法 OK；如果 buffer 是完整 vec4，直接 `mvp * position` 即可，少一次构造。`default*.vert` 全系列同样问题。

---

### 1.2 OpenGL ES：`default.vert` / `default.frag`（ES 3.00）

#### Critical
- [Shader] `testCat/shader/opengles/default.vert:18` 与 `default.frag` 整体 — VS 输出 `vertex_color` 并赋值 `vec4(1,1,1,1)`，但 `default.frag` 完全未使用该 varying，**整个 `vertex_color` 链路是死代码**。在 GLES 3.0 上，未使用的 out → in 链不会被链接器消掉（必须前后匹配），这会浪费 4 个 float 的 varying 带宽（每像素插值开销）并增加链接限制。**建议直接删除 `vertex_color` 的声明和写入**。

#### Warning
- [Shader] `testCat/shader/opengles/default.vert:13-15` — `joints` 属性用 `vec4` + `int(joints.x)` 做下标转换。移动端骨骼数据应该用 **整型 attribute (`in uvec4 joints`)** 上传：① 避免 float→int 的精度截断（>16M 的 joint id 会出错，虽然实际不会发生）；② 节省每个顶点 4 次类型转换；③ 与 `object.vert` 已经使用 `uvec4 i_joints` 保持一致。

#### Warning
- [Shader] `testCat/shader/opengles/default.vert:15` — `uniform mat4 joint_matrices[50]` 占 50×16 = 800 float = 200 个 vec4。GLES 3.0 标准只保证 `MAX_VERTEX_UNIFORM_VECTORS ≥ 256`，加上 `mvp` 和其他，已经接近下限。在 Mali-G31/Adreno 5xx 等老移动 GPU 上可能触顶导致编译失败或退到 indirect uniform 路径。建议改成 UBO（GLES 3.0 已支持 `uniform Block`）或 SSBO（ES 3.1）。Vulkan 端 `object.vert` 已经用 UBO，应该统一。

#### Warning
- [Shader] `testCat/shader/opengles/default.vert:24-27` — Skin 路径在 VS 内构造一个完整 `mat4`，然后 `mvp * skinMatrix * pos`。先算 `skinMatrix * vec4(pos.xyz,1.0)`（mat4 × vec4 = 16 mul + 12 add）再 `mvp * result` 即可；当前写法编译器多半会优化掉，但显式写更安全。**更高效的做法**：让每根骨骼存 mat4x3（GLES 3.0 支持），变换路径变成 12 mul + 9 add 而不是 16+12。

#### Warning
- [Shader] `testCat/shader/opengles/default.vert` vs `default_30.vert` — **顶点属性 location 不一致**。`default.vert`：loc1=normal(vec3), loc2=tangent(vec3), loc3=coord(vec2)；`default_30.vert`：loc1=color(vec4), loc2=coord(vec2)。如果 C++ 端用同一份 VAO 绑定两个 shader，**会得到完全错误的几何/纹理**。注释 `defined in primitive.cpp function _attrNameToIndex` 提示是按 name → location 匹配，但 GLSL 这里写了 `layout(location = N)` 是**显式覆盖**，name 匹配规则被忽略。需要确认 C++ 侧到底以哪个为准。

#### Suggestion
- [Shader] `testCat/shader/opengles/default.frag:3` — 整文件只有 `precision mediump float`；`sampler2D` 在 GLES 中默认 `lowp`（实际由实现决定），如果纹理是 sRGB color 还好，但**任何依赖纹理值做光照计算**的扩展都会精度不足。建议显式：`precision mediump sampler2D;` 或 `precision highp sampler2D;`（取决于用途）。

#### Suggestion
- [Shader] `testCat/shader/opengles/default.frag:7` — `uniform sampler2D tex;` 没有 binding 注解。GLES 3.0 已支持 `layout(binding=0) uniform sampler2D tex;`，可以避免 C++ 端再调用 `glUniform1i` 设置 sampler unit。

---

### 1.3 OpenGL ES：`default10.vert` / `default10.frag`（ES 2.0 风格）

#### Critical
- [Shader] `testCat/shader/opengles/default10.vert:1` — 文件首行是空行，**没有 `#version` 声明**。GLSL ES 编译器默认按 ES 1.00 处理，正好对应 `attribute`/`varying` 语法。但很多驱动（特别是 Mali）会严格要求显式 `#version 100`，建议补上以避免某些设备拒绝编译。`default10.frag` 同样问题。

#### Critical
- [Shader] `testCat/shader/opengles/default10.vert:11` — ES 1.00 中 `uniform mat4 joint_matrices[50]` 的 **动态索引访问**（`joint_matrices[int(joints.x)]`）只在 vertex shader 中支持「constant-index-expression」是 ES 1.00 的硬性约束。多数桌面/移动驱动会放宽，但**部分老 Mali 驱动**仍按规范拒绝编译。如果项目仍需要支持 ES 2.0 设备，建议显式 `#extension GL_EXT_uniform_buffer_object` 或上线时确认目标设备列表。

#### Warning
- [Shader] `testCat/shader/opengles/default10.vert` — 没有 `precision` 声明。ES 1.00 vertex shader 默认 float = highp，OK；但 `mat4 joint_matrices[50]` 没有显式精度，部分驱动可能把它当 mediump，**蒙皮变换在远处会出现 jitter**。建议显式 `uniform highp mat4 joint_matrices[50];`。

#### Warning
- [Shader] `testCat/shader/opengles/default10.frag:2` — 整文件 `precision mediump float` 唯一一行精度声明。`gl_FragColor` 默认无精度，但 `texture2D` 返回 mediump sampler 的结果。如果将来扩展加光照，**世界坐标重建会丢精度**。当前只采纹理无影响。

#### Warning
- [Shader] `testCat/shader/opengles/default10.vert:33` 与 `default10.frag:9` — VS 计算 `vertex_color = color.bgra` 并 varying，FS 完全未使用。**死 varying 链路**，浪费一个 vec4 的插值带宽（移动端 fragment 速率敏感）。

---

### 1.4 OpenGL ES：`default_30.vert` / `default_30.frag`（ES 3.00）

#### Critical
- [Shader] `testCat/shader/opengles/default_30.vert:1` — 首行是注释 `//#version 300 es` —— **`#version` 被注释掉了！** 但文件使用了 `layout(location=...)`、`in`/`out` 等 ES 3.00 语法。如果 C++ 加载时**没有在 source 前注入 `#version 300 es`**，会按默认 ES 1.00 编译失败。需要检查 `shader_gles.cpp` 是否做了 version 注入；如果没有，**这两个文件根本无法运行**。`default_30.frag:1` 直接没有 version 字符串，同样问题。

#### Critical
- [Shader] `testCat/shader/opengles/default_30.vert` vs `default.vert` — 两份名称暗示同样功能（默认 ES 3.0 shader），但 **attribute layout 与 macro 行为完全不同**：
  - `default.vert`: loc1=normal, loc2=tangent, loc3=coord，无 color attribute
  - `default_30.vert`: loc1=color(vec4)，loc2=coord(vec2)，没有 normal/tangent
  并且 `default_30.vert` 里 `color` 被读出来后**完全没用**，`vertex_color = vec4(1,1,1,1)` 直接覆盖。这是一份**未完成的或半废弃的 shader**，必须明确：到底用哪份？建议删一份或者文件名加上区分意图（`default_color.vert` / `default_lit.vert` 之类）。

#### Warning
- [Shader] `testCat/shader/opengles/default_30.vert:5,32` — `color` attribute 声明了但从未使用，`vertex_color` 直接被赋值 `vec4(1,1,1,1)`。两个浪费：① VBO 仍要绑定 color buffer（C++ 侧浪费）；② 死 varying（FS 也不用 `vertex_color`）。**建议删除 color attribute 与 vertex_color 整链**。

#### Warning
- [Shader] `testCat/shader/opengles/default_30.frag:1` — 没有 `#version`，但用 `in`、`out`、`texture()`（ES 3.00 才有）。同上 critical，依赖加载器注入 version。

---

### 1.5 Vulkan：`object.vert` / `object.frag`

#### Warning
- [Shader] `testCat/shader/vulkan/object.frag:1` — `#version 460` 但**整个文件没有任何 `precision` 声明**。Vulkan GLSL 中精度限定符不是必须，但**编译到 SPIR-V 后驱动可能按 highp 处理所有 float**，在 Mali / Adreno 上 color/UV 这种 mediump 已足够的运算会被错误地用 32-bit 算力。建议加：
  ```glsl
  precision mediump float;
  precision mediump sampler2D;
  ```
  或对个别 highp 的中间量（如世界坐标）单独 `highp` 标注。`object.vert` 同样需要（虽然 vertex 通常 highp 是对的，但 uv 输出可以 mediump）。

#### Warning
- [Shader] `testCat/shader/vulkan/object.vert:55,86` 与 `object.frag:8` — `o_position = gl_Position.xyz` 把 **clip-space 位置** 当 "position" 传给 FS，但 `object.frag` 的 `i_position` **从未使用**。这是 3 个 float 的死 varying + tiler 上额外的 attribute interpolator slot。**直接删除** vert 的 `o_position` 和 frag 的 `i_position`。注意此 varying 是无 `#ifdef` 包裹的，永远存在。

#### Warning
- [Shader] `testCat/shader/vulkan/object.vert:55` — `o_texcoord` 类型是 `vec4`，但 vert 只写 `o_texcoord.xy = i_uv.xy`，`.zw` 未定义；frag 也只读 `i_texcoord.xy`。这浪费 2 个 float 的插值。改为 `vec2 o_texcoord` 即可，**与对应 frag 的 `i_texcoord` 同步**。

#### Warning
- [Shader] `testCat/shader/vulkan/object.vert:75-81` — Skin 计算（同 default.vert 一样的 sum-of-4 mat4 模式）。Vulkan 端这里至少用了 `uvec4 i_joints` 正确，比 GLES 端好。但仍可以用 mat4x3 节省 1/4 的乘法。**与 GLES 端语义保持一致**：检查 C++ 端两套 shader 上传的骨骼矩阵存储格式是否完全一致（行主/列主、变换是否预乘 inverse-bind）。

#### Suggestion
- [Shader] `testCat/shader/vulkan/object.frag:14-19,34-42` — `PICK` 分支让 `o_color = pushConst.pickColor`，**直接忽略 `i_color` 和 texture**。但是 `#ifdef COLOR` 包裹的 `vec4 color = i_color;` 仍然会被计算（被驱动 DCE 掉一般没事），不过：如果 `PICK` 与 `TEXTURE` 同时定义，texture 采样也会被 DCE，**前提是驱动支持 SPIR-V dead code 消除**。在桌面驱动上一般 OK，在某些 Adreno OpenCL/Vulkan 编译器上可能保留无用 sampler 访问。建议把 `PICK` 拆成独立 entrypoint / 独立 shader 而不是 ubershader 分支。

#### Suggestion
- [Shader] `testCat/shader/vulkan/object.vert:58` — `o_position` 注释里写 outputs，没有 `#ifdef`。如上面 Warning 所述应删除。

---

### 1.6 跨文件一致性

#### Critical
- [Shader] `default.vert` vs `default_30.vert` vs `object.vert` 三者的 vertex attribute layout **互相冲突**：
  - `default.vert`（ES 3.0）：loc0=position, loc1=normal, loc2=tangent, loc3=coord, loc6=joints, loc7=weights
  - `default_30.vert`（ES 3.0）：loc0=position, loc1=color, loc2=coord, loc3=joints(SKIN), loc4=weights(SKIN)
  - `object.vert`（Vulkan）：loc0=position, loc1=normal, loc2=tangent, loc3=uv, loc5=color, loc6=joints, loc7=weights
  
  Vulkan 端和 OpenGL `default.vert` 几乎一致（推测后端正确），但 `default_30.vert` 单独一套。如果 C++ 侧 `primitive.cpp::_attrNameToIndex` 是按 **name** 分配 location，那么 `layout(location=N)` 是错的；如果是按显式 location，那么 `default_30.vert` 的 `joints` 与 `default.vert` 的 `tangent` 都在 location=3，**绑定 SKIN mesh 时几何会被骨骼数据踩烂**。强烈建议统一 layout，或在 `default_30.vert` 删除 `layout(location=...)` 让 link-time 按 name 分配。

#### Warning
- [Shader] OpenGL ES 系列 frag 全部使用了 `mediump sampler2D`（默认），但 **没有任何文件指定 mipmap filtering / texture wrapping hint**。这是 C++ 侧的事（`glTexParameteri`），但 shader 里如果将来加 `textureLod` / `textureGrad`，**ES 2.0 没有这俩内建**，ES 3.0+ 才有。当前混用 ES 1.00 (`default10`) 和 ES 3.00（其他）的工程，新增功能要小心 fallback。

#### Suggestion
- [Shader] 全部 shader 文件 — **缺少版本注释/作者/用途说明**。建议每个 shader 顶部加：
  ```glsl
  // shader: default.vert
  // stage: vertex
  // target: GLES 3.00
  // macros: SKIN_MESH
  // attributes: see primitive.cpp::_attrNameToIndex
  ```
  可读性和可维护性显著提升。

---

### 1.7 ALU / 采样 / 控制流 总览（10 个 shader）

| Shader | sampler 数 | 算术复杂度 | discard | divergent if |
|---|---|---|---|---|
| color.frag | 0 | 极低 | 无 | 无 |
| default.frag | 1 | 极低（仅 1 次 texture） | 无 | 无 |
| default10.frag | 1 | 极低 | 无 | 无 |
| default_30.frag | 1 | 极低 | 无 | 无 |
| object.frag | 1 | 低 | 无 | `PICK`/`TEXTURE`/`COLOR` 静态宏，编译期决定 |

- ✅ 无 `sin/cos/pow/exp/log` 在 frag 主路径
- ✅ 无 `discard` → Early-Z / HSR 不会被破坏
- ✅ 无 dependent texture read（UV 直接来自 varying，未在 frag 计算）
- ✅ 所有分支都是 **静态 #ifdef** 而非 runtime branch，对移动端 GPU 友好
- ⚠️ Skin 路径在 VS 里展开成 4 个 mat4 sum，**没有 unroll hint 但编译器一般能自动展开**

---

## 第二节：Shader 系统 C++ 审查

### 2.1 `cat/shader.cpp::Shader::_loadfile` — 多个 Critical bug

#### Critical
- [Shader] `cat/shader.cpp:51-86` (`_loadfile`) — **错误返回值**：
  ```cpp
  if (!f.open(filename, "rb"))
  {
      assert(false);
      return false;  // ← 函数返回类型是 char*，这里返回 bool false 隐式转 NULL
  }
  ```
  Release 构建 `assert` 是空操作，shader 文件缺失时返回 `NULL`。然后 `Shader::shader()` 行 40 直接把 `NULL` 传给 `render->createShader(vs_code, ps_code)`，又传给 GLES 的 `glShaderSource` 或 Vulkan 的 `shaderc_compile_into_spv` → 段错误或编译失败。**至少要在 Release 也走错误路径**：返回早期错误码，或者 `Shader::shader()` 显式判 NULL。

#### Critical
- [Shader] `cat/shader.cpp:70-83` — **macro 注入算法对「`#version` 前面有非空白字符」的情况会产生 garbage 字节**。算法逻辑：
  1. 在 source 中 `strstr` 找 `#version`，得到指针 `version`（可能位于注释里）。
  2. 复制 `version` 起 `versionlen` 字节到 `buf[0..versionlen-1]`。
  3. 在 `buf[versionlen..]` 写入 macros。
  4. **不修改 buf[macrolen..] 区域的原始文件内容**，依赖 "文件内容里的 #version 行刚好覆盖了 buf[16..16+macrolen-1] 的尾部" 来对齐。
  
  这只在「文件以 `#version` 开头，零前缀」时刚好工作。如果文件前面有任何字符（哪怕 `//` 注释、BOM、空白），就会出现 **buf 中间一段错位的、未清零的旧 file content 字节**。
  
  **`testCat/shader/opengles/default_30.vert/.frag` 正中此坑**：文件第 1 行是 `//#version 300 es`，`strstr` 在注释里匹配 `#version`：
  - 把 `#version 300 es\n` 复制到 buf 开头（**意外地把版本激活了**，这反倒"歪打正着"），
  - 然后 macros，
  - 然后 buf 中间剩下 `s\n` 这种 2 字节的尾巴（因为原文件 `//` 比 `#version` 多 2 字节，错位了 2 字节），
  - 然后才是真正的代码。
  
  GLSL 解析器看到 `[macros]\ns\n[rest]` 会**编译报错**。`default_30.*` 这两份 shader 在带 macro 时无法用。
  
  **修复方案**：明确语义 —— 找到 `#version` 行所在的**整行**（从行首到行尾），在该整行后插入 macros，并把原文件中的 `#version` 行替换为空白或重写整个 buffer。建议用 `String` 拼接而非这种就地内存操作：
  ```cpp
  // 伪代码
  String result;
  const char* p = strstr(source, "#version");
  if (p) {
      const char* lineEnd = strchr(p, '\n');
      result.append(source, lineEnd - source + 1);
      result.append(macros);
      result.append(lineEnd + 1);
  } else {
      result.append(macros);
      result.append(source);
  }
  ```

#### Critical
- [Shader] `cat/shader.cpp:78-79` — `while (*p++ != '\n') { }` 没有边界检查。如果 source 里 `#version` 之后**没有换行符就到了文件末尾**（例如最后一个 shader 文件没有终止换行），会越界读到 buffer 末尾以外。配合 `new char[buflen]` + `memset(buf, 0, buflen)` 一般 `\0` 会终止读取，但 `\0 != '\n'`，循环依然继续到下一页内存 → **越界 / UB**。修：`while (*p != 0 && *p != '\n') p++;`。

#### Critical
- [Shader] `cat/shader.cpp:64` — `f.read(buf + macrolen, filesize)` 不检查返回值即 `assert(readlen == filesize)`。Release 失去断言后，readlen 可以是 -1 或小于 filesize，后面 strstr 仍然在 zero-padding 上扫描 — 大概率走 NULL 分支。**建议**：检查并返回错误。

#### Warning
- [Shader] `cat/shader.cpp:35,51` — `_loadfile` 返回 `char*`，由调用方 `delete[]`。当前实现用 `new char[]`，没问题，但**全文件没有错误时仍然返回一个 1-字节 buffer**（`return false` → NULL ≠ 1 字节，纠正前面说法：如果 `f.open` 失败，会返回 NULL，调用方 `delete[] NULL` 安全，但接下来 `render->createShader(NULL, ...)` 会崩溃）。和上面的 critical 是同一问题，**优先级 critical**。

### 2.2 `cat/shaderMacro.cpp`

#### Critical
- [Shader] `cat/shaderMacro.cpp:72-81` — `ShaderMacroArray::assign(const ShaderMacro* macros, const int macroCount)` 有**严重 typo**：
  ```cpp
  m_macros.clear();
  for (int i = 0; i < macroCount; ++i)
      add(m_macros[i]);   // ← 错！应该是 macros[i]
  ```
  当前实现把刚刚清空的 `m_macros` 里的元素再"加回去"（其实是越界读 0 个元素，因为 size==0）。**结果是 assign 后 m_macros 永远是空数组**。任何调用此 overload 的代码（看似没有现成调用方，但 `shaderCache.cpp::getPickShader` 用了 `pickMacros.assign(shader->macros())`，那个 overload 是不同的，没踩这个雷）会得到错误结果。**必修**。

#### Warning
- [Shader] `cat/shaderMacro.cpp:17,27` — `add(...)` 在 `contains(name)` 时**静默丢弃新值**。如果先 `add("JOINT_MATRIX_COUNT", 50)` 再 `add("JOINT_MATRIX_COUNT", 100)`，第二个被忽略，**bug 很难发现**。建议至少 `assert(false)`，或允许覆盖语义。

#### Warning
- [Shader] `cat/shaderCache.cpp:23-66` 和 `shader.cpp::_allmacros` — **shader cache key 顺序敏感**。例如：
  - 先 add `NORMAL`、再 add `TEXTURE` → key 是 `"NORMAL::TEXTURE::"`
  - 先 add `TEXTURE`、再 add `NORMAL` → key 是 `"TEXTURE::NORMAL::"`
  
  两次得到**不同的 Shader 对象**，编译两次，但 GLSL 编译结果完全相同。浪费编译时间 + 内存。建议 cache 之前对 macros **按 name 排序**，让 key canonical。`_getShacroFromGltfPrimitive` 当前是固定顺序，"凑巧"不踩雷，但容易被新代码破坏。

#### Warning
- [Shader] `cat/shaderMacro.cpp:35-45` (`remove`) — 用 `m_macros.erase_fast(i)`（swap-and-pop）+ 反向遍历。`contains` 已经保证无重复，所以单次 remove 实际只命中一个；反向遍历在 swap-and-pop 下安全。**但语义不直观**，注释一下或改成正向 + 第一次命中 break 更易读。

#### Suggestion
- [Shader] `cat/shaderMacro.cpp:40,51` — `ShaderMacro m = m_macros[i];` 是**拷贝**而不是引用，每次 contains/remove 都额外构造 String。改成 `const ShaderMacro&` 引用。

### 2.3 `cat/shaderCache.cpp`

#### Warning
- [Shader] `cat/shaderCache.cpp:137-173` (`_getShacroFromGltfPrimitive`) — **属性名匹配带平台差异**：
  - 检查的 attr name 是 `"joints"`、`"weights"`（**小写**）
  - 但又检查 `"NORMAL"`、`"TANGENT"`、`"TEXCOORD"`、`"COLOR"`（**大写**）
  
  glTF 规范里 attribute 是大写（`POSITION`, `NORMAL`, `JOINTS_0`, `WEIGHTS_0`...）。如果 `cgltf_primitive_has_attr` 是大小写敏感的：
  - 用小写 `"joints"` 很可能**永远返回 false** → SKIN 宏永不开启 → 所有骨骼 mesh 都不开启 skin shader path → **角色蒙皮渲染挂了**。
  - 用大写 `"NORMAL"` 配合 cgltf 内部 enum/字符串可能也对不上，因为 glTF 标准里是 `NORMAL`，但 cgltf 通常用 `cgltf_attribute_type_normal`（type enum）而非字符串。
  
  需要核对 `cgltf_util::cgltf_primitive_has_attr` 的实现。**强烈怀疑这是个隐藏的 critical bug**。

#### Warning
- [Shader] `cat/shaderCache.cpp:169-172` — `cgltf_primitive_has_attr(primitive, "COLOR")` 触发 `macros.add("COLOR")`。但 vertex shader `object.vert` 里 `COLOR` macro 控制是否启用 `i_color` 在 location=5 的输入。如果 mesh 实际有 `COLOR_0` 但 C++ 端 VAO 绑定到了不同 location，**Vulkan layout 校验会失败**。需要同步 C++ 顶点 buffer 绑定与 shader location。

#### Warning
- [Shader] `cat/shaderCache.cpp:23-66` — **cache key 用 String 拼接**，每次 getShader 都构造长串、走 `scl::tree<String, Shader*>` 平衡树查找。对于 cache 命中率高的场景，每帧若有几百次 getShader 调用会有可观察开销。建议改成「macros 排序 + FNV-1a hash + uint64 → Shader*」哈希表。

#### Suggestion
- [Shader] `cat/shaderCache.cpp:27-34` — 每次都 `string256` 包装 filename 并 `.trim()`。filename 一般不会有头尾空白，trim 浪费 CPU。如果是为防御性，可以在 `Shader::load` 里做一次即可。

#### Suggestion
- [Shader] `cat/shaderCache.cpp:108-118` (`getPickShader`) — 每次拾取都查/编译一份独立 shader，但 PICK 路径只输出 `pushConst.pickColor`，**和原本 vertex shader 输入的几何完全无关**。其实 PICK shader 完全可以是**一份共享的极简 shader**（只用 position + mvp + pushconst 颜色），不需要按"原 shader 的 macro 减 COLOR/TEXTURE 加 PICK"派生 N 份。**减少 permutation 爆炸**。

### 2.4 `cat/shader_gles.cpp` + `shader_gles.h`

#### Warning
- [Shader] `cat/shader_gles.h:123-259` — 头文件里塞了 **8 个完整 GLSL shader 字符串**作为 `static const char* const`，被多个 .cpp include 时会**多副本编译进 binary**。改成 `extern const char* const` 在 .h，定义在 .cpp。

#### Warning
- [Shader] `cat/shader_gles.h:121` — `#else  // end of USE_OPENGL_300_ES` 分支下嵌入的内联 shader 是 `#version 100`（ES 2.0），但 cpp 端 `shader_gles.cpp` 在 ifndef OPENGL_ES 时引入 GLEW + glut 的桌面 OpenGL 头。**`#version 100` 在桌面 GL 上是 GLSL 1.00 ES**，桌面驱动 (GLEW) 一般不接受 ES 版本字符串。建议在桌面构建里用 `#version 110` 或加 `#version 100\n#extension GL_ARB_ES2_compatibility : require` —— 或者干脆在桌面 + USE_OPENGL_300_ES off 的情况下用 `#version 120` 兼容版。

#### Warning
- [Shader] `cat/shader_gles.cpp:51` — `FILE_LEN = 1024 * 1024` 写死成 1MB 的 malloc。① 没有 `f` 空检查（line 47 检查了，OK）；② shader 源代码 >1MB 会被静默截断（极不可能但仍是隐患）；③ 应该 `fseek+ftell` 拿真实大小。同时 line 53 `memset` 整 1MB 浪费 CPU，每次加载至少要清零 1MB。

#### Warning
- [Shader] `cat/shader_gles.cpp:54` — `fread` 返回值未检查，截断时无错误。

#### Suggestion
- [Shader] `cat/shader_gles.cpp:113-114` — `glBindAttribLocation` 被注释掉。这意味着 GLES 端**完全依赖 GLSL 内的 `layout(location = N)` 来分配 attribute slot**。问题：① `default10.vert` 是 ES 1.00，**不支持** `layout`，必须靠 `glBindAttribLocation`！现在被注释掉 → ES 1.00 路径下 attribute 与 C++ VBO 绑定**完全随机**，渲染必然出错；② 此处至少要在 ES 1.00 路径下补 bind。

#### Suggestion
- [Shader] `cat/shader_gles.cpp:87-105` — 编译失败时只 print log 和 return 0，**没有把 source code 也打出来**。调试 macro 注入失败时（参考 2.1 的 critical）极难定位是哪一段被切坏。建议失败时 dump source + macros 到 stderr 或文件。

#### Suggestion
- [Shader] `cat/shader_gles.cpp` — **完全没有 GL Program binary 缓存**（`GL_OES_get_program_binary` / `GL_ARB_get_program_binary`）。移动端冷启动每次都重编 GLSL → SPIR-V → 平台 ISA，**首次启动可能 200ms+**。建议加 disk 缓存。

### 2.5 `catvulkan/cat/vulkanRender.cpp` + `simplevulkan.cpp`

#### Warning
- [Shader] `catvulkan/cat/simplevulkan.cpp:696` — `shaderc_compile_into_spv` **每次 createShader 都全量调用 glslang/shaderc**，没有 SPIR-V disk cache。移动端 cold-start 编译时间是性能黑洞。建议在 Vulkan 端：① 优先 fopen 同名 `.spv` 文件；② 找不到才走 shaderc + 缓存写盘。

#### Warning
- [Shader] `catvulkan/cat/simplevulkan.cpp:701` — `shaderc_compile_into_spv` 的第 6 个参数 `entry_point = "main"`，固定写死。如果将来想用同一个 SPIR-V 模块导出多个 entry point（如 Vulkan SPIR-V 支持），不能复用。可接受。

#### Warning
- [Shader] `catvulkan/cat/simplevulkan.cpp:703` — `shaderc_compile_into_spv(..., nullptr)` 第 7 个参数 `compile_options` 传 NULL。这意味着 **没有传 macros 给 shaderc**！项目用法是 macros 已经被字符串拼到 source 头部（`cat/shader.cpp::_allmacros`），所以 shaderc 端不需要再处理，但**也意味着 shaderc 默认编译目标是 Vulkan 1.0 + 没有优化**。建议显式设置：
  ```cpp
  shaderc_compile_options_t opts = shaderc_compile_options_initialize();
  shaderc_compile_options_set_target_env(opts, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
  shaderc_compile_options_set_optimization_level(opts, shaderc_optimization_level_performance);
  // Debug build:
  // shaderc_compile_options_set_generate_debug_info(opts);
  ```
  当前默认情形下，shaderc 不做 SPIR-V 优化（`shaderc_optimization_level_zero`），导致 **Vulkan 端 SPIR-V 比手写 .spv 大 2-3x**，对 mobile pipeline cache 命中率不利。

#### Warning
- [Shader] `catvulkan/cat/simplevulkan.cpp:732-749` — `_createShaderFromFile` 与 `_createShaderFromCode` 同时存在两套路径，但**项目主流程**（`cat/shader.cpp`）走的是 `createShader(vs_code, ps_code)` → `svkCreateShaderProgramFromCode` 即 code 路径。`_createShaderFromFile` 看起来是测试/历史代码，建议明确标注 deprecated 或删除，避免歧义。

#### Warning
- [Shader] `catvulkan/cat/simplevulkan.cpp:741` — `int len = fread(buf, 1, BUF_SIZE, f);` — 同 GLES 端，没检查 f != NULL，没检查 fread 返回值，写死 1MB buffer。`fopen` 失败时 f=NULL → `fread(buf, 1, BUF_SIZE, NULL)` 是 UB。

#### Suggestion
- [Shader] `catvulkan/cat/simplevulkan.cpp:725` — `_createShader(device.device, (uint*)bytes, bytesLen)` 把 SPIR-V byte 长度直接传，但 Vulkan 要求 `codeSize` 是字节数（OK），`pCode` 是 `uint32_t*` 且要 4 字节对齐。`shaderc_result_get_bytes` 返回的是 `const char*`，对齐**未必保证**。强转 `(uint*)` 在 ARM 严格对齐设备上**可能 trap**。规范做法：把 bytes 拷到 `std::vector<uint32_t>` 再交给 VkShaderModule。

#### Suggestion
- [Shader] `catvulkan/cat/vulkanRender.cpp:559-565` (`createShader`) — `svkCreateShaderProgramFromCode` 内部用 `new VkDescriptorSetLayoutBinding[bindCount]` 与 `new VkPushConstantRange[]`，对应 `releaseShader` 调 `svkDestroyShaderProgram`。需要核对 destroy 函数是否真的 `delete[]` 这两块数组（容易漏一个变成内存泄露）。

### 2.6 跨后端命名一致性

#### Warning
- [Shader] `cat/shader.h:9-13` — 路径硬编码：
  ```cpp
  #ifdef TEST_VULKAN
  #define SHADER_PATH "shader/vulkan/"
  #else
  #define SHADER_PATH "shader/opengles/"
  #endif
  ```
  但 `shaderCache::getDefaultShader` 强制用 `SHADER_PATH "object.vert"`。**GLES 端 `opengles/` 目录下根本没有 `object.vert`**！只有 `default.vert`、`default_30.vert`、`default10.vert`、`color.*`。运行 GLES build 时 `getDefaultShader` 必然找不到文件 → `_loadfile` 返回 NULL → 崩溃。**Critical**，请同步两边命名。

#### Critical
- [Shader] 命名冲突总结：
  - GLES path 找的是 `shader/opengles/object.vert`，**不存在**
  - 同一份 ShaderCache::getDefaultShader 接口必须改成根据后端选择不同 base name（如 GLES 用 `default.vert`），或在 GLES 目录补一个 `object.vert` 镜像。
  - `object.frag` 同样不存在于 GLES 目录。

### 2.7 macro permutation 评估

实际可能的 macro 集合（来自 `_getShacroFromGltfPrimitive`）：
- `SKIN`（带 `JOINT_MATRIX_COUNT` 数值，N 个 distinct skinJointCount 各算一种）
- `NORMAL`
- `TANGENT`
- `TEXTURE`
- `COLOR`
- `PICK`（getPickShader 添加）

布尔型：5 个 → 32 种组合；× JOINT_MATRIX_COUNT 的不同值（场景中常用 1~3 种）≈ **96 个 shader permutation**。这个数量级**对 Vulkan 完全可控**，但因为是运行时 shaderc 编译 + 没有 disk cache，**首次访问每个 permutation 都会卡顿**。建议预热（在 loading 屏期间把场景内所有用到的 permutation 提前 createShader）。

---

## 总结

### 严重度统计

| 级别 | 数量 |
|---|---|
| Critical | 8 |
| Warning | 27 |
| Suggestion | 12 |
| **合计** | **47** |

### Shader 数量统计

- OpenGL ES shader：8 个（4 对 vert/frag）
- Vulkan shader：2 个（1 对 vert/frag）
- C++ 内嵌 GLSL shader（`shader_gles.h`）：8 段
- **总计**：18 段 shader source

### 最严重的 5 个问题（按优先级）

1. **`cat/shader.cpp::_loadfile` macro 注入算法错误** — 当 source 不以 `#version` 开头（例如 `default_30.*` 的 `//#version`）时，注入后会出现 2~N 字节 garbage，shader 直接编译失败。这是 `default_30.vert`/`default_30.frag` 无法工作的根因。修复：用 String 拼接代替就地内存覆盖。

2. **`cat/shader.cpp::_loadfile` 文件打开失败返回 `false`（NULL）** — 返回类型是 `char*`，调用方完全不检查 NULL，直接传给 GL/Vulkan API → 段错误或 ShaderModule 创建失败。

3. **`shader_gles.cpp` 注释掉 `glBindAttribLocation`** — ES 1.00 路径下 `default10.vert` 不支持 `layout(location=...)`，依赖 bind attrib 才能正确绑定 VBO。被注释后，ES 1.00 路径渲染必然错乱。

4. **`ShaderCache::getDefaultShader` 与 GLES 文件名不一致** — 强制读 `shader/opengles/object.vert`，但 GLES 目录下无此文件 → GLES build 启动崩溃。

5. **`ShaderMacroArray::assign(const ShaderMacro*, int)` 用 `m_macros[i]` 而非参数 `macros[i]`** — 任何走这个 overload 的代码都得到空 macro 数组，shader permutation 全错。

### 二级重要（高优先级 Warning）

6. **`default.vert` vs `default_30.vert` 顶点 attribute layout 冲突**（loc1 一个是 normal、一个是 color）—— C++ VAO 绑定到错的 shader 会得到错误几何。
7. **`object.frag` 没有 `precision` 声明** —— 移动 Vulkan 上所有 float 走 highp，移动 GPU ALU 浪费严重。
8. **Skin 用 `vec4 joints` + `int(joints.x)` 而非 `uvec4`**（GLES 端）—— 性能浪费且与 Vulkan 端不一致。
9. **`shaderc_compile_into_spv` 无 optimization options + 无 SPIR-V disk cache** —— 移动端冷启动时间长。
10. **`o_position` / `vertex_color` 死 varying 链** —— 多个 shader 的 VS 输出但 FS 不读，浪费 fragment 插值带宽。

---

