// ═══════════════════════════════════════════════════════════════════════════
//  COD-001 · Đảo Linked List            Phiên: 2026-08-09 · coding · dsa
//  ĐIỂM: 4/4
//
//  ✅ Code đúng, không cần sửa dòng nào. Giải thích 3 con trỏ + prev=nullptr
//     + while(cur) đều đúng bản chất. Follow-up doubly list trả lời đúng.
//  ⚠️ Hỏi "time VÀ space" → chỉ trả lời time. Space O(1) mới là lý do chọn
//     bản iterative. Xem [1] cuối file.
//  ⚠️ Hậu quả của while(cur->next) nêu thiếu vế nguy hiểm hơn. Xem [2].
// ═══════════════════════════════════════════════════════════════════════════

#include <bits/stdc++.h>          // ⚠️ [style] chỉ dùng khi luyện tập.
using namespace std;              //    Code thật: include đúng header cần.

struct Node {
    int val;
    Node* next;
    Node() : val(0), next(nullptr) {}
    Node(int val) : val(val), next(nullptr) {}
    Node(int val, Node* next) : val(val), next(next) {}
};

// ─── BẢN CỦA BẠN — giữ nguyên, không sửa gì ────────────────────────────────
Node* reverseList(Node* head) {
    Node* prev = nullptr;         // ✅ sẽ thành `next` của node đầu cũ = tail mới
    Node* cur = head;

    while (cur) {                 // ✅ [2] đúng. `while (cur->next)` sai HAI đường:
                                  //    (a) bỏ sót node cuối chưa kịp đảo  ← bạn nêu
                                  //    (b) SEGFAULT ngay với list rỗng    ← bạn thiếu
                                  //        (deref nullptr->next). Nêu vế crash mạnh
                                  //        hơn nêu vế sai kết quả.
        Node* nxt = cur->next;    // ✅ không lưu là mất sạch phần đuôi
        cur->next = prev;
        prev = cur;
        cur = nxt;
    }

    return prev;                  // ✅ cur == nullptr, prev đứng ở node cuối cùng
}
// ─── HẾT BẢN CỦA BẠN ───────────────────────────────────────────────────────

// [1] ĐỘ PHỨC TẠP — phải nói ĐỦ CẢ HAI, đây là chỗ ăn điểm:
//
//     Time  O(n)  — mỗi node thăm đúng một lần.
//     Space O(1)  — chỉ 3 con trỏ, KHÔNG phụ thuộc n.
//
//     Vì sao O(1) space đáng nói: bản đệ quy cũng O(n) time nhưng tốn O(n)
//     STACK. Trên embedded (stack task vài KB, không MMU → không guard page)
//     list dài = tràn stack ĐÈ LÊN .bss, corruption âm thầm, crash ở nơi khác
//     lúc khác. Đó là lý do thực chiến để chọn iterative, không phải "cho đẹp".
//     → 08-embedded-systems/memory-and-startup.md §4

// [3] BIẾN THỂ DOUBLY LINKED LIST (bạn trả lời đúng trong phiên — chép lại
//     để lần sau viết được ngay):
//
//     struct DNode { int val; DNode *next, *prev; };
//
//     DNode* reverseDList(DNode* head) {
//         DNode* cur  = head;
//         DNode* last = nullptr;
//         while (cur) {
//             std::swap(cur->next, cur->prev);  // đảo tại chỗ, KHÔNG cần biến nxt
//             last = cur;
//             cur  = cur->prev;                 // ← prev giờ đang giữ node KẾ cũ
//         }
//         return last;                          // node cuối thăm được = head mới
//     }
//
//     Điểm khác cốt lõi: singly cần `nxt` vì đảo xong là mất đường đi tiếp;
//     doubly KHÔNG cần vì sau swap, đường đi tiếp nằm trong `prev`.

int main() {
    // Ba ca biên phải test mồm trong phỏng vấn — bạn đã nêu đúng cả ba:
    //   ① rỗng     : head=nullptr → while không chạy → trả nullptr        ✅
    //   ② một node : chạy 1 vòng, node->next = nullptr → trả chính nó     ✅
    //   ③ hai node : ca nhỏ nhất thật sự có đảo, bắt được lỗi hoán vị
    Node* head = new Node(1, new Node(2, new Node(3, new Node(4))));
    for (Node* p = reverseList(head); p; p = p->next) cout << p->val << ' ';
    cout << '\n';                                     // kỳ vọng: 4 3 2 1
}
