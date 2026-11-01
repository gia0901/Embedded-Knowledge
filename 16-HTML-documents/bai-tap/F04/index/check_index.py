#!/usr/bin/env python3
"""Chấm bài tập F04-2.  Dùng: python3 check_index.py tuning.sql"""
import re, sqlite3, sys, time

SCHEMA = """
CREATE TABLE event (
    id      INTEGER PRIMARY KEY,
    imsi    TEXT    NOT NULL,
    cell_id INTEGER NOT NULL,
    ts      INTEGER NOT NULL,      -- giây kể từ đầu tuần
    cause   INTEGER NOT NULL       -- mã nguyên nhân, 0 = thành công
);
WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i + 1 FROM n WHERE i < 400000)
INSERT INTO event
SELECT i, '45204' || printf('%010d', (i * 7919) % 50000), 1 + (i * 13) % 500, (i * 37) % 604800,
       CASE WHEN i % 20 = 0 THEN 1 + i % 7 ELSE 0 END
FROM n;
"""
QUERIES = {
    "A": ("SELECT id, ts, cause FROM event WHERE imsi = '452040000012345' ORDER BY ts DESC LIMIT 20", True),
    "B": ("SELECT COUNT(*) FROM event WHERE cell_id = 42 AND ts >= 86400 AND ts < 172800", True),
    "C": ("SELECT COUNT(*) FROM event WHERE substr(imsi, 1, 12) = '452040000012'", True),
    "D": ("SELECT cause, COUNT(*) FROM event WHERE ts >= 601200 GROUP BY cause ORDER BY cause", True),
}
MAX_INDEXES = 3

def timed(db, sql, reps=5):
    best = 1e9
    for _ in range(reps):
        t0 = time.perf_counter(); rows = db.execute(sql).fetchall(); best = min(best, time.perf_counter() - t0)
    return rows, best

def plan(db, sql):
    return [r[3] for r in db.execute("EXPLAIN QUERY PLAN " + sql)]

def sections(text):
    out, key, cur = {}, None, []
    for line in text.splitlines():
        m = re.match(r"^--\s*@(\w+)", line)
        if m:
            if key: out[key] = "\n".join(cur)
            key, cur = m.group(1), []
        elif key and not line.strip().startswith("--"):
            cur.append(line)
    if key: out[key] = "\n".join(cur)
    return out

def main():
    sec = sections(open(sys.argv[1], encoding="utf-8").read())
    db = sqlite3.connect(":memory:")
    db.executescript(SCHEMA)
    base = {k: timed(db, q, 2) for k, (q, _) in QUERIES.items()}
    for k in QUERIES:
        print(f"Ban đầu {k}: {base[k][1] * 1000:7.2f} ms   plan: {'; '.join(plan(db, QUERIES[k][0]))}")
    ddl = sec.get("ddl", "").strip()
    n_idx = len(re.findall(r"create\s+(unique\s+)?index", ddl, re.I))
    try:
        if ddl: db.executescript(ddl)
    except sqlite3.Error as e:
        print("Lỗi trong phần @ddl:", e); return 1
    db.execute("ANALYZE")
    queries = dict((k, q) for k, (q, _) in QUERIES.items())
    if sec.get("qC", "").strip():
        queries["C"] = sec["qC"].strip().rstrip(";")
    ok = True
    print()
    for k, q in queries.items():
        rows, t = timed(db, q)
        p = plan(db, q)
        scan = any(re.match(r"SCAN event\b", s) for s in p)
        same = rows == base[k][0]
        fast = t * 10 <= base[k][1]
        verdict = "ĐẠT" if (same and not scan and fast) else "CHƯA ĐẠT"
        if verdict != "ĐẠT": ok = False
        why = [] if same else ["kết quả khác ban đầu"]
        if scan: why.append("vẫn quét cả bảng")
        if not fast: why.append(f"chỉ nhanh hơn {base[k][1] / max(t, 1e-9):.0f} lần")
        print(f"Sau     {k}: {t * 1000:7.2f} ms   plan: {'; '.join(p)}   {verdict} {', '.join(why)}")
    if n_idx > MAX_INDEXES:
        print(f"\nCHƯA ĐẠT: dùng {n_idx} index, tối đa {MAX_INDEXES}"); ok = False
    print("\nĐẠT tất cả" if ok else "\nCòn truy vấn chưa đạt")
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main())
