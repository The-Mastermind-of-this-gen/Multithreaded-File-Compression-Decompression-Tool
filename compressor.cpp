#include "compressor.h"
#include "HuffmanTree.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <bitset>
#include <ctime>

using namespace std;

// Return codes (optional)
#define FileCompressedSuccessfully 0
#define CouldNotOpenFile 1
#define CouldNotCreateCompressionFile 2

int Compress(const char* filename, long BlockSize) {
    // Open the file
    FILE* file = fopen(filename, "rb");
    if (!file) return CouldNotOpenFile;

    Huffman* a = new Huffman(file);
    TreeNode<unsigned char>* HuffmanTreeRoot = a->ConstructHuffmanTree();

    string CodesArray[256];
    a->GenerateCodes(HuffmanTreeRoot, "", CodesArray);
    delete a;

    fseek(file, 0, SEEK_END);
    long totalFileSize = ftell(file);
    rewind(file);

    printf("Compressing %s (%.2f KB)...\n", filename, totalFileSize / 1024.0);

    // Build output filename
    string nameForCompression = string(filename) + ".MohamedHazem";
    FILE* CompressedFile = fopen(nameForCompression.c_str(), "wb");
    if (!CompressedFile) {
        fclose(file);
        return CouldNotCreateCompressionFile;
    }

    // Write code table
    int Table = 0;
    for (int i = 0; i < 256; i++) if (!CodesArray[i].empty()) Table++;
    fwrite(&Table, sizeof(int), 1, CompressedFile);

    for (int i = 0; i < 256; i++) {
        if (!CodesArray[i].empty()) {
            unsigned char symbol = i;
            unsigned char len = CodesArray[i].length();
            fwrite(&symbol, 1, 1, CompressedFile);
            fwrite(" ", 1, 1, CompressedFile);
            fwrite(CodesArray[i].c_str(), 1, len, CompressedFile);
            fwrite("\n", 1, 1, CompressedFile);
        }
    }

    // Compress blocks
    char* block = new char[BlockSize];
    unsigned int bitStorage = 0;
    int bitsCounter = 0;
    size_t bytesRead;
    while ((bytesRead = fread(block, 1, BlockSize, file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            const string& codeStr = CodesArray[(unsigned char)block[i]];
            for (char bit : codeStr) {
                bitStorage = (bitStorage << 1) | (bit - '0');
                bitsCounter++;
                if (bitsCounter == 8) {
                    unsigned char byte = (unsigned char)(bitStorage & 0xFF);
                    fwrite(&byte, sizeof(unsigned char), 1, CompressedFile);
                    bitStorage >>= 8;
                    bitsCounter -= 8;
                }
            }
        }
    }

    // Handle padding
    unsigned char PaddSize = 0;
    if (bitsCounter > 0) {
        PaddSize = 8 - bitsCounter;
        bitStorage <<= PaddSize;
        unsigned char byte = (unsigned char)(bitStorage & 0xFF);
        fwrite(&byte, sizeof(unsigned char), 1, CompressedFile);
    }
    fwrite(&PaddSize, sizeof(unsigned char), 1, CompressedFile);

    delete[] block;
    fclose(file);
    fclose(CompressedFile);

    printf("Done: %s\n", filename);
    return FileCompressedSuccessfully;
}
