-- Phiên bản 2: đếm số lần gửi thất bại. Có DEFAULT nên dòng cũ hợp lệ ngay.
ALTER TABLE outbox ADD COLUMN tries INTEGER NOT NULL DEFAULT 0;
