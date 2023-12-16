#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RANGE 200
#define HEIGHT 50


typedef struct {
    int year, month, day;
} Date;

typedef struct {
    Date date;
    double open, close;
    double high, low;
    int volume;
} Flare;

int main(int argc,char *argv[]) {
    if (argc < 2) { puts("No file provided."); exit(1); }
    
    FILE *file_fd = fopen(argv[1], "r");
    if (!file_fd) { puts("Incorrect file name provided."); exit(1); }

    // Find size of file in number of chars
    fseek(file_fd, 0, SEEK_END);
    int buffer_s = ftell(file_fd);
    rewind(file_fd);

    char *buffer = (char*)malloc(sizeof(char) * buffer_s);
    if (!buffer) { puts("Memory allocation error!"); exit(1); }

    int ok = fread(buffer, 1, buffer_s, file_fd);
    if (!ok) { puts("Error while reading file."); exit(1); }
    fclose(file_fd);

    char *token;
    token = strtok(buffer, ",\n");

    // Count number of lines
    int data_s = 0;
    for (int i = 0; i < buffer_s; i++) { if (buffer[i] == '\n') { data_s++; } }

    // Allocate apropriate amount of memory
    Flare *data = (Flare*)malloc(sizeof(Flare) * data_s);
    if (!data) { puts("Memory allocation error!"); exit(1); }

    // Skip first row
    strtok(NULL, "\n");

    for (int i = 0; i < data_s - 1; i++) {
        char* line = strtok(NULL, "\n");
        // Parse tokens
        int year, month, day;
        double open, close;
        double high, low;
        double volume;

        sscanf(line, "%4d-%2d-%2d,%lf,%lf,%lf,%lf,%lf", &year, &month, &day, &open, &close, &high, &low, &volume);
        data[i] = (Flare){{year, month, day}, open, close, high, low, volume};

        // printf("[DEBUG] %4d-%2d-%2d: %lf %lf %lf %lf %lf\n", year, month, day, open, close, high, low, volume);
    }

    for (int i = 0; i < data_s - 1; i++) {
        int year  = data[i].date.year;
        int month = data[i].date.month;
        int day   = data[i].date.day;
        double open   = data[i].open;
        double close  = data[i].close;
        double high   = data[i].high;
        double low    = data[i].low;
        double volume = data[i].volume;

        printf("[DEBUG] %4d-%02d-%02d: %lf %lf %lf %lf %lf\n", year, month, day, open, close, high, low, volume);
    }

    return 0;
}