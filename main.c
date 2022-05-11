#include <stdio.h>
#include <stdint.h>
#include "gbk.h"

// Helper
void print_gbk_char(gbk_char_t c) {
    switch (c.type) {
        case GBK_CHAR_TYPE_SINGLE:
            putchar(c.ch.sb);
            break;
        case GBK_CHAR_TYPE_DOUBLE:
            putchar(c.ch.db[0]);
            putchar(c.ch.db[1]);
            break;
        case GBK_CHAR_TYPE_COMPOSE:
            putchar(c.ch.compose[0]);
            putchar(c.ch.compose[1]);
            putchar(c.ch.compose[2]);
            break;
        case GBK_CHAR_TYPE_NULL:
        default:
            break;
    }
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        printf("%s [file]\n", argv[0]);
        return 1;
    }
    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("File %s not exist\n", argv[1]);
        return 1;
    }
    char buf[4] = { };
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        buf[0] = ch & 0xff;
        if (ch == (ch & 0x7f)) {
            uint32_t idx = buf[0] & 0x7f;
            gbk_char_t gbkc = gbk_char_from_utf8(idx);
            print_gbk_char(gbkc);
        } else if (0xf0 == (ch & 0xf8)) {
            size_t len = fread(buf + 1, 1, 3, fp);
            if (len != 3) {
                break;
            }
            if ((0x80 != (buf[1] & 0xc0)) ||
                (0x80 != (buf[2] & 0xc0)) ||
                (0x80 != (buf[3] & 0xc0))) {
                break;
            }
            uint32_t idx = ((buf[0] & 0x7) << 18) | ((buf[1] & 0x3f) << 12) | ((buf[2] & 0x3f) << 6) | (buf[3] & 0x3f);
            gbk_char_t gbkc = gbk_char_from_utf8(idx);
            print_gbk_char(gbkc);
        } else if (0xe0 == (ch & 0xf0)) {
            size_t len = fread(buf + 1, 1, 2, fp);
            if (len != 2) {
                break;
            }
            if ((0x80 != (buf[1] & 0xc0)) ||
                (0x80 != (buf[2] & 0xc0))) {
                break;
            }
            uint32_t idx = ((uint32_t)(buf[0] & 0xf) << 12) | ((uint32_t)(buf[1] & 0x3f) << 6) | (uint32_t)(buf[2] & 0x3f);
            gbk_char_t gbkc = gbk_char_from_utf8(idx);
            print_gbk_char(gbkc);
        } else if (0xc0 == (ch & 0xe0)) {
            size_t len = fread(buf + 1, 1, 1, fp);
            if (len != 1) {
                break;
            }
            if ((0x80 != (buf[1] & 0xc0))) {
                break;
            }
            uint32_t idx = ((buf[0] & 0x1f) << 6) | (buf[1] & 0x3f);
            gbk_char_t gbkc = gbk_char_from_utf8(idx);
            print_gbk_char(gbkc);
        }
    }
    fclose(fp);
    fp = NULL;
    return 0;
}
