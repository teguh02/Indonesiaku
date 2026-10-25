#include <math.h>
#include <string.h>
#include "native.h"
#include "vm.h"
#include "sqlite3.h"

#define DB_EXACT_INT 9007199254740992LL

static Value dbError(sqlite3* db) {
    nativeRaiseKelas(vm.galatNilai, "SQLite: %s", sqlite3_errmsg(db));
    return KOSONG_VAL;
}

static ObjDb* dbHandle(int count, Value* args, int min, int max) {
    if (count < min || count > max || !IS_DB(args[0])) {
        nativeRaiseKelas(vm.galatTipe, "db: jumlah argumen atau handle tidak valid");
        return NULL;
    }
    ObjDb* db = AS_DB(args[0]);
    if (db->db == NULL) {
        nativeRaiseKelas(vm.galatNilai, "db: handle sudah ditutup");
        return NULL;
    }
    return db;
}

static bool dbText(Value value) {
    if (!IS_STRING(value)) {
        nativeRaiseKelas(vm.galatTipe, "db: SQL/jalur harus teks");
        return false;
    }
    ObjString* s = AS_STRING(value);
    if (memchr(s->chars, '\0', (size_t)s->length) != NULL) {
        nativeRaiseKelas(vm.galatNilai, "db: SQL/jalur tidak boleh mengandung NUL");
        return false;
    }
    return true;
}

Value nativeFnDbBuka(int count, Value* args) {
    if (count != 1) {
        nativeRaiseKelas(vm.galatTipe, "db_buka() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!dbText(args[0])) return KOSONG_VAL;
    ObjDb* db = newDb();
    push(OBJ_VAL(db));
    int rc = sqlite3_open_v2(AS_CSTRING(args[0]), &db->db,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK) {
        dbError(db->db);
        sqlite3_close_v2(db->db);
        db->db = NULL;
        pop();
        return KOSONG_VAL;
    }
    pop();
    return OBJ_VAL(db);
}

Value nativeFnDbTutup(int count, Value* args) {
    if (count != 1 || !IS_DB(args[0])) {
        nativeRaiseKelas(vm.galatTipe, "db_tutup() memerlukan handle db");
        return KOSONG_VAL;
    }
    ObjDb* db = AS_DB(args[0]);
    if (db->db != NULL) {
        int rc = sqlite3_close_v2(db->db);
        if (rc != SQLITE_OK) return dbError(db->db);
        db->db = NULL;
    }
    return KOSONG_VAL;
}

// Do not prepare the tail: some PRAGMAs take effect at prepare time.
static bool dbTailEmpty(const char* tail) {
    while (*tail) {
        if (*tail == ' ' || (*tail >= '\t' && *tail <= '\r')) {
            tail++;
        } else if (tail[0] == '-' && tail[1] == '-') {
            tail += 2;
            while (*tail && *tail != '\n') tail++;
        } else if (tail[0] == '/' && tail[1] == '*') {
            tail += 2;
            while (*tail && !(tail[0] == '*' && tail[1] == '/')) tail++;
            if (*tail) tail += 2;
        } else {
            return false;
        }
    }
    return true;
}

static sqlite3_stmt* dbPrepare(sqlite3* db, Value sql, Value params) {
    if (!dbText(sql)) return NULL;
    if (!IS_LIST(params) && !IS_KOSONG(params)) {
        nativeRaiseKelas(vm.galatTipe, "db: params harus daftar");
        return NULL;
    }
    int count = IS_LIST(params) ? AS_LIST(params)->items.count : 0;
    Value* values = IS_LIST(params) ? AS_LIST(params)->items.values : NULL;
    // Validate every value before preparing or stepping any SQL.
    for (int i = 0; i < count; i++) {
        Value v = values[i];
        if (IS_NUMBER(v)) {
            double n = AS_NUMBER(v);
            if (!isfinite(n) || (floor(n) == n && fabs(n) > DB_EXACT_INT)) {
                nativeRaiseKelas(vm.galatNilai, "db: angka parameter di luar rentang tepat");
                return NULL;
            }
        } else if (!IS_STRING(v) && !IS_BOOL(v) && !IS_KOSONG(v)) {
            nativeRaiseKelas(vm.galatTipe, "db: tipe parameter tidak didukung");
            return NULL;
        }
    }
    // Preflight BEFORE prepare: PRAGMA foreign_keys and similar statements
    // can mutate connection state during prepare, not just during step.
    // sqlite3_complete understands quoted semicolons and trigger bodies.
    char* text = sqlite3_mprintf("%s", AS_CSTRING(sql));
    if (text == NULL) {
        nativeRaiseKelas(vm.galatNilai, "SQLite: memori habis");
        return NULL;
    }
    bool single = true;
    // ponytail: prefix checks are quadratic for huge trigger bodies; use a
    // tokenizer only if large generated SQL becomes a supported workload.
    for (int i = 0; i < AS_STRING(sql)->length; i++) {
        if (text[i] != ';') continue;
        char saved = text[i + 1];
        text[i + 1] = '\0';
        int complete = sqlite3_complete(text);
        text[i + 1] = saved;
        if (complete) {
            single = dbTailEmpty(text + i + 1);
            break;
        }
    }
    sqlite3_free(text);
    if (!single) {
        nativeRaiseKelas(vm.galatNilai, "db: tepat satu pernyataan SQL diperlukan");
        return NULL;
    }
    sqlite3_stmt* stmt = NULL;
    const char* tail = NULL;
    if (sqlite3_prepare_v2(db, AS_CSTRING(sql), AS_STRING(sql)->length,
                           &stmt, &tail) != SQLITE_OK) {
        dbError(db);
        sqlite3_finalize(stmt);
        return NULL;
    }
    if (stmt == NULL || !dbTailEmpty(tail)) {
        nativeRaiseKelas(vm.galatNilai, "db: tepat satu pernyataan SQL diperlukan");
        goto fail;
    }
    if (sqlite3_bind_parameter_count(stmt) != count) {
        nativeRaiseKelas(vm.galatNilai, "db: jumlah parameter tidak cocok");
        goto fail;
    }
    for (int i = 1; i <= count; i++) {
        if (sqlite3_bind_parameter_name(stmt, i) != NULL) {
            nativeRaiseKelas(vm.galatNilai, "db: hanya parameter anonymous ? didukung");
            goto fail;
        }
    }
    for (int i = 0; i < count; i++) {
        Value v = values[i];
        int rc;
        if (IS_KOSONG(v)) rc = sqlite3_bind_null(stmt, i + 1);
        else if (IS_BOOL(v)) rc = sqlite3_bind_int(stmt, i + 1, AS_BOOL(v) ? 1 : 0);
        else if (IS_INT(v)) {
            rc = sqlite3_bind_int64(stmt, i + 1, (sqlite3_int64)AS_INT(v));
        } else if (IS_NUMBER(v)) {
            double n = AS_NUMBER(v);
            rc = floor(n) == n ? sqlite3_bind_int64(stmt, i + 1, (sqlite3_int64)n)
                              : sqlite3_bind_double(stmt, i + 1, n);
        } else {
            ObjString* s = AS_STRING(v);
            rc = sqlite3_bind_text(stmt, i + 1, s->chars, s->length, SQLITE_TRANSIENT);
        }
        if (rc != SQLITE_OK) {
            dbError(db);
            goto fail;
        }
    }
    return stmt;
fail:
    sqlite3_finalize(stmt);
    return NULL;
}

// K2: SQLite INTEGER is a bulat now, so the old ±2^53 restriction is gone —
// sqlite3_int64 and our int64 are the same width.
static Value dbInteger(sqlite3_int64 n) {
    return INT_VAL((int64_t)n);
}

static Value dbColumn(sqlite3_stmt* stmt, int i) {
    switch (sqlite3_column_type(stmt, i)) {
        case SQLITE_NULL: return KOSONG_VAL;
        case SQLITE_INTEGER: return dbInteger(sqlite3_column_int64(stmt, i));
        case SQLITE_FLOAT: return NUMBER_VAL(sqlite3_column_double(stmt, i));
        case SQLITE_TEXT: {
            const char* text = (const char*)sqlite3_column_text(stmt, i);
            int length = sqlite3_column_bytes(stmt, i);
            if (text == NULL) return dbError(sqlite3_db_handle(stmt));
            return OBJ_VAL(copyString(text, length));
        }
        default:
            nativeRaiseKelas(vm.galatTipe, "db: BLOB belum didukung");
            return KOSONG_VAL;
    }
}

// mode: 0 executes/discards rows, 1 returns one row, 2 materializes all rows.
static Value dbQuery(int count, Value* args, int mode) {
    ObjDb* db = dbHandle(count, args, 2, 3);
    if (db == NULL) return KOSONG_VAL;
    if (count == 3 && !IS_LIST(args[2])) {
        nativeRaiseKelas(vm.galatTipe, "db: params harus daftar");
        return KOSONG_VAL;
    }
    sqlite3_stmt* stmt = dbPrepare(db->db, args[1], count == 3 ? args[2] : KOSONG_VAL);
    if (stmt == NULL) return KOSONG_VAL;
    Value result = mode == 2 ? OBJ_VAL(newList()) : KOSONG_VAL;
    push(result);
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (mode == 0) continue;
        ObjDict* row = newDict();
        push(OBJ_VAL(row));
        for (int i = 0; i < sqlite3_column_count(stmt); i++) {
            const char* name = sqlite3_column_name(stmt, i);
            if (name == NULL) {
                dbError(db->db);
                pop();
                goto done;
            }
            ObjString* key = copyString(name, (int)strlen(name));
            push(OBJ_VAL(key));
            Value value = dbColumn(stmt, i);
            push(value);
            if (vm.nativeErrorPending) {
                pop(); pop(); pop();
                goto done;
            }
            dictSet(row, key, value);
            pop(); pop();
        }
        if (mode == 1) {
            result = OBJ_VAL(row);
            pop();
            pop();
            push(result);
            break;
        }
        writeValueArray(&AS_LIST(result)->items, OBJ_VAL(row));
        pop();
    }
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) dbError(db->db);
done:
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK && !vm.nativeErrorPending) dbError(db->db);
    pop();
    return vm.nativeErrorPending ? KOSONG_VAL : result;
}

Value nativeFnDbEksekusi(int n, Value* a) { return dbQuery(n, a, 0); }
Value nativeFnDbAmbilSatu(int n, Value* a) { return dbQuery(n, a, 1); }
Value nativeFnDbAmbilSemua(int n, Value* a) { return dbQuery(n, a, 2); }

Value nativeFnDbBarisTerpengaruh(int n, Value* a) {
    ObjDb* db = dbHandle(n, a, 1, 1);
    return db == NULL ? KOSONG_VAL : dbInteger(sqlite3_changes64(db->db));
}
Value nativeFnDbIdTerakhir(int n, Value* a) {
    ObjDb* db = dbHandle(n, a, 1, 1);
    return db == NULL ? KOSONG_VAL : dbInteger(sqlite3_last_insert_rowid(db->db));
}

static Value dbTransaction(int n, Value* a, const char* sql) {
    ObjDb* db = dbHandle(n, a, 1, 1);
    if (db == NULL) return KOSONG_VAL;
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) dbError(db->db);
    int finish = sqlite3_finalize(stmt);
    if (finish != SQLITE_OK && !vm.nativeErrorPending) dbError(db->db);
    return KOSONG_VAL;
}
Value nativeFnDbMulai(int n, Value* a) { return dbTransaction(n, a, "BEGIN"); }
Value nativeFnDbKomit(int n, Value* a) { return dbTransaction(n, a, "COMMIT"); }
Value nativeFnDbBatal(int n, Value* a) { return dbTransaction(n, a, "ROLLBACK"); }
