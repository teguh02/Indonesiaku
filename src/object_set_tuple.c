ObjTuple* newTuple(ValueArray items) {
    ObjTuple* tuple = ALLOCATE_OBJ(ObjTuple, OBJ_TUPLE);
    tuple->items = items;
    return tuple;
}

ObjSet* newSet() {
    ObjSet* set = ALLOCATE_OBJ(ObjSet, OBJ_SET);
    initValueTable(&set->table);
    initValueArray(&set->items);
    return set;
}

bool setAdd(ObjSet* set, Value value) {
    uint32_t hash;
    if (!hashValue(value, &hash)) {
        vm.runtimeErrorClass = vm.galatTipe;
        runtimeError("Tidak bisa menambah nilai yang tidak bisa di-hash ke himpunan.");
        return false;
    }
    if (valueTableGet(&set->table, value, NULL)) {
        return false;  // sudah ada
    }
    if (!valueTableSet(&set->table, value, value)) {
        // valueTableSet bisa gagal hanya jika out-of-memory di resize; ini
        // sangat jarang dan maknanya sama dengan “tidak bisa menyimpan”.
        runtimeError("Himpunan terlalu besar.");
        return false;
    }
    writeValueArray(&set->items, value);
    return true;
}

bool setRemove(ObjSet* set, Value value) {
    if (!valueTableDelete(&set->table, value)) return false;
    for (int i = 0; i < set->items.count; i++) {
        if (valuesEqual(set->items.values[i], value)) {
            for (int j = i; j < set->items.count - 1; j++) {
                set->items.values[j] = set->items.values[j + 1];
            }
            set->items.count--;
            return true;
        }
    }
    return false;
}

int setLength(ObjSet* set) {
    return set->items.count;
}

Value setMemberAt(ObjSet* set, int index) {
    if (index < 0 || index >= set->items.count) return KOSONG_VAL;
    return set->items.values[index];
}
