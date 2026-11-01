#!/usr/bin/env python3
"""Chấm bài tập SQL F04-1.  Dùng: python3 check.py answers.sql
Mỗi câu chạy trên một database mới tạo từ schema.sql, kết quả được so với mã băm của đáp án."""
import hashlib, json, re, sqlite3, sys
from pathlib import Path

HERE = Path(__file__).parent
EXPECTED = {"q01": "afa92f38d63d0280", "q02": "94b91728db143051", "q03": "a3da999315bd0a2d", "q04": "c8ff3b78e79c119c", "q05": "5c931f27e4dbe39b", "q06": "e56d3bf078abcc60", "q07": "e0fb072e5d124495", "q08": "356fd3acfbcc1cfe", "q09": "c9f8648c064f7591", "q10": "92f8a9a031a163c4", "q11": "303fd6a9ee0bf869", "q12": "c1588823f6518619", "q13": "daf375080c313c3e", "q14": "87a9c4d2a786b0dc", "q15": "fb54709f65f089c1"}   # @@EXPECTED@@ (tự sinh bởi: python3 check.py --make-expected solutions.sql)
CHECK_AFTER = {"q15": "SELECT plan, COUNT(*) FROM subscriber GROUP BY plan ORDER BY plan"}

def split(text):
    parts, cur, key = {}, [], None
    for line in text.splitlines():
        m = re.match(r"^--\s*@(q\d+)", line)
        if m:
            if key: parts[key] = "\n".join(cur)
            key, cur = m.group(1), []
        elif key:
            cur.append(line)
    if key: parts[key] = "\n".join(cur)
    return parts

def is_ordered(block):
    return "Thứ tự:" in block

def run(sql, key):
    db = sqlite3.connect(":memory:")
    db.executescript((HERE / "schema.sql").read_text(encoding="utf-8"))
    code = "\n".join(l for l in sql.splitlines() if not l.strip().startswith("--")).strip()
    if not code:
        return None, "chưa làm"
    try:
        rows = db.execute(code).fetchall()
        if key in CHECK_AFTER:
            rows = db.execute(CHECK_AFTER[key]).fetchall()
    except sqlite3.Error as e:
        return None, f"lỗi SQL: {e}"
    return rows, None

def digest(rows, ordered):
    rows = [list(r) for r in rows]
    if not ordered:
        rows = sorted(rows, key=lambda r: json.dumps(r))
    return hashlib.sha256(json.dumps(rows).encode()).hexdigest()[:16]

def main():
    make = sys.argv[1] == "--make-expected"
    path = sys.argv[2] if make else sys.argv[1]
    blocks = split(Path(path).read_text(encoding="utf-8"))
    if make:
        exp = {}
        for k, b in blocks.items():
            rows, err = run(b, k)
            assert err is None, (k, err)
            exp[k] = digest(rows, is_ordered(b))
            print(k, len(rows), "dòng")
        src = Path(__file__).read_text(encoding="utf-8")
        src = re.sub(r"EXPECTED = \{.*?\}   # @@EXPECTED@@", "EXPECTED = " + json.dumps(exp) + "   # @@EXPECTED@@", src)
        Path(__file__).write_text(src, encoding="utf-8")
        return 0
    ok = 0
    for k in sorted(EXPECTED):
        b = blocks.get(k, "")
        rows, err = run(b, k)
        if err:
            print(f"{k}: {err}")
        elif digest(rows, is_ordered(b)) == EXPECTED[k]:
            print(f"{k}: ĐÚNG"); ok += 1
        else:
            print(f"{k}: SAI ({len(rows)} dòng). Vài dòng đầu: {rows[:3]}")
    print(f"\n{ok}/{len(EXPECTED)} câu đúng")
    return 0 if ok == len(EXPECTED) else 1

if __name__ == "__main__":
    sys.exit(main())
