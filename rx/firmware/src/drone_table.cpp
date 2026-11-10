#include "drone_table.h"
#include <Arduino.h>
#include <string.h>
#include "config.h"

static DroneEntry s_table[TABLE_SIZE];

void table_init() { memset(s_table, 0, sizeof(s_table)); }

static DroneEntry *find_or_make(const char *mac, uint32_t now_ms) {
    for (auto &e : s_table)
        if (e.used && strcmp(e.last.mac, mac) == 0) return &e;
    // Frei oder am längsten her?
    DroneEntry *victim = nullptr;
    for (auto &e : s_table) {
        if (!e.used) { victim = &e; break; }
        if (!victim || e.last_ms < victim->last_ms) victim = &e;
    }
    memset(victim, 0, sizeof(*victim));
    victim->used = true;
    victim->first_ms = now_ms;
    return victim;
}

bool table_update(const RidFix &fix) {
    uint32_t now = millis();
    bool was_used = false;
    for (auto &e : s_table)
        if (e.used && strcmp(e.last.mac, fix.mac) == 0) { was_used = true; break; }

    DroneEntry &e = *find_or_make(fix.mac, now);
    bool keep_pub = e.last_pub_ms;
    bool keep_ann = e.id_announced;
    e.last = fix;
    e.last_ms = now;
    e.last_pub_ms = keep_pub;
    e.id_announced = keep_ann;
    return !was_used;
}

void table_expire(uint32_t now_ms) {
    for (auto &e : s_table)
        if (e.used && now_ms - e.last_ms > TABLE_EXPIRY_MS) e.used = false;
}

const DroneEntry *table_get(int index) {
    int seen = 0;
    for (auto &e : s_table) {
        if (!e.used) continue;
        if (seen++ == index) return &e;
    }
    return nullptr;
}

int table_count() {
    int n = 0;
    for (auto &e : s_table)
        if (e.used) n++;
    return n;
}
