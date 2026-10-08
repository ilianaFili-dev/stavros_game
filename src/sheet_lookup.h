#pragma once
#include <cstring>

#include "generated/sheets.h"

namespace sheets {

// Finds a row by id in a generated sheet array; nullptr when the milestone doesn't include it.
template <typename Row, int N>
const Row* find(const Row (&rows)[N], int count, const char* id) {
    for (int i = 0; i < count && i < N; ++i) {
        if (rows[i].id && std::strcmp(rows[i].id, id) == 0) return &rows[i];
    }
    return nullptr;
}

}  // namespace sheets
