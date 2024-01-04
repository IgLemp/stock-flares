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

typedef enum {
    RECORD,
    DATE,
} RangeType;
typedef struct {
    RangeType type;
    union {
        struct { int  start, end; } record;
        struct { Date start, end; } date;
    } range;
} Range;

typedef enum {
    DAY,
    WEEK,
    MONTH,
} Span;

#define GREET "\
STOCK FLARES GENERATOR\n\
Igor Lempicki EiT gr.1 200449\n\
"

#define HELP "\
Select an option py typing a letter:\n\
g -> Make a chart form intc_us_data.csv and export it to chart.txt\n\
i -> Set input file name.\n\
o -> Set output file name.\n\
e -> Set chart height.\n\
r -> Select range of exproted data.\n\
c -> Select range type: by date or by entry number.\n\
l -> Load data from input file.\n\
s -> Select span of one bar: day, week or month.\n\
p -> Print chart to output file.\n\
h -> Display help.\n\
q -> Quit.\n\
"

const char *default_in  = "intc_us_data.csv";
const char *default_out = "chart.txt";

// TODO:
// fix graph generation so it loads by date and record ranges

void load_data(FILE *input_fd, int *data_s, Flare **data);
void generate_graph(int data_s, Flare *data, Range range, int height, Span span, char ***graph, int *selected);
void print_graph(FILE *output_fd, char **graph, int selected, int height);
void calculate_selected(int data_s, Flare *data, Range range, int *selected);

// When I look at error handling in this file I start to miss haskell

int main() {
    FILE *input_fd;
    FILE *output_fd;
    Range range = {RECORD, 0, 200};
    int height = 50;
    Span span = DAY;
    int selected = 0;

    int data_s;
    Flare *data;

    char **graph;

    char input_name [1024] = {0};
    char output_name[1024] = {0};
    memcpy(input_name,  default_in,  17);    
    memcpy(output_name, default_out, 10);
    
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

        printf("Range type is: "); if (range.type == RECORD) { printf("record\n"); } else { printf("date\n"); }

        if (range.type == RECORD) { printf("Range is: %i -> %i\n", range.range.record.start, range.range.record.end); }
        else {
            Date rgdt_start = range.range.date.start;
            Date rgdt_end = range.range.date.end;
            printf("Range is: ");
            printf("%04d-%02d-%02d -> ", rgdt_start.year, rgdt_start.month, rgdt_start.day);
            printf("%04d-%02d-%02d\n"  , rgdt_end.year, rgdt_end.month, rgdt_end.day);
        }
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
            // With this scanf()
            // I'm just ignoring trailing '\n'
            // Couldn't find easier way
            scanf("%*c");
            input_fd  = fopen("intc_us_data.csv", "r");
            output_fd = fopen("chart.txt", "w");
            if (!input_fd)  { puts("Couldn't open input file.");  scanf("%*c"); break; }
            if (!output_fd) { puts("Couldn't open output file."); scanf("%*c"); break; }

            load_data(input_fd, &data_s, &data);
            range = (Range){RECORD, data_s - 200, data_s};
            generate_graph(data_s, data, range, height, span, &graph, &selected);
            print_graph(output_fd, graph, selected, height);

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
        case 'e':
            scanf("%*c");
            puts("Provide a height of the graph:");
            scanf("%d", &height);
            cls();
            printf("Height set to: %i\n", height);
            scanf("%*c");
            break;
        case 'r':
            scanf("%*c");
            if (range.type == DATE) {
                Date rgdt_start;
                Date rgdt_end;  
                int matched;

                puts("Provide starting date in format:");
                puts("yyyy-mm-dd");
                fgets(buffer, 255, stdin);
                matched = sscanf(buffer, "%4d-%2d-%2d", &rgdt_start.year, &rgdt_start.month, &rgdt_start.day);
                if (matched < 3) { puts("Incorrect date formatting. Defaulting to 2014-01-01"); rgdt_start = (Date){2014, 1, 1}; }
                if (rgdt_start.month < 1 || rgdt_start.month > 12) { puts("Incorrect month range. Setting to January."); rgdt_start.month = 1; }
                if (rgdt_start.month < 1 || rgdt_start.month > 31) { puts("Incorrect day range. Setting to 1."); rgdt_start.day = 1; }

                puts("Provide ending date in format:");
                puts("yyyy-mm-dd");
                fgets(buffer, 255, stdin);
                matched = sscanf(buffer, "%4d-%2d-%2d", &rgdt_end.year, &rgdt_end.month, &rgdt_end.day);
                if (matched < 3) { puts("Incorrect date formatting. Defaulting to 2018-01-01"); rgdt_end = (Date){2018, 1, 1}; };
                if (rgdt_end.month < 1 || rgdt_end.month > 12) { puts("Incorrect month range. Setting to January."); rgdt_end.month = 1; }
                if (rgdt_end.month < 1 || rgdt_end.month > 31) { puts("Incorrect day range. Setting to 1."); rgdt_end.day = 1; }

                range.range.date.start = rgdt_start;
                range.range.date.end   = rgdt_end;
            } else {
                int rgrc_start;
                int rgrc_end;

                puts("Provide starting record:");
                scanf("%d", &rgrc_start);

                puts("Provide ending record:");
                scanf("%d", &rgrc_end);

                range.range.record.start = rgrc_start;
                range.range.record.end = rgrc_end;
            }
            
            if (data != NULL) {
                calculate_selected(data_s, data, range, &selected);
            }

            scanf("%*c");
            break;
        case 'c':
            scanf("%*c");
            puts("Provide range type: 'd' -> date, 'r' -> records");
            scanf("%c", &input);
            if (input == 'd') { range.type = DATE;   cls(); puts("Changed to date.");    }
            if (input == 'r') { range.type = RECORD; cls(); puts("Changed to records."); }
            else { range.type = DATE; cls(); puts("Incorrect input. Defaulting to date."); }
            scanf("%*c");
            break;
        case 'l':
            scanf("%*c");
            // Yes the file info won't be updated until load
            // I've already spent enough time avoiding doing math
            // and I'm not spending more time fixing this program
            if (data != NULL) { free(data); }
            input_fd  = fopen(input_name,  "r");
            if (!input_fd)  { puts("Couldn't open input file.");  scanf("%*c"); break; }
            load_data(input_fd, &data_s, &data);
            fclose(input_fd);
            puts("Data loaded succesfully.");
            scanf("%*c");
            break;
        case 's':
            scanf("%*c");
            puts("Provide a span of one column: d -> day, w -> week, m -> month");
            scanf("%c", &input);
            switch (input) {
            case 'd': span = DAY;   break;
            case 'w': span = WEEK;  break;
            case 'm': span = MONTH; break;
            default: puts("Incorrect input. Defaulting to day."); span = DAY; break;
            }
            scanf("%*c");
            break;
        case 'p':
            scanf("%*c");
            if (data == NULL) { puts("You need to load the data first."); scanf("%*c"); break; }
            if (graph != NULL) { free(graph); }
            generate_graph(data_s, data, range, height, span, &graph, &selected);
            output_fd = fopen(output_name, "w");
            if (!output_fd) { puts("Couldn't open output file."); scanf("%*c"); break; }
            print_graph(output_fd, graph, selected, height);
            fclose(output_fd);
            cls();
            puts("Graph exported succesfully.");
            scanf("%*c");
            break;
        case 'h':
            scanf("%*c");
            puts(HELP);
            scanf("%*c");
            break;
        case 'q': done = true; break;
        default: break;
        }
    }
    
    cls();
    return 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////
}

void calculate_selected(int data_s, Flare *data, Range range, int *selected) {
    // count number of entries selected
    int start_entry = 0;
    int end_entry   = data_s;
    if (range.type == DATE) {
        Date start_date = range.range.date.start;
        Date end_date = range.range.date.end;
        for (int i = 0; i < data_s; i++) {
            Date curr_date = data[i].date;
            if (curr_date.year  >= start_date.year  &&
                curr_date.month >= start_date.month &&
                curr_date.day   >= start_date.day)
                { start_entry = i; break; }
        }

        for (int i = 0; i < data_s; i++) {
            Date curr_date = data[i].date;
            if (curr_date.year  >= end_date.year  &&
                curr_date.month >= end_date.month &&
                curr_date.day   >= end_date.day)
                { end_entry = i; break; }
        }
    } else {
        start_entry = range.range.record.start;
        end_entry = range.range.record.end;
    }

    int len = end_entry - start_entry;
    *selected = len;
}

void load_data(FILE *input_fd, int *data_s, Flare **data) {
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

void generate_graph(int data_s, Flare *data, Range range, int height, Span span, char ***graph_out, int *selected) {
    // count number of entries selected
    int start_entry = 0;
    int end_entry   = data_s;
    if (range.type == DATE) {
        Date start_date = range.range.date.start;
        Date end_date = range.range.date.end;
        for (int i = 0; i < data_s; i++) {
            Date curr_date = data[i].date;
            if (curr_date.year  >= start_date.year  &&
                curr_date.month >= start_date.month &&
                curr_date.day   >= start_date.day)
                { start_entry = i; break; }
        }

        for (int i = 0; i < data_s; i++) {
            Date curr_date = data[i].date;
            if (curr_date.year  >= end_date.year  &&
                curr_date.month >= end_date.month &&
                curr_date.day   >= end_date.day)
                { end_entry = i; break; }
        }
    } else {
        start_entry = range.range.record.start;
        end_entry = range.range.record.end;
    }

    int len = end_entry - start_entry;
    *selected = len;

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

    for (int i = start_entry; i < end_entry; i++) {
        Flare flare = data[i];
        bool falling = flare.open > flare.close;
        double top = (flare.high - lowest) * scale_factor;
        double bot = (flare.low  - lowest) * scale_factor;
        double g_floor, g_ceil;

        if (falling) { g_ceil = (flare.open  - lowest) * scale_factor; g_floor = (flare.close - lowest) * scale_factor; }
        else         { g_ceil = (flare.close - lowest) * scale_factor; g_floor = (flare.open  - lowest) * scale_factor; }

        int gi = i - start_entry;

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

void print_graph(FILE *output_fd, char **graph, int selected, int height) {
    for (int i = (height - 1); i > 0; i--) {
        for (int j = 0; j < selected; j++) {
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
