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

#ifdef __WIN32__
    #define cls() system("cls");
#else
    #define cls() printf("\e[1;1H\e[2J");
#endif

typedef struct {
    int year, month, day;
} Date;

typedef struct {
    Date date;
    double open, close;
    double high, low;
    int volume;
} Flare;

typedef struct {
    int start, end;
} Range;

typedef enum {
    DAY,
    WEEK,
    YEAR,
} Span;

#define GREET "\
STOCK FLARES GENERATOR\n\
Igor Lempicki EiT gr.1 200449\n\
"

void load_data(FILE *input_fd, FILE *output_fd, int *data_s, Flare **data);
void generate_graph(int data_s, Flare *data, Range range, int height, Span span, char ***graph);
void print_graph(FILE *output_fd, char **graph, Range range, int height);

// When I look at error handling in this file I start to miss haskell

int main() {
    FILE *input_fd;
    FILE *output_fd;
    Range range = {0, 0};
    int height = 50;
    Span span = DAY;
    int selected = 0;

    int data_s;
    Flare *data;

    char **graph;

    char input_name [1024] = {0};
    char output_name[1024] = {0};
    
    // for numbers
    char buffer     [256] = {0};

    /// MENU //////////////////////////////////////////////////////////////////////////////////////
    bool done = false;
    char input;

    while (!done) {
        cls();
        puts(GREET);
        printf("Input  file name: %s\n", input_name);
        printf("Output file name: %s\n", output_name);
        printf("Range is: %i -> %i\n", range.start, range.end);
        printf("Records selected: %i\n", selected);
        printf("Height: %i\n", height);
        printf("\n");

        // Clear buffer
        memset(buffer, '\0', 256);
        scanf("%c", &input);
        
        cls();
        puts(GREET);

        switch (input) {
        case 'g':
            // I'm just ignoring trailing '\n'
            // Couldn't find easier way
            scanf("%*c");
            input_fd  = fopen("intc_us_data.csv", "r");
            output_fd = fopen("chart.txt", "w");
            if (!input_fd)  { puts("Error while opening file."); exit(1); }
            if (!output_fd) { puts("Error while opening file."); exit(1); }

            load_data(input_fd, output_fd, &data_s, &data);
            range = (Range){data_s - 200, data_s};
            generate_graph(data_s, data, range, height, span, &graph);
            print_graph(output_fd, graph, range, height);

            free(data);
            free(graph);
            fclose(input_fd);
            fclose(output_fd);

            cls();
            puts("File read: intc_us_data.csv");
            puts("Data written to: chart.txt");
            scanf("%*c");
            break;
        case 'i':
            scanf("%*c");
            puts("Provide input file name:");
            fgets(input_name, 1023, stdin);
            // Because c strings are null terminated I can get away with not clearing the buffer
            for (int i = 0; i < 1024; i++) { if (input_name[i] == '\n') { input_name[i] = '\0'; break; } }
            
            cls();
            puts("File name set to:");
            puts(input_name);
            scanf("%*c");
            break;
        case 'o':
            scanf("%*c");
            puts("Provide output file name:");
            fgets(output_name, 1023, stdin);
            for (int i = 0; i < 1024; i++) { if (output_name[i] == '\n') { output_name[i] = '\0'; break; } }
            
            cls();
            puts("File name set to:");
            puts(output_name);
            scanf("%*c");
            break;
        case 'h':
            scanf("%*c");
            puts("Provide a height of the graph:");
            fgets(buffer, 255, stdin);
            height = atoi(buffer);

            cls();
            printf("Height set to: %i\n", height);
            scanf("%*c");
            break;
        case 'r':
            scanf("%*c");
            fgets(buffer, 255, stdin);
            scanf("%*c");
            break;
        case 's':
            scanf("%*c");
            // scanf("%c", );
            scanf("%*c");
            break;
        case 'l': break;
        case 'p': break;
        case 'q': done = true; break;
        default: break;
        }
    }
    
    cls();
    return 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////
}

void load_data(FILE *input_fd, FILE *output_fd, int *data_s, Flare **data) {
    // Find size of file in number of chars
    fseek(input_fd, 0, SEEK_END);
    int buffer_s = ftell(input_fd);
    rewind(input_fd);

    char *buffer = (char*)malloc(sizeof(char) * buffer_s);
    if (!buffer) { puts("Memory allocation error!"); exit(1); }

    int ok = fread(buffer, 1, buffer_s, input_fd);
    if (!ok) { puts("Error while reading file."); exit(1); }

    char *token;
    token = strtok(buffer, ",\n");

    // Count number of lines
    *data_s = 0;
    for (int i = 0; i < buffer_s; i++) { if (buffer[i] == '\n') { (*data_s)++; } }

    // Allocate apropriate amount of memory
    (*data) = (Flare*)malloc(sizeof(Flare) * (*data_s));
    if (!(*data)) { puts("Memory allocation error!"); exit(1); }

    // Skip first row
    strtok(NULL, "\n");

    for (int i = 0; i < (*data_s) - 1; i++) {
        char* line = strtok(NULL, "\n");
        if (!line) { continue; }

        // Parse lines
        int year, month, day;
        double open, close;
        double high, low;
        double volume;

        sscanf(line, "%4d-%2d-%2d,%lf,%lf,%lf,%lf,%lf", &year, &month, &day, &open, &high, &low, &close, &volume);
        (*data)[i] = (Flare){{year, month, day}, open, close, high, low, volume};

        // printf("[DEBUG] %04d-%02d-%02d: %lf %lf %lf %lf %lf\n", year, month, day, open, high, low, close, volume);
    }

}

void generate_graph(int data_s, Flare *data, Range range, int height, Span span, char ***graph_out) {
    int len = range.end - range.start;
    
    // allocate array for strings
    char **graph;
    graph = (char**)malloc(sizeof(char*) * (len));
    if (graph == NULL) { puts("Memory allocation error!"); exit(1); }

    // allocate said strings, remember null terminator because C
    for (int i = 0; i < len; i++) {
        graph[i] = (char*)malloc(sizeof(char) * (height + 1));
        if (graph[i] == NULL) { puts("Memory allocation error!"); exit(1); }
    }

    // currently temporary restriction
    if (data_s < len) { puts("Minimum number of records is 200"); exit(1); }

    // Find highest and lowest value in range
    double highest = MAX(data[0].high, data[0].low);
    double lowest  = MIN(data[0].high, data[0].low);
    for (int i = 0; i < len; i++) {
        if (highest < data[i].high) { highest = data[i].high; }
        if (highest < data[i].low ) { highest = data[i].low ; }
        if (lowest  > data[i].high) { lowest  = data[i].high; }
        if (lowest  > data[i].low ) { lowest  = data[i].low ; }
    }
    double amplitude = highest - lowest;
    double scale_factor = (height / amplitude);

    for (int i = range.start; i < range.end; i++) {
        Flare flare = data[i];
        bool falling = flare.open > flare.close;
        double top = (flare.high - lowest) * scale_factor;
        double bot = (flare.low  - lowest) * scale_factor;
        double g_floor, g_ceil;

        if (falling) { g_ceil = (flare.open  - lowest) * scale_factor; g_floor = (flare.close - lowest) * scale_factor; }
        else         { g_ceil = (flare.close - lowest) * scale_factor; g_floor = (flare.open  - lowest) * scale_factor; }

        int gi = i - range.start;

        // Set string
        memset(graph[gi], ' ', height);
        graph[gi][height] = '\0';
        
        // Fill string
        for (int j = 0; j < height; j++) {
            if (j > bot && j < g_floor)    { graph[gi][j] = '|'; }
            if (j > g_ceil && j < top)     { graph[gi][j] = '|'; }
            if (j > g_floor && j < g_ceil) { graph[gi][j] = (falling ? 'O' : '#'); }
        }
    }

    *graph_out = graph;
}

void print_graph(FILE *output_fd, char **graph, Range range, int height) {
    int len = range.end - range.start;
    for (int i = (height - 1); i > 0; i--) {
        for (int j = 0; j < len; j++) {
            char c = graph[j][i];
            // if (c == '|') { fprintf(stdout, "│"); }
            // if (c == 'O') { fprintf(stdout, RED("█")); }
            // if (c == '#') { fprintf(stdout, GREEN("█")); }
            // if (c == ' ') { fprintf(stdout, " "); }
            fprintf(output_fd, "%c", graph[j][i]);
        }
        fprintf(output_fd, "\n");
    }
}
