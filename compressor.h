#ifndef COMPRESSOR_H
#define COMPRESSOR_H

// prototypes — no default args here
int Compress(long BlockSize, FILE** file, char* name);
int Decompress(long BlockSize, FILE** file, char* name);

// optional return codes (copy from your enum)
enum Status {
    CompressionProcess = 0,
    DeCompressionProcess = 1,
    CouldNotOpenTheFile = -1,
    CouldNotCreateTheCompressionFile = -2,
    CouldNotCreateTheDeCompressionFile = -5,
    DialogWasCanceledOrFailed = -3,
    FileOpendSucessfully = 2,
    FileCompressedSuccessfuly = 99,
    FileDeCompressedSuccessfuly = 100,
    FailedToReadTableSize = -4
};

#endif // COMPRESSOR_H
