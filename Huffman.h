
#ifndef HUFFMAN_C_LAB
#define HUFFMAN_C_LAB

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef unsigned char uchar;
typedef unsigned long ulong;

void set_bit(uchar* byte, uchar i, uchar val);
uchar get_bit(uchar byte, uchar i);
void swap(void* a, void* b, size_t size);
size_t ceil_(size_t a, size_t b);
size_t fsize(FILE* fin);

#define THuffman_tree struct __THuffman_tree__
THuffman_tree{
    THuffman_tree* left, *right;
    size_t count;
    uchar symbol;
};

#define IBUFF_SIZE 0x400
#define OBUFF_SIZE 0x400

size_t* count_bytes(FILE* fin, size_t* cnt_n0);
size_t find_min(THuffman_tree* arr, size_t count);

THuffman_tree* build_Huffman_tree(FILE* fin);
void delete_Huffman_tree(THuffman_tree* tree);

void dfs_for_encoding_table(THuffman_tree* node, uchar** table, uchar* path, uchar deep);
uchar** encoding_table(THuffman_tree* tree);

void zip_tree(THuffman_tree* tree, uchar* bins, size_t *pos);
void unzip_tree(THuffman_tree* tree, uchar* bins, size_t *pos);

void zip(FILE* fin, FILE* fout);
void unzip(FILE* fin, FILE* fout);

#endif
