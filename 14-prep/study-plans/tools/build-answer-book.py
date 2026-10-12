#!/usr/bin/env python3
"""Sinh "Sổ trả lời mẫu" (§14, từ bank) và "Sổ hình vẽ" (§15, từ whiteboard.md) trong resume-plan.md.

Nguồn sự thật DUY NHẤT là bank: mỗi câu có khối
    <details><summary>🎙️ Trả lời mẫu</summary> ... </details>
ngay dưới đề bài. Script này chép đề + khối đó vào plan, giữa hai marker
ANSWER-BOOK. KHÔNG sửa tay phần giữa hai marker — sửa ở bank rồi chạy lại:

    python3 14-prep/study-plans/tools/build-answer-book.py

Thứ tự và nhóm theo các bảng "### 🎤 Mock" của plan (không lấy rapid pool).
Câu xuất hiện ở nhiều buổi chỉ chép một lần, ở buổi đầu tiên.
"""
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PLAN = os.path.normpath(os.path.join(HERE, '..', 'resume-plan.md'))
BANK = os.path.normpath(os.path.join(HERE, '..', '..', 'mock-interview', 'bank'))
BEGIN = '<!-- BEGIN ANSWER-BOOK (tự sinh bởi tools/build-answer-book.py — KHÔNG sửa tay) -->'
END = '<!-- END ANSWER-BOOK -->'
WB_BEGIN = '<!-- BEGIN WHITEBOARD (tự sinh từ 14-prep/whiteboard.md — KHÔNG sửa tay) -->'
WB_END = '<!-- END WHITEBOARD -->'
WHITEBOARD = os.path.normpath(os.path.join(HERE, '..', '..', 'whiteboard.md'))
MODEL_SUMMARY = '<summary>🎙️ Trả lời mẫu</summary>'
# Thứ tự buổi trong sổ (khớp thứ tự chạy của plan)
SESSION_ORDER = ['R1', "R1′", 'R2', 'R3', 'R4', 'R5', 'R6']


def load_bank():
    entries = {}
    for path in sorted(glob.glob(os.path.join(BANK, '*.md'))):
        text = open(path, encoding='utf-8').read()
        for m in re.finditer(r'^#### ([A-Z]+-\d+) [^\n]*\n(.*?)(?=^#### |\Z)', text, re.S | re.M):
            entries[m.group(1)] = (path, m.group(2))
    return entries


def split_entry(body):
    """Trả (đề bài, nội dung trả lời mẫu hoặc None)."""
    lines = body.split('\n')
    stem, i = [], 0
    while i < len(lines) and not lines[i].startswith('<details>'):
        stem.append(lines[i])
        i += 1
    stem = [l for l in stem if not l.startswith('>')]            # bỏ ghi chú dạng blockquote
    stem_text = '\n'.join(stem).strip()
    stem_text = re.sub(r'\n---\s*$', '', stem_text).strip()
    model = None
    for m in re.finditer(r'<details><summary>(.*?)</summary>\n(.*?)\n</details>', body, re.S):
        if m.group(1) == '🎙️ Trả lời mẫu':
            model = m.group(2).strip()
            break
    return stem_text, model


def relink(text, src_dir):
    """Viết lại link tương đối: tính từ thư mục bank -> tính từ thư mục plan."""
    plan_dir = os.path.dirname(PLAN)

    def fix(m):
        target = m.group(2)
        if re.match(r'^(https?:|#|mailto:)', target):
            return m.group(0)
        path, sep, anchor = target.partition('#')
        if not path:
            return m.group(0)
        absolute = os.path.normpath(os.path.join(src_dir, path))
        new = os.path.relpath(absolute, plan_dir)
        return f'{m.group(1)}({new}{sep}{anchor})'
    return re.sub(r'(\]\s*)\(([^)\s]+)\)', fix, text)


def plan_sessions(plan_text):
    """[(nhãn buổi, tiêu đề mục, [(ID, tóm tắt)])] theo các bảng 🎤 Mock."""
    out = []
    for sec in re.finditer(r'^## \d+\. (R\d′?)[^\n]*\n(.*?)(?=^## |\Z)', plan_text, re.S | re.M):
        label = sec.group(1)
        title = sec.group(0).split('\n', 1)[0][3:].strip()
        mock = re.search(r'### 🎤 Mock[^\n]*\n(.*?)(?=\n### |\Z)', sec.group(2), re.S)
        if not mock:
            continue
        rows = []
        for r in re.finditer(r'^\| \d+ \| `([A-Z]+-\d+)`[^|]*\| ([^|]+)\|', mock.group(1), re.M):
            rows.append((r.group(1), r.group(2).strip()))
        out.append((label, title, rows))
    order = {k: i for i, k in enumerate(SESSION_ORDER)}
    out.sort(key=lambda s: order.get(s[0], 99))
    return out


def build(plan_text, bank):
    parts, seen, missing = [], set(), []
    for label, title, rows in plan_sessions(plan_text):
        parts.append(f'### 📚 Sổ · {title}\n')
        repeated = []
        for qid, summary in rows:
            if qid in seen:
                repeated.append(qid)
                continue
            seen.add(qid)
            path, body = bank[qid]
            stem, model = split_entry(body)
            src_dir = os.path.dirname(path)
            bank_link = os.path.relpath(path, os.path.dirname(PLAN))
            if model is None:
                missing.append(qid)
                model = '*(chưa có trả lời mẫu trong bank)*'
            parts.append(
                f'<details><summary><b>{qid}</b> · {summary}</summary>\n\n'
                f'{relink(stem, src_dir)}\n\n'
                f'{relink(model, src_dir)}\n\n'
                f'<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [{qid}]({bank_link})</sub>\n\n'
                f'</details>\n')
        if repeated:
            parts.append('*Câu đã có ở buổi trước: ' + ' · '.join(f'`{q}`' for q in repeated) + '*\n')
        parts.append('')
    return '\n'.join(parts).rstrip() + '\n', len(seen), missing


def wb_heading(title):
    """Tiêu đề của một phần whiteboard khi chép vào plan (D0..D5 có tiền tố 'Hình')."""
    return ('Hình ' + title) if re.match(r'D\d', title) else ('Whiteboard · ' + title)


def build_whiteboard():
    text = open(WHITEBOARD, encoding='utf-8').read()
    src_dir = os.path.dirname(WHITEBOARD)
    parts = []
    for m in re.finditer(r'^## ([^\n]+)\n(.*?)(?=^## |\Z)', text, re.S | re.M):
        title, body = m.group(1).strip(), m.group(2)
        body = re.sub(r'\n---\s*$', '', body.strip()).strip()
        body = re.sub(r'^(#{3,5}) ', r'#\1 ', body, flags=re.M)        # hạ một cấp tiêu đề
        body = relink(body, src_dir)
        parts.append(f'### {wb_heading(title)}\n\n<details><summary>Mở hình + kịch bản</summary>\n\n{body}\n\n</details>\n')
    return '\n'.join(parts)


def local_whiteboard_links(text):
    """Link sang whiteboard.md#dN-... trong sổ trả lời -> nhảy tới mục hình ngay trong plan."""
    return re.sub(r'\(\.\./whiteboard\.md#(d\d[^)]*)\)', lambda m: '(#hình-' + m.group(1) + ')', text)


def replace_between(text, begin, end, content):
    if begin not in text or end not in text:
        sys.exit(f'Không thấy marker {begin[:30]}… trong plan')
    head, rest = text.split(begin, 1)
    _, tail = rest.split(end, 1)
    return head + begin + '\n\n' + content + '\n' + end + tail


def main():
    plan_text = open(PLAN, encoding='utf-8').read()
    book, n, missing = build(plan_text, load_bank())
    plan_text = replace_between(plan_text, BEGIN, END, local_whiteboard_links(book))
    if WB_BEGIN in plan_text:
        plan_text = replace_between(plan_text, WB_BEGIN, WB_END, build_whiteboard())
    open(PLAN, 'w', encoding='utf-8').write(plan_text)
    print(f'{n} câu đã chép vào sổ trả lời.')
    if missing:
        print('Chưa có trả lời mẫu:', ', '.join(missing))


if __name__ == '__main__':
    main()
