// Tabelle der zuletzt gesehenen Drohnen, indiziert über MAC des Senders.
#pragma once
#include "rid_parser.h"

constexpr int TABLE_SIZE = 16;

struct DroneEntry {
    bool used;
    RidFix last;
    uint32_t first_ms, last_ms, last_pub_ms;
    bool id_announced;
};

void table_init();
// Liefert true, wenn die Drohne neu in der Tabelle ist.
bool table_update(const RidFix &fix);
void table_expire(uint32_t now_ms);
const DroneEntry *table_get(int index);
int table_count();
