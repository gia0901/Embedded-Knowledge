/* =============================================================================
 * COD-002 — Phat hien vong trong linked list   Phien B2 · 2026-09-13 · DIEM 3/4
 * Log: ../../sessions/2026-09-13--B2--coding.md
 *
 * ✅ DUOC
 *   [D1] Nhip (1) hoi dung hai cau spec quan trong: co tail khong? mot node
 *        co tinh la cycle khong?
 *   [D2] Nhip (2) dung: slow 1 buoc / fast 2 buoc, O(n) time, O(1) space
 *   [D3] Dieu kien vong lap `fast->next && fast->next->next` — DUNG, va la
 *        cho de sai nhat cua bai nay
 *   [D4] Guard `head == nullptr` o dau
 *   [D5] Nhip (4) la DRY-RUN THAT bang tay, phu 5 ca (0 / 1-2 khong vong /
 *        >2 khong vong / self-loop / vong 2 node) — tien bo ro so voi bai 1
 *   [D6] Nhip (5) neu DUNG lo hong cua chinh minh: "lam sao ket luan O(n)"
 *   [D7] Follow-up 3b: bat duoc "khoang cach doi 1 don vi moi vong" va chan
 *        tren O(L) — phan cot loi cua chung minh
 *
 * ❌ MAT DIEM
 *   [L1] 3b dien dat nguoc chieu: noi "khoang cach XA DAN tung don vi".
 *        Do theo chieu di, khoang cach tu fast TOI slow GIAM 1 moi vong
 *        (fast an them 1 buoc). Con so dung, chieu sai.
 *   [L2] 3c "chua ro" — khong biet pha 2 cua Floyd (tim DAU vong).
 *        Day la phan mo rong gan nhu chac chan bi hoi tiep sau bai nay.
 * ========================================================================== */

#include <cstdio>
#include <cassert>
#include <vector>

struct Node {
    int   val;
    Node *next;
    Node() : val(0), next(nullptr) {}
    explicit Node(int v) : val(v), next(nullptr) {}
};

/* ---------------------------------------------------------------------------
 * PHAN 1 — BAN UNG VIEN NOP, GIU NGUYEN TUNG DONG
 * ------------------------------------------------------------------------- */
bool has_cycle_sub(Node *head) {
    if (head == nullptr) return false;               /* [D4] */

    Node *slow = head;
    Node *fast = head;

    while (fast->next && fast->next->next) {         /* [D3] dung */
        fast = fast->next->next;
        slow = slow->next;
        if (fast == slow) return true;
    }
    return false;
}
/* Ban nay DUNG hoan toan — da kiem moi ca bien o phan 3. Diem 3 mat o phan
 * LY LUAN (3b nguoc chieu) va PHAN MO RONG (3c chua biet), khong mat o code. */

/* ---------------------------------------------------------------------------
 * PHAN 2 — BAN SUA / MO RONG
 * Code phat hien vong GIU NGUYEN cua ung vien. Chi bo sung pha 2.
 * ------------------------------------------------------------------------- */
bool has_cycle(Node *head)                 /* y het ban ung vien */
{
    if (head == nullptr) return false;
    Node *slow = head, *fast = head;
    while (fast->next && fast->next->next) {
        fast = fast->next->next;
        slow = slow->next;
        if (fast == slow) return true;
    }
    return false;
}

/* [L2] PHA 2 — tra ve node BAT DAU vong, hoac nullptr neu khong co vong.
 *
 * Vi sao dung. Goi:
 *     m = quang duong tu head toi dau vong
 *     L = chu vi vong
 *     k = quang tu dau vong toi diem gap (di theo chieu vong)
 * Luc gap nhau: slow di (m + k), fast di 2*(m + k), va fast di hon slow
 * dung mot so nguyen lan chu vi => 2(m+k) - (m+k) = m + k = i*L
 * => m = i*L - k  == quang tu DIEM GAP di tiep toi DAU VONG (modulo L).
 * Nen: dat mot con tro ve head, ca hai di 1 BUOC — chung gap nhau dung tai
 * dau vong. Van O(1) space.
 */
Node *cycle_start(Node *head)
{
    if (head == nullptr) return nullptr;

    Node *slow = head, *fast = head;
    bool met = false;
    while (fast->next && fast->next->next) {
        fast = fast->next->next;
        slow = slow->next;
        if (fast == slow) { met = true; break; }
    }
    if (!met) return nullptr;

    slow = head;                       /* mot con tro ve dau           */
    while (slow != fast) {             /* CA HAI di 1 buoc (khong phai 2) */
        slow = slow->next;
        fast = fast->next;
    }
    return slow;                       /* diem gap = dau vong */
}

/* [L1] Chieu dung cua lap luan 3b:
 *   Trong vong, moi lan lap fast di 2, slow di 1 => fast AN THEM 1 buoc.
 *   Do theo chieu di, khoang cach tu fast toi slow GIAM di 1 moi vong lap.
 *   Khoang cach ban dau < L, giam 1 moi buoc, khong bao gio nhay qua 0
 *   (vi giam dung 1) => gap nhau sau TOI DA L lan lap.
 *   Cong doan di toi vong (m buoc) => tong O(m + L) = O(n). */

/* ---------------------------------------------------------------------------
 * PHAN 3 — TEST moi ca bien da ban trong phien
 * ------------------------------------------------------------------------- */
struct List {                                  /* giu node de xoa sach */
    std::vector<Node *> nodes;
    Node *build(int n) {                       /* n node, chua noi vong */
        if (n == 0) return nullptr;
        for (int i = 0; i < n; ++i) nodes.push_back(new Node(i));
        for (int i = 0; i + 1 < n; ++i) nodes[i]->next = nodes[i + 1];
        return nodes[0];
    }
    void link_tail_to(int idx) { nodes.back()->next = nodes[idx]; }
    ~List() { for (Node *p : nodes) delete p; } /* xoa theo vector, khong
                                                 * duyet next (co the co vong) */
};

int main()
{
    /* --- 0 node --- */
    assert(has_cycle(nullptr) == false);
    assert(cycle_start(nullptr) == nullptr);

    /* --- 1 node, khong vong --- */
    { List L; Node *h = L.build(1);
      assert(has_cycle(h) == false);  assert(has_cycle_sub(h) == false); }

    /* --- 1 node, SELF-LOOP --- */
    { List L; Node *h = L.build(1); L.link_tail_to(0);
      assert(has_cycle(h) == true);   assert(has_cycle_sub(h) == true);
      assert(cycle_start(h) == h); }

    /* --- 2 node, khong vong --- */
    { List L; Node *h = L.build(2);
      assert(has_cycle(h) == false);  assert(has_cycle_sub(h) == false); }

    /* --- 2 node, vong --- */
    { List L; Node *h = L.build(2); L.link_tail_to(0);
      assert(has_cycle(h) == true);   assert(has_cycle_sub(h) == true);
      assert(cycle_start(h) == h); }

    /* --- 6 node, khong vong --- */
    { List L; Node *h = L.build(6);
      assert(has_cycle(h) == false);  assert(has_cycle_sub(h) == false); }

    /* --- 6 node, vong bat dau o index 2 (m=2, L=4) --- */
    { List L; Node *h = L.build(6); L.link_tail_to(2);
      assert(has_cycle(h) == true);   assert(has_cycle_sub(h) == true);
      Node *st = cycle_start(h);
      assert(st == L.nodes[2]);
      printf("vong bat dau tai node val = %d  (ky vong 2)\n", st->val); }

    /* --- vong dai, dau vong o cuoi: m=99, L=1 --- */
    { List L; Node *h = L.build(100); L.link_tail_to(99);
      assert(has_cycle(h) == true);
      assert(cycle_start(h) == L.nodes[99]); }

    puts("has_cycle       : OK tren 8 ca bien");
    puts("cycle_start     : OK (self-loop, 2 node, m=2/L=4, m=99/L=1)");
    puts("ban ung vien    : trung khop ban sua tren MOI ca");
    return 0;
}
