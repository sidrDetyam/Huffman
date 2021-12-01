
#include <time.h>
#include "Huffman.h"

int main(int argc, char** argv) {

    size_t tlabel = time(0);
    FILE* fin = fopen(argv[1], "rb");
    FILE* fout = fopen(argv[2], "wb");
    assert(argc==4 && fin && fout && (argv[3][0]=='c' || argv[3][0]=='d'));

    if(argv[3][0]=='c')
        zip(fin, fout);
    else
        unzip(fin, fout);

    printf("\nTime of work %d seconds\n", (int)(time(0)-tlabel));
    if(argv[3][0]=='c')
        printf("Compression ratio %.2f\n", (float)ftell(fin) / ftell(fout));

    fclose(fin);
    fclose(fout);

    return 0;
}
