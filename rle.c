//Runlength compression in C
// Usage: ./rle <input file> <output file> <runlength> <mode>
//        Where mode=0 is compress and mode=1 is decompress
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void errorExit(const char *message) {
    perror(message);
    exit(-1);
}

void writeRun(int out_fd, const unsigned char *pattern, int k, int count) {
    while (count > 255) {
        unsigned char byte_count = 255;
        if (write(out_fd, &byte_count, 1) < 0 || write(out_fd, pattern, k) < 0)
            errorExit("Error writing compressed data");
        count -= 255;
    }
    unsigned char byte_count = count;
    if (write(out_fd, &byte_count, 1) < 0 || write(out_fd, pattern, k) < 0)
        errorExit("Error writing compressed data");
}

void compress(const char *input_file, const char *output_file, int k) {
    int in_fd = open(input_file, O_RDONLY);
    if (in_fd < 0) errorExit("Error opening input file");
    
    int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (out_fd < 0) {
        close(in_fd);
        errorExit("Error opening output file");
    }
    
    unsigned char buffer[k], prev[k];
    ssize_t bytes_read;
    int count = 0, first = 1;
    
    while ((bytes_read = read(in_fd, buffer, k)) > 0) {
        if (first || memcmp(buffer, prev, k) != 0) {
            if (!first) writeRun(out_fd, prev, k, count);
            memcpy(prev, buffer, k);
            count = 1;
            first = 0;
        } else {
            count++;
        }
    }
    
    if (bytes_read < 0) errorExit("Error reading input file");
    if (!first) writeRun(out_fd, prev, k, count);
    
    close(in_fd);
    close(out_fd);
}

void decompress(const char *input_file, const char *output_file, int k) {
    int in_fd = open(input_file, O_RDONLY);
    if (in_fd < 0) errorExit("Error opening input file");
    
    int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (out_fd < 0) {
        close(in_fd);
        errorExit("Error opening output file");
    }
    
    unsigned char buffer[k], count;
    ssize_t bytes_read;
    
    while ((bytes_read = read(in_fd, &count, 1)) > 0) {
        if (read(in_fd, buffer, k) != k) errorExit("Error reading pattern");
        for (int i = 0; i < count; i++) {
            if (write(out_fd, buffer, k) < 0) errorExit("Error writing decompressed data");
        }
    }
    
    if (bytes_read < 0) errorExit("Error reading input file");
    
    close(in_fd);
    close(out_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input file> <output file> <compression length> <mode>\n", argv[0]);
        return -1;
    }
    
    int k = atoi(argv[3]);
    int mode = atoi(argv[4]);
    
    if (k < 1) {
        fprintf(stderr, "Compression length must be at least 1\n");
        return -1;
    }
    if (mode != 0 && mode != 1) {
        fprintf(stderr, "Mode must be 0 or 1\n");
        return -1;
    }
    
    if (mode == 0) {
        compress(argv[1], argv[2], k);
    } else {
        decompress(argv[1], argv[2], k);
    }
    
    return 0;
}
