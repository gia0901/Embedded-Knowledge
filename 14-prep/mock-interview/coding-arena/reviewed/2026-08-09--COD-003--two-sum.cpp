// ═══════════════════════════════════════════════════════════════════════════
//  COD-003 · Two Sum (hash map)          Phiên: 2026-08-09 · coding · dsa
//  ĐIỂM: 4/4
//
//  ✅ One-pass chuẩn, tính đúng, xử lý đúng ca không có nghiệm.
//  ✅ Nêu đủ brute force O(n²)/O(1) → tối ưu O(n) time đổi bằng O(n) space.
//  ✅ Câu giải thích tính đúng của one-pass GIỮ NGUYÊN MÀ DÙNG khi phỏng vấn:
//     "nếu phần bù chưa có thì đến lượt phần bù, nó sẽ tìm ra được số này."
//     Gọn hơn hầu hết cách diễn đạt khác.
//  ⚠️ Gán nhầm nhãn bộ test khi nêu ca hỏng — cơ chế đúng, nhãn sai. Xem [1].
//  ⚠️ Hai lần tra hash cho cùng một key. Xem [2].
// ═══════════════════════════════════════════════════════════════════════════

#include <unordered_map>
#include <vector>
#include <cstdio>

// ─── BẢN CỦA BẠN — logic đúng, chỉ tinh chỉnh ở bản dưới ───────────────────
std::vector<int> twoSum_yours(const std::vector<int>& arr, int target) {
    std::unordered_map<int,int> mp;
    for (int i = 0; i < (int)arr.size(); i++) {
        int remain = target - arr[i];
        if (mp.count(remain)) {          // ⚠️ [2] tra hash lần 1
            return {mp[remain], i};      // ⚠️ [2] tra hash lần 2 — CÙNG key
        }
        mp[arr[i]] = i;                  // ✅ chèn SAU khi tra → tự động loại
                                         //    ca "tự cộng với chính mình"
    }
    return {};                           // ✅ có trả về cho ca vô nghiệm
}

// ─── BẢN TINH CHỈNH ────────────────────────────────────────────────────────
std::vector<int> twoSum(const std::vector<int>& arr, int target) {
    std::unordered_map<int,int> seen;
    seen.reserve(arr.size());            // ✅ né rehash dọc đường (n lần chèn
                                         //    → vài lần rehash + copy toàn bộ)
    for (int i = 0; i < (int)arr.size(); ++i) {
        auto it = seen.find(target - arr[i]);   // ✅ [2] MỘT lần tra
        if (it != seen.end()) return { it->second, i };
        seen.emplace(arr[i], i);
    }
    return {};
}

// [1] CA HỎNG — bạn mô tả ĐÚNG cơ chế nhưng gán nhầm bộ dữ liệu.
//
//     Cơ chế bạn nói: "chỉ có một số 3, nó tự tìm và cộng lại với chính nó"
//     → đó CHÍNH LÀ bộ ①, không phải bộ ②.
//
//     ① nums = [3, 2, 4], target = 6   (đáp án đúng: [1,2])
//        Nếu XÂY MAP ĐẦY ĐỦ TRƯỚC rồi mới duyệt (hai vòng, không kiểm tra
//        index): i=0, arr[0]=3, phần bù=3, map có 3→0 → trả [0,0]. ❌ SAI.
//        Đây là ca hỏng thật.
//
//     ② nums = [3, 3], target = 6      (đáp án đúng: [0,1])
//        Map hai vòng chỉ giữ được MỘT index cho key 3 (index sau ghi đè
//        index trước → 3→1). i=0 tra ra index 1 ≠ 0 → trả [0,1]. ✅ ĐÚNG,
//        nhưng đúng do MAY, không do thiết kế.
//
//     ⇒ One-pass miễn nhiễm cả hai vì CHÈN SAU KHI TRA: lúc tra key
//       arr[i], chính arr[i] còn CHƯA nằm trong map, nên không thể tự khớp.
//       Đây là lý do thật sự để chọn one-pass — không chỉ vì "một vòng nhanh
//       hơn hai vòng" (cùng O(n) cả).

// [3] Đánh đổi phải nói thành lời khi phỏng vấn:
//     - Brute force : O(n²) time / O(1) space
//     - Hash map    : O(n) time  / O(n) space   ← đổi bộ nhớ lấy thời gian
//     - Hai con trỏ : O(n log n) time / O(1) space — CHỈ khi mảng đã sắp xếp,
//                     hoặc được phép sắp xếp VÀ không cần trả về index gốc
//                     (sắp xếp là đảo mất index).
//     Trên hệ RAM chật, bản O(n) space có thể là bản KHÔNG dùng được → phải
//     hỏi ràng buộc bộ nhớ trước khi tuyên bố "tối ưu". (→ DSA-012)
//
//     ⚠️ O(1) trung bình của hash map là TRUNG BÌNH. Worst case O(n) khi
//     đụng độ nhiều — đủ để loại nó khỏi đường dữ liệu realtime cần tất định.

int main() {
    struct { std::vector<int> a; int t; } cases[] = {
        {{3,2,4},   6},   // → 1 2   ← ca bẫy "tự cộng chính mình"
        {{3,3},     6},   // → 0 1   ← ca trùng giá trị
        {{2,7,11,15}, 9}, // → 0 1
        {{1,2},     99},  // → (vô nghiệm)
    };
    for (auto& c : cases) {
        auto r = twoSum(c.a, c.t);
        if (r.empty()) std::printf("(vo nghiem)\n");
        else           std::printf("%d %d\n", r[0], r[1]);
    }
}
