/* =============================================================================
 * COD-013 — Queue (FIFO) bang HAI STACK       Phien B2 · 2026-09-13 · DIEM 2/4
 * Log: ../../sessions/2026-09-13--B2--coding.md
 *
 * ✅ DUOC
 *   [D1] Nhip (1) hoi spec dung cho: "complexity mong doi?", "pop khi rong?"
 *   [D2] Y tuong nen dung: hai stack, dao thu tu hai lan thanh FIFO
 *   [D3] empty() kiem CA HAI stack — dung
 *   [D4] size() = tong hai stack — dung
 *   [D5] Nhip (4) trace tay chinh xac
 *   [D6] Tu phat hien ca pop() tren queue rong (nhip 5), va DA SUA trong phien
 *
 * ❌ MAT DIEM  (day la bai duoc bao truoc "diem nam o nhip (2)")
 *   [L1] 🔴 KHONG dat toi AMORTIZED O(1). Thiet ke don TOAN BO phan tu o
 *        MOI loi goi => n thao tac ton O(n^2), khong phai O(n).
 *   [L2] 🔴 Vong "don nguoc" st2 -> st1 o cuoi pop() la nguyen nhan goc.
 *        No huy bo dung cong viec vua lam xong.
 *   [L3] Sau 2 probe van khong tim ra. Cau 2d "mot phan tu bi chuyen bao
 *        nhieu lan": thiet ke nay = 2k lan (k = so pop no nam qua);
 *        toi uu = DUNG 1 LAN, ca doi.
 *   [L4] Phuong an 2c (doi cho ton sang push) van la O(n)/thao tac —
 *        chi doi cho, khong khu duoc chi phi.
 *   [L5] Nhip (5) noi "cho return -1" nhung ban dau nop KHONG co guard =>
 *        st2.top() tren stack rong = UB. Loi khong khop giua LOI NOI va CODE.
 *   [L6] Guard sau khi sua chi kiem st1, trong khi empty() kiem ca hai —
 *        dung duoc nho bat bien rieng cua thiet ke nay, nhung mong manh.
 * ========================================================================== */

#include <stack>
#include <stdexcept>
#include <cstdio>
#include <cassert>

/* ---------------------------------------------------------------------------
 * PHAN 1 — BAN UNG VIEN NOP (ban cuoi, sau khi tu them guard trong phien)
 * ------------------------------------------------------------------------- */
class Queue2Stack_sub {
    std::stack<int> st1, st2;
public:
    void push(int v) {
        /* [L2] Vong nay la CODE CHET voi chinh bat bien cua thiet ke: pop()
         * luon don nguoc het ve st1, nen st2 LUON rong khi toi day. */
        while (!st2.empty()) { st1.push(st2.top()); st2.pop(); }
        st1.push(v);
    }

    int pop() {
        if (st1.empty()) return -1;              /* [D6][L6] them trong phien */

        while (!st1.empty()) {                   /* don xuoi:  O(n) */
            st2.push(st1.top()); st1.pop();
        }
        int popped_val = st2.top();
        st2.pop();
        while (!st2.empty()) {                   /* [L1][L2] don NGUOC: O(n) */
            st1.push(st2.top()); st2.pop();      /* <- chinh la cho hong */
        }
        return popped_val;
    }

    bool   empty() const { return st1.empty() && st2.empty(); }  /* [D3] */
    size_t size()  const { return st1.size()  + st2.size();  }   /* [D4] */
};

/* ---------------------------------------------------------------------------
 * PHAN 2 — BAN SUA
 * Giu nguyen moi quyet dinh hop ly: hai std::stack, ten in_/out_, empty()
 * kiem ca hai, size() la tong. Chi va [L1][L2][L5][L6].
 * ------------------------------------------------------------------------- */
class Queue2Stack {
    std::stack<int> in_, out_;

    /* Doc TOAN BO in_ sang out_. Goi DUY NHAT khi out_ rong. */
    void shift() {
        while (!in_.empty()) { out_.push(in_.top()); in_.pop(); }
    }
public:
    void push(int v) { in_.push(v); }            /* [L2] bo vong chet -> O(1) */

    int pop() {
        if (out_.empty()) shift();               /* [L1] CHI don khi CAN */
        if (out_.empty())                        /* [L5][L6] kiem sau khi don */
            throw std::runtime_error("pop tren queue rong");
        int v = out_.top();
        out_.pop();
        return v;                                /* [L2] KHONG don nguoc */
    }

    bool   empty() const { return in_.empty() && out_.empty(); }
    size_t size()  const { return in_.size()  + out_.size();  }
};
/* ⭐ CAU PHAI NOI RA O PHONG VAN:
 *    "pop te nhat la O(n), nhung AMORTIZED O(1) — vi moi phan tu chi bi
 *     chuyen tu in_ sang out_ DUNG MOT LAN trong ca doi no. n thao tac ton
 *     tong O(n), khong phai O(n^2)."
 *    Nguoi noi "O(n)" dung mot nua. Dieu kien `if (out_.empty())` chinh la
 *    TOAN BO thuat toan — bo no di thi hai stack thanh vo nghia. */

/* ---------------------------------------------------------------------------
 * PHAN 3 — TEST + DO SO LAN DICH CHUYEN (bang chung cho [L3])
 * ------------------------------------------------------------------------- */
/* Ban dem: giong het hai thiet ke nhung dem so lan mot phan tu bi day sang
 * stack kia. Day la con so cua cau 2d. */
static long moves_sub = 0, moves_fix = 0;

class CountSub {
    std::stack<int> a, b;
public:
    void push(int v){ while(!b.empty()){a.push(b.top());b.pop();++moves_sub;} a.push(v); }
    int  pop(){
        if (a.empty()) return -1;
        while(!a.empty()){ b.push(a.top()); a.pop(); ++moves_sub; }
        int v=b.top(); b.pop();
        while(!b.empty()){ a.push(b.top()); b.pop(); ++moves_sub; }
        return v;
    }
    bool empty() const { return a.empty() && b.empty(); }
};
class CountFix {
    std::stack<int> a, b;
public:
    void push(int v){ a.push(v); }
    int  pop(){
        if (b.empty()) while(!a.empty()){ b.push(a.top()); a.pop(); ++moves_fix; }
        if (b.empty()) return -1;
        int v=b.top(); b.pop(); return v;
    }
    bool empty() const { return a.empty() && b.empty(); }
};

int main()
{
    /* --- dung dan: FIFO --- */
    Queue2Stack q;
    q.push(1); q.push(2); q.push(3);
    assert(q.pop() == 1);
    assert(q.pop() == 2);
    q.push(4);                      /* push XEN GIUA — ca de sai nhat */
    assert(q.pop() == 3);
    assert(q.pop() == 4);
    assert(q.empty());
    puts("FIFO + push xen giua:  OK (1 2 3 4)");

    /* --- [L5] ca bien: pop tren queue rong --- */
    bool threw = false;
    try { q.pop(); } catch (const std::exception &e) { threw = true;
        printf("pop tren queue rong:   nem \"%s\"\n", e.what()); }
    assert(threw);

    /* --- size() --- */
    Queue2Stack s;
    s.push(10); s.push(20);
    assert(s.size() == 2);
    (void)s.pop();
    assert(s.size() == 1);
    puts("size() qua shift:      OK");

    /* --- [L3] DO SO LAN DICH CHUYEN: bang chung O(n^2) vs O(n) --- */
    const int N = 2000;
    { CountSub c; for (int i=0;i<N;++i) c.push(i); for (int i=0;i<N;++i) (void)c.pop(); }
    { CountFix c; for (int i=0;i<N;++i) c.push(i); for (int i=0;i<N;++i) (void)c.pop(); }

    printf("\nN = %d phan tu, push het roi pop het:\n", N);
    printf("  ban ung vien : %8ld lan dich chuyen   (~N^2 = %d)\n", moves_sub, N*N);
    printf("  ban sua      : %8ld lan dich chuyen   (= N, moi phan tu DUNG 1 lan)\n", moves_fix);
    printf("  ty le        : %.1f lan\n", (double)moves_sub / (double)moves_fix);
    assert(moves_fix == N);
    return 0;
}
