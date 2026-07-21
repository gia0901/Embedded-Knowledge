# 💠 Kế hoạch ôn — System Software / C++ Engineer

> Đúng background hiện tại của bạn (C++ shared library, viết API interface cho lớp trên). Xem [cách dùng chung](README.md). Hai tầng: 🏃 **Sprint** + 📚 **Nền tảng**.
> Trọng tâm phân biệt với BSP: nghiêng về **ngôn ngữ C++ hiện đại + ABI/shared library + concurrency + design** thay vì kernel/phần cứng.

---

## Bản đồ nhanh — vị trí System SW/C++ hỏi gì

```
Modern C++ ──► ABI / Shared library ──► Concurrency ──► Design
(smart ptr,     (Pimpl, versioning,      (thread, mutex,   (SOLID, pattern,
 move, RAII,     visibility, memory        atomic, memory    abstraction,
 rule 5/0,       qua boundary)             order, deadlock)   interface)
 template)             │
                       ▼
        OS fundamentals ── DSA ── System design ── Debug
        (process/thread,  (Big-O,  (trade-off,      (gdb, sanitizer,
         VM, IPC, sched)   pattern) scalability)     core dump)
```

Nguồn tự kiểm tra chính: **[02_question_bank.md](../technical_round/02_question_bank.md)** (phần 1–2, 6–7) + [11-interview-questions/cpp](../../11-interview-questions/cpp.md) + [11/design-patterns](../../11-interview-questions/design-patterns.md).

---

## 🏃 Tầng SPRINT (≈4 tuần — chỉ mục 🎯)

### Tuần 1 — Modern C++ core

- [ ] 🎯 **Smart pointer: unique/shared/weak, control block, make_shared, chu trình** — [EMC++ cụm 4](../../16-book-summaries/effective-modern-cpp.md) · [02-modern-cpp/raii-smart-pointers](../../02-modern-cpp/raii-smart-pointers.md)
- [ ] 🎯 **Move semantics: std::move/forward, universal ref, perfect forwarding, RVO** — [EMC++ cụm 5](../../16-book-summaries/effective-modern-cpp.md) · [02/move-semantics](../../02-modern-cpp/move-semantics.md)
- [ ] 🎯 **Rule of 5/0, special members, noexcept + move trong container** — [EMC++ cụm 3](../../16-book-summaries/effective-modern-cpp.md)
- [ ] 🎯 **RAII + exception safety; virtual destructor; vtable/vptr** — [01/oop](../../01-cpp-fundamentals/oop.md) · [02_question_bank Q8–9, Q12](../technical_round/02_question_bank.md)
- [ ] Tự kiểm tra: [02_question_bank](../technical_round/02_question_bank.md) **Q1–Q12**

### Tuần 2 — Concurrency

- [ ] 🎯 **thread/mutex/atomic; volatile vs atomic; data race** — [EMC++ cụm 7](../../16-book-summaries/effective-modern-cpp.md) · [02/concurrency](../../02-modern-cpp/concurrency.md)
- [ ] 🎯 **Lock: spinlock vs mutex, futex; condition variable (while-not-if)** — [ostep/concurrency](../../16-book-summaries/ostep/concurrency.md) cụm 2–3
- [ ] 🎯 **4 điều kiện deadlock + cách phá; atomicity violation; std::scoped_lock** — [ostep/concurrency](../../16-book-summaries/ostep/concurrency.md) cụm 5 · [02_question_bank Q15–17](../technical_round/02_question_bank.md)
- [ ] 🎯 **async vs thread, future, memory order (seq_cst/acquire/release)** — [EMC++ cụm 7](../../16-book-summaries/effective-modern-cpp.md) · [03/sync-primitives](../../03-operating-system/sync-primitives.md)
- [ ] Tự kiểm tra: [02_question_bank](../technical_round/02_question_bank.md) **Q15–Q20**

### Tuần 3 — ABI/Shared library + Design

- [ ] 🎯 **Static vs shared vs header-only; link time vs load time** — [cpp-mindset/modularity](../../16-book-summaries/cpp-mindset/modularity.md) cụm 1 · [07/static-vs-shared](../../07-shared-libraries/static-vs-shared.md)
- [ ] 🎯 **Giữ ABI ổn định: Pimpl, hidden visibility, inline namespace versioning** — [cpp-mindset/modularity](../../16-book-summaries/cpp-mindset/modularity.md) cụm 2–3 · [07/abi-versioning](../../07-shared-libraries/abi-versioning.md) · [EMC++ Item 22](../../16-book-summaries/effective-modern-cpp.md)
- [ ] 🎯 **Memory qua boundary; không ném exception qua ABI; failure vs error, std::expected** — [cpp-mindset/modularity](../../16-book-summaries/cpp-mindset/modularity.md) cụm 3 · [cpp-mindset/thinking-computationally](../../16-book-summaries/cpp-mindset/thinking-computationally.md) cụm 5 · [07/api-design](../../07-shared-libraries/api-design.md)
- [ ] 🎯 **SOLID + pattern hay hỏi (strategy, factory, observer, Pimpl/bridge)** — [12-design-patterns](../../12-design-patterns/) · [11/design-patterns](../../11-interview-questions/design-patterns.md)
- [ ] Tự kiểm tra: [11/design-patterns](../../11-interview-questions/design-patterns.md) + [11/cpp](../../11-interview-questions/cpp.md)

### Tuần 4 — OS/DSA/System design + Mock

- [ ] 🎯 **process vs thread, context switch, virtual memory/paging, syscall** — [ostep/virtualization-cpu](../../16-book-summaries/ostep/virtualization-cpu.md) + [ostep/virtualization-memory](../../16-book-summaries/ostep/virtualization-memory.md) · [03-operating-system](../../03-operating-system/)
- [ ] 🎯 **IPC so sánh (pipe/shm/mq/socket); memory bug + sanitizer** — [03/ipc](../../03-operating-system/ipc.md) · [09/memory-bugs](../../09-debugging/memory-bugs.md) · [02_question_bank Q18](../technical_round/02_question_bank.md)
- [ ] 🎯 **Big-O, amortized (vector), container trade-off, std::map vs unordered vs flat** — [cpp-mindset/algorithmic-complexity](../../16-book-summaries/cpp-mindset/algorithmic-complexity.md) + [cpp-mindset/data-structures](../../16-book-summaries/cpp-mindset/data-structures.md) · [13-dsa](../../13-dsa/)
- [ ] **System design (embedded-leaning)** — [10/system-design](../../10-thinking/system-design.md) · [11/system-design](../../11-interview-questions/system-design.md)
- [ ] **Mock**: nói to smart pointer (Q1), move (Q4), ABI-stability, deadlock (Q15) không nhìn tài liệu
- [ ] **Behavioral**: ôn STAR + "vì sao rời công ty cũ" + câu hỏi ngược — [technical_round/03_behavior.md](../technical_round/03_behavior.md)

---

## 📚 Tầng NỀN TẢNG (dài hơi — phủ toàn diện)

**C++ ngôn ngữ (đọc trọn):**
- [ ] [EMC++](../../16-book-summaries/effective-modern-cpp.md) — cả 8 cụm/42 items (kể cả deduction, lambda, tweaks)
- [ ] [cpp-mindset/](../../16-book-summaries/cpp-mindset/README.md) — cả 6 chương Phần 1 (tư duy → abstraction → complexity → machine → data structures → modularity)
- [ ] [01-cpp-fundamentals/](../../01-cpp-fundamentals/) — memory-model, oop, templates 🕳️ *(template metaprogramming sâu: SFINAE/CRTP/traits còn mỏng)*
- [ ] [02-modern-cpp/](../../02-modern-cpp/) — đầy đủ (lambdas-functional...)

**OS & system programming:**
- [ ] [ostep/](../../16-book-summaries/ostep/README.md) — cả 4 file
- [ ] [03-operating-system/](../../03-operating-system/) + [04-linux-system-programming/](../../04-linux-system-programming/) — đầy đủ

**Design & kiến trúc:**
- [ ] [12-design-patterns/](../../12-design-patterns/) — SOLID, creational/structural/behavioral đầy đủ
- [ ] [10-thinking/](../../10-thinking/) — problem-solving, system-design

**Shared library (đúng nghề — đọc kỹ):**
- [ ] [07-shared-libraries/](../../07-shared-libraries/) — cả 4 file (static-vs-shared, linking-loading, abi-versioning, api-design)

**DSA & networking:**
- [ ] [13-dsa/](../../13-dsa/) — complexity-and-structures, algorithm-patterns (luyện LeetCode song song)
- [ ] [14-networking/](../../14-networking/) — tcp-ip, sockets 🕳️ *(còn nhẹ)*

**Debug:**
- [ ] [09-debugging/](../../09-debugging/) — gdb, tools, memory-bugs (sanitizer)

---

## ✅ Vòng tự kiểm tra

1. **[02_question_bank.md](../technical_round/02_question_bank.md)** — phần 1 (Modern C++), 2 (System), 6 (Debug), 7 (Coding). **Bộ chính.**
2. [11-interview-questions/](../../11-interview-questions/) — cpp, design-patterns, operating-system, dsa (đáp án ẩn, phân độ khó).
3. [01_theory.md](../technical_round/01_theory.md) — mục A (Modern C++), B (System), G (DSA).
4. Book summary "Góc interview" — mỗi cụm EMC++/cpp-mindset có câu hỏi + đáp án đầy đủ + bẫy.

**Chuẩn "đạt":** giải thích được **bản chất** (vì sao thiết kế vậy) + nêu bẫy, không chỉ trả lời cụt.

---

## 🕳️ Lỗ hổng tài liệu cho vị trí C++

Xem [gap-register.md](gap-register.md) mục "C++". Đáng chú ý: **C++20/23 depth** (ranges/concepts/coroutines — EMC++ dừng ở C++14, cpp-mindset chạm nhẹ), **template metaprogramming sâu** (SFINAE/CRTP/type traits). Lấp khi các mục 🎯 đã vững.
