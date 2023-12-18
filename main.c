#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BOLD(s) "\e[1m" s "\e[0m"
#define RED(s) "\e[31m" s "\e[0m"
#define GREEN(s) "\e[32m" s "\e[0m"

typedef struct {
    int year, month, day;
} Date;

typedef struct {
    Date date;
    double open, close;
    double high, low;
    int volume;
} Flare;

// When I look at error handling in this file I start to miss haskell

int main(int argc,char *argv[]) {
    if (argc < 3) { puts("No input or output file provided."); exit(1); }
    
    FILE *input_fd = fopen(argv[1], "r");
    if (!input_fd) { puts("Incorrect input file name provided."); exit(1); }

    FILE *output_fd = fopen(argv[2], "w");
    if (!input_fd) { puts("Incorrect output file name provided."); exit(1); }

    int range = 200;
    int height = 50;
    if (argc < 4) { puts("No height provided. Assuming 50."); }
    else { height = atoi(argv[3]); }

    if (argc < 5) { puts("No range provided. Assuming 200."); }
    else { range = atoi(argv[4]); }

    // Find size of file in number of chars
    fseek(input_fd, 0, SEEK_END);
    int buffer_s = ftell(input_fd);
    rewind(input_fd);

    char *buffer = (char*)malloc(sizeof(char) * buffer_s);
    if (!buffer) { puts("Memory allocation error!"); exit(1); }

    int ok = fread(buffer, 1, buffer_s, input_fd);
    if (!ok) { puts("Error while reading file."); exit(1); }
    fclose(input_fd);

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
        // Parse lines
        int year, month, day;
        double open, close;
        double high, low;
        double volume;

        sscanf(line, "%4d-%2d-%2d,%lf,%lf,%lf,%lf,%lf", &year, &month, &day, &open, &high, &low, &close, &volume);
        data[i] = (Flare){{year, month, day}, open, close, high, low, volume};

        // printf("[DEBUG] %04d-%02d-%02d: %lf %lf %lf %lf %lf\n", year, month, day, open, high, low, close, volume);
    }

    // allocate array for strings
    char **graph = (char**)malloc(sizeof(char*) * range);
    if (!graph) { puts("Memory allocation error!"); exit(1); }

    // allocate said strings, remember null terminator because C
    for (int i; i < range; i++) {
        graph[i] = (char*)malloc(sizeof(char) * (height + 1));
        if (!graph[i]) { puts("Memory allocation error!"); exit(1); }
    }

    // currently temporary restriction
    if (data_s < range) { puts("Minimum number of records is 200"); exit(1); }

    // Find highest and lowest value in range
    double highest = MAX(data[0].high, data[0].low);
    double lowest  = MIN(data[0].high, data[0].low);
    for (int i = 0; i < range; i++) {
        if (highest < data[i].high) { highest = data[i].high; }
        if (highest < data[i].low ) { highest = data[i].low ; }
        if (lowest  > data[i].high) { lowest  = data[i].high; }
        if (lowest  > data[i].low ) { lowest  = data[i].low ; }
    }
    double amplitude = highest - lowest;
    double scale_factor = (height / amplitude);

    for (int i = 0; i < range; i++) {
        Flare flare = data[i];
        bool falling = flare.open > flare.close;
        double top = (flare.high - lowest) * scale_factor;
        double bot = (flare.low  - lowest) * scale_factor;
        double g_floor, g_ceil;

        if (falling) { g_ceil = (flare.open  - lowest) * scale_factor; g_floor = (flare.close - lowest) * scale_factor; }
        else         { g_ceil = (flare.close - lowest) * scale_factor; g_floor = (flare.open  - lowest) * scale_factor; }

        // Set string
        memset(graph[i], ' ', height);
        graph[i][height] = '\0';
        
        // Fill string
        for (int j = 0; j < height; j++) {
            if (j > bot && j < g_floor)    { graph[i][j] = '|'; }
            if (j > g_ceil && j < top)     { graph[i][j] = '|'; }
            if (j > g_floor && j < g_ceil) { graph[i][j] = (falling ? 'O' : '#'); }
        }

        // printf("[DEBUG] %4d-%02d-%02d:            open: %lf   close: %lf   high: %lf   low:   %lf\n", flare.date.year, flare.date.month, flare.date.day, flare.open, flare.close, flare.high, flare.low);
        // printf("[DEBUG] %4d-%02d-%02d: fall: %d    top:  %lf    bot:   %lf    g_ceil: %lf    g_floor: %lf\n", flare.date.year, flare.date.month, flare.date.day, falling, top, bot, ceil, floor);
    }

    for (int i = (height - 1); i > 0; i--) {
        for (int j = (range - 1); j > 0; j--) {
            char c = graph[j][i];
            // if (c == '|') { fprintf(stdout, "│"); }
            // if (c == 'O') { fprintf(stdout, RED("█")); }
            // if (c == '#') { fprintf(stdout, GREEN("█")); }
            // if (c == ' ') { fprintf(stdout, " "); }
            fprintf(output_fd, "%c", graph[j][i]);
            fprintf(stdout, "%c", graph[j][i]);
        }
        fprintf(output_fd, "\n");
        fprintf(stdout, "\n");
    }

    // printf("highest: %lf    lowest: %lf\n", highest, lowest);
    // printf("scale factor: %lf\n", scale_factor);

    // for (int i = 0; i < data_s - 1; i++) {
    //     int year  = data[i].date.year;
    //     int month = data[i].date.month;
    //     int day   = data[i].date.day;
    //     double open   = data[i].open;
    //     double close  = data[i].close;
    //     double high   = data[i].high;
    //     double low    = data[i].low;
    //     double volume = data[i].volume;

    //     printf("[DEBUG] %4d-%02d-%02d: %lf %lf %lf %lf %lf\n", year, month, day, open, close, high, low, volume);
    // }

    return 0;
}