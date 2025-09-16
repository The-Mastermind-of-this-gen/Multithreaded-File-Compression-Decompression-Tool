// multithread_driver.cpp
// Compile with: g++ -o compress_mt multithread_driver.cpp compressor_impl.cpp HuffmanTree.cpp Heap.cpp HashMap.cpp -std=c++17 -pthread

#include <pthread.h>
#include <deque>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/stat.h>
#include <dirent.h>
#include "compressor.h"

using namespace std;
using std::string;

static std::deque<std::string> fileQueue;
static pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;
static bool producerDone = false;

static const char* END_SENTINEL = "__END_OF_QUEUE__";

// pushing a path into queue
void enqueue_path(const std::string &p) {
    pthread_mutex_lock(&queueMutex);
    fileQueue.push_back(p);
    pthread_cond_signal(&queueCond);
    pthread_mutex_unlock(&queueMutex);
}

// pop path (caller must free the returned string copy in C++ land)
bool dequeue_path(std::string &out) {
    pthread_mutex_lock(&queueMutex);
    while (fileQueue.empty() && !producerDone) {
        pthread_cond_wait(&queueCond, &queueMutex);
    }
    if (fileQueue.empty() && producerDone) {
        pthread_mutex_unlock(&queueMutex);
        return false;
    }
    out = fileQueue.front();
    fileQueue.pop_front();
    pthread_mutex_unlock(&queueMutex);
    return true;
}

// Utility: get basename (works with / and \)
static std::string get_basename(const std::string &path) {
    size_t pos1 = path.find_last_of('/');
    size_t pos2 = path.find_last_of('\\');
    size_t pos = std::string::npos;
    if (pos1 != std::string::npos && pos2 != std::string::npos) pos = std::max(pos1, pos2);
    else if (pos1 != std::string::npos) pos = pos1;
    else if (pos2 != std::string::npos) pos = pos2;
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

// Worker: opens file, prepares mutable name buffer and calls Compress/Decompress
void process_file(const std::string &path, long BlockSizeBytes) {
    // open file
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        std::cerr << "[worker] Failed to open: " << path << " : " << strerror(errno) << "\n";
        return;
    }

    string base = get_basename(path);

    const size_t NAME_BUF = 4096;
    char namebuf[NAME_BUF];
    memset(namebuf, 0, NAME_BUF);

    strncpy(namebuf, path.c_str(), NAME_BUF - 1);

    // decide compress or decompress by extension
    const std::string COMP_EXT = ".MohamedHazem";
    bool isCompressed = false;
    if (base.length() >= COMP_EXT.length()) {
    if (base.substr(base.length() - COMP_EXT.length()) == COMP_EXT) isCompressed = true;
    }

    int result = 0;
    if (isCompressed) {
        result = Decompress(BlockSizeBytes, &f, namebuf);
        if (result == 0) std::cout << "[worker] Decompressed: " << path << "\n";
        else std::cout << "[worker] Decompress failed for: " << path << " (code " << result << ")\n";
    } else {
        result = Compress(BlockSizeBytes, &f, namebuf);
        if (result == 99) std::cout << "[worker] Compressed: " << path << "\n";
        else std::cout << "[worker] Compress failed for: " << path << " (code " << result << ")\n";
    }
}

// Worker thread function
void* worker_thread(void* arg) {
    long BlockSizeBytes = (long)(intptr_t)arg;
    while (true) {
        std::string path;
        bool ok = dequeue_path(path);
        if (!ok) break; // no more work and producer signaled done
        if (path == END_SENTINEL) break;
        process_file(path, BlockSizeBytes);
    }
    return nullptr;
}

// Helper: check if path is a directory
bool is_directory(const std::string &p) {
    struct stat st;
    if (stat(p.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

// Helper: scan directory (one level) and enqueue regular files
void scan_directory_and_enqueue(const std::string &dirpath) {
    DIR* d = opendir(dirpath.c_str());
    if (!d) {
        std::cerr << "Failed to open directory: " << dirpath << " : " << strerror(errno) << "\n";
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        std::string full = dirpath;
        if (full.back() != '/' && full.back() != '\\') full += "/";
        full += entry->d_name;
        // check if regular file
        struct stat st;
        if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            enqueue_path(full);
        }
    }
    closedir(d);
}

// Usage message
void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [-t threads] [-b blockKB] <file-or-dir> [file-or-dir ...]\n"
              << "  -t threads   number of worker threads (default 4)\n"
              << "  -b blockKB   block size in KB used by compress/decompress (default 4096 KB)\n"
              << "Examples:\n"
              << "  " << prog << " -t 6 file1.txt file2.txt\n"
              << "  " << prog << " -t 4 /mnt/c/Users/HP/Docs/myfiles\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    int numThreads = 4;
    long blockKB = 4096; // default KB
    std::vector<std::string> inputs;

    // parse args
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            numThreads = atoi(argv[++i]);
            if (numThreads < 1) numThreads = 1;
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            blockKB = atol(argv[++i]);
            if (blockKB < 1) blockKB = 1;
        } else {
            inputs.push_back(argv[i]);
        }
    }

    long blockBytes = blockKB * 1024L;

    // Here Iam creating the threads
    std::vector<pthread_t> threads(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&threads[i], nullptr, worker_thread, (void*)(intptr_t)blockBytes);
    }


    for (auto &p : inputs) {
        if (is_directory(p)) {
            scan_directory_and_enqueue(p);
        } else {
            enqueue_path(p);
        }
    }


    pthread_mutex_lock(&queueMutex);
    producerDone = true;
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);

    
    for (auto &t : threads) pthread_join(t, nullptr);

    return 0;
}
