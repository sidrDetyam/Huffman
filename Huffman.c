
#include "Huffman.h"


void set_bit(uchar* byte, uchar i, uchar val){
    assert(0<=i && i<8 && (val==0 || val==1));
    *byte = *byte & ~(1<<i) | (val<<i);
}


uchar get_bit(uchar byte, uchar i){
    assert(0<=i && i<8);
    return (byte & (1<<i)) != 0;
}


size_t* count_bytes(FILE* fin, size_t* cnt_n0){

    size_t *counts = (size_t*) malloc(sizeof(size_t)*0x100);
    assert(counts);
    for(size_t i=0; i<0x100; ++i)
        counts[i] = 0;
    uchar ibuff[IBUFF_SIZE];
    *cnt_n0 = 0;

    assert(!fseek(fin, 0, SEEK_SET));
    size_t cnt_read = fread(ibuff, sizeof(uchar), IBUFF_SIZE, fin);
    while(cnt_read) {
        for (size_t i = 0; i < cnt_read; ++i)
            *cnt_n0 += counts[ibuff[i]]++ == 0;

        cnt_read = fread(ibuff, sizeof(uchar), IBUFF_SIZE, fin);
    }

    return counts;
}


void swap(void* a, void* b, size_t size){
    for(uchar* ptr_a=a, *ptr_b=b; size--; ++ptr_a, ++ptr_b){
        uchar c = *ptr_a;
        *ptr_a = *ptr_b;
        *ptr_b = c;
    }
}


size_t find_min(THuffman_tree* arr, size_t count){
    size_t mn = 0;
    for(size_t i=0; i<count; ++i) {
        if (arr[i].count < arr[mn].count)
            mn = i;
    }
    return mn;
}


THuffman_tree* build_Huffman_tree(FILE* fin){

    size_t cnt_n0;
    size_t* counts = count_bytes(fin, &cnt_n0);
    THuffman_tree* heap = (THuffman_tree*) malloc(sizeof(THuffman_tree)*cnt_n0);
    assert(heap);
    for(size_t i=0, j=0; i<0x100; ++i){
        if(counts[i]) {
            heap[j].left = heap[j].right = NULL;
            heap[j].count = counts[i];
            heap[j].symbol = (uchar) i;
            ++j;
        }
    }
    free(counts);

    for(size_t i=0; i<cnt_n0-1; ++i){

        size_t mn = find_min(heap, cnt_n0-i);
        swap(heap, heap+mn, sizeof(THuffman_tree));
        size_t mn2 = find_min(heap+1, cnt_n0-1-i) + 1; //!
        swap(heap+mn2, heap+cnt_n0-1-i, sizeof(THuffman_tree));

        THuffman_tree* l = (THuffman_tree*) malloc(sizeof(THuffman_tree));
        THuffman_tree* r = (THuffman_tree*) malloc(sizeof(THuffman_tree));
        assert(l && r);
        *l = heap[0];
        *r = heap[cnt_n0-1-i];
        heap[0].count = l->count+r->count;
        heap[0].right = r;
        heap[0].left = l;
    }

    THuffman_tree* res = (THuffman_tree*) malloc(sizeof(THuffman_tree));
    assert(res);
    *res = heap[0];
    free(heap);
    return res;
}


void delete_Huffman_tree(THuffman_tree* tree){

    if(tree->left)
        delete_Huffman_tree(tree->left);
    if(tree->right)
        delete_Huffman_tree(tree->right);

    free(tree);
}


void dfs_for_encoding_table(THuffman_tree* node, uchar** table, uchar* path, uchar deep){

    if(node->left==NULL){
        table[node->symbol][0] = deep;
        for(size_t i=1; i<=deep; ++i)
            table[node->symbol][i] = path[i-1];
        return;
    }

    path[deep] = 0;
    dfs_for_encoding_table(node->left, table, path, deep+1);
    path[deep] = 1;
    dfs_for_encoding_table(node->right, table, path, deep+1);
}


uchar** encoding_table(THuffman_tree* tree){

    uchar** table = (uchar**)malloc(sizeof(uchar*)*0x100);
    assert(table);
    for(size_t i=0; i<0x100; ++i) {
        table[i] = (uchar *) malloc(sizeof(uchar)*0x100);
        table[i][0] = 0;
    }

    uchar* path = (uchar*) malloc(sizeof(uchar)*0x100);
    assert(path);
    dfs_for_encoding_table(tree, table, path, 0);
    free(path);

    return table;
}


void zip_tree(THuffman_tree* tree, uchar* bins, size_t *pos){

    if(tree->left==NULL) {
        set_bit(bins + *pos/8, *pos % 8, 1);
        ++(*pos);
        for(size_t i=0; i<8; ++i){
            set_bit(bins + *pos/8, *pos % 8, get_bit(tree->symbol, i));
            ++(*pos);
        }
        return;
    }

    set_bit(bins + *pos/8, *pos % 8, 0);
    ++(*pos);
    zip_tree(tree->left, bins, pos);
    zip_tree(tree->right, bins, pos);
}


void unzip_tree(THuffman_tree* tree, uchar* bins, size_t *pos){

    uchar bit = get_bit(bins[*pos/8], *pos%8);
    ++*pos;

    if(bit==0){
        tree->left = (THuffman_tree*) malloc(sizeof(THuffman_tree));
        tree->right = (THuffman_tree*) malloc(sizeof(THuffman_tree));
        assert(tree->left && tree->right);
        unzip_tree(tree->left, bins, pos);
        unzip_tree(tree->right, bins, pos);
        return;
    }

    tree->left = tree->right = NULL;
    for(size_t i=0; i<8; ++i){
        set_bit(&tree->symbol, i, get_bit(bins[*pos/8], *pos%8));
        ++*pos;
    }
}


size_t ceil_(size_t a, size_t b){
    return a/b + (a%b!=0);
}


size_t fsize(FILE* fin){
    size_t pos = ftell(fin);
    assert(!fseek(fin, 0, SEEK_END));
    size_t s = ftell(fin);
    assert(!fseek(fin, pos, SEEK_SET));
    return s;
}


void zip(FILE* fin, FILE* fout){

    if(fsize(fin)==0)
        return;

    THuffman_tree* tree = build_Huffman_tree(fin);
    uchar** table = encoding_table(tree);
    size_t cnt_n0 = 0;
    for(size_t i=0; i<0x100; ++i) {
        if (table[i][0])
            ++cnt_n0;
    }
    uchar cnt_n0_ = cnt_n0-1; //!

    ulong fsize = ftell(fin);
    assert(!fseek(fin, 0, SEEK_SET));

    assert(fwrite(&fsize, sizeof(ulong), 1, fout));
    assert(fwrite(&cnt_n0_, sizeof(uchar), 1, fout));
    uchar* bin = (uchar*) malloc(sizeof(uchar) * ceil_((size_t)cnt_n0*10-1, 8));
    size_t *pos = (size_t*)malloc(sizeof(size_t));
    assert(pos && bin);
    *pos = 0;
    zip_tree(tree, bin, pos);
    assert(fwrite(bin, sizeof(uchar), ceil_((size_t)cnt_n0*10-1, 8), fout));
    free(bin);
    free(pos);
    delete_Huffman_tree(tree);

    uchar ibuff[IBUFF_SIZE], obuff[OBUFF_SIZE];
    size_t cnt_read = fread(ibuff, sizeof(uchar), IBUFF_SIZE, fin);
    size_t pos_ = 0;
    while(cnt_read){

        for(size_t i=0; i<cnt_read; ++i){
            for(size_t j=0; j<table[ibuff[i]][0]; ++j){
                set_bit(obuff + pos_/8, pos_%8, table[ibuff[i]][j+1]);
                ++pos_;

                if(pos_ == OBUFF_SIZE*8){
                    pos_ = 0;
                    assert(fwrite(obuff, sizeof(uchar), OBUFF_SIZE, fout));
                }
            }
        }

        cnt_read = fread(ibuff, sizeof(uchar), IBUFF_SIZE, fin);
    }

    assert(fwrite(obuff, sizeof(uchar), pos_/8 + (pos_%8!=0), fout));
    for(size_t i = 0; i<0x100; ++i)
        free(table[i]);
    free(table);
}


void unzip(FILE* fin, FILE* fout){

    if(fsize(fin)==0)
        return;

    THuffman_tree* tree = (THuffman_tree*) malloc(sizeof(THuffman_tree));
    ulong fsize;
    uchar cnt_n0;
    assert(fread(&fsize, sizeof(ulong), 1, fin));
    assert(fread(&cnt_n0, sizeof(uchar), 1, fin));
    uchar* bin = (uchar*) malloc(sizeof(uchar) * ceil_(((size_t)cnt_n0+1)*10-1, 8));
    size_t *pos = (size_t*) malloc(sizeof(size_t));
    *pos = 0;
    assert(bin && pos);
    assert(fread(bin, sizeof(uchar), ceil_(((size_t)cnt_n0+1)*10-1, 8), fin));
    unzip_tree(tree, bin, pos);
    free(bin);
    free(pos);

    size_t pos_i = IBUFF_SIZE*8;
    size_t pos_o = 0;
    uchar ibuff[IBUFF_SIZE], obuff[OBUFF_SIZE];
    for(size_t i=0; i<fsize; ++i){

        THuffman_tree* it = tree;
        while(it->left!=NULL){
            if(pos_i==IBUFF_SIZE*8){
                pos_i = 0;
                assert(fread(ibuff, sizeof(uchar), IBUFF_SIZE, fin));
            }

            uchar bit = get_bit(ibuff[pos_i/8], pos_i%8);
            it = bit? it->right : it->left;
            ++pos_i;
        }

        obuff[pos_o] = it->symbol;
        ++pos_o;
        if(pos_o==OBUFF_SIZE){
            pos_o = 0;
            assert(fwrite(obuff, sizeof(uchar), OBUFF_SIZE, fout));
        }
    }

    assert(fwrite(obuff, sizeof(uchar), pos_o, fout));
    delete_Huffman_tree(tree);
}
