#pragma once

struct block_store {
    int (*read)(int block_no, char* dst);
    int (*write)(int block_no, char* src);
};
