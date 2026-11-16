// devinfo: tra thiết bị theo serial trong database fleet, theo hai cách.
//   --concat  ghép chuỗi serial vào câu SQL rồi PQexec (SAI: SQL injection)
//   --param   câu SQL cố định với $1, giá trị gửi riêng qua PQexecParams (ĐÚNG)
// Build: g++ -O2 -std=c++20 -Wall -I/usr/include/postgresql -o devinfo devinfo.cpp -lpq
// Chạy:  PGHOST=127.0.0.1 PGDATABASE=fleet PGUSER=... PGPASSWORD=... ./devinfo --concat|--param SERIAL
//        (libpq đọc các biến PG* khi chuỗi kết nối rỗng)
#include <libpq-fe.h>
#include <cstdio>
#include <cstring>
#include <string>

static void print(PGresult* r) {
    ExecStatusType st = PQresultStatus(r);
    if (st != PGRES_TUPLES_OK && st != PGRES_COMMAND_OK) { printf("loi: %s", PQresultErrorMessage(r)); return; }
    for (int i = 0; i < PQntuples(r); i++) {
        for (int j = 0; j < PQnfields(r); j++)
            printf("%s%s", j ? " | " : "", PQgetisnull(r, i, j) ? "(null)" : PQgetvalue(r, i, j));
        printf("\n");
    }
    printf("[%s, %d dong]\n", PQcmdStatus(r), PQntuples(r));
}

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s --concat|--param SERIAL\n", argv[0]); return 1; }
    PGconn* c = PQconnectdb("");
    if (PQstatus(c) != CONNECTION_OK) { fprintf(stderr, "%s", PQerrorMessage(c)); return 1; }
    PGresult* r;
    if (!strcmp(argv[1], "--concat")) {
        std::string sql = "SELECT serial, site, fw FROM devices WHERE serial = '" + std::string(argv[2]) + "'";
        printf("SQL: %s\n", sql.c_str());
        r = PQexec(c, sql.c_str());                      // PQexec chạy được nhiều câu cách nhau bởi ;
    } else {
        const char* vals[1] = {argv[2]};
        r = PQexecParams(c, "SELECT serial, site, fw FROM devices WHERE serial = $1",
                         1, nullptr, vals, nullptr, nullptr, 0);   // giá trị không bao giờ là SQL
    }
    print(r);
    PQclear(r);
    PQfinish(c);
}
