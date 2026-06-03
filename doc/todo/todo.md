# todo

- **ensure 多线程去重**（`free/scl/assert.h` 宏内 `static bool s_logged`）：scl 进多线程项目且要严格 once 时换 `volatile int + scl::compare_and_swap`；当前 race 是良性退化（多几次 log/break），非必修。详见 `doc/architect/错误处理-哲学与工具集.md`。
