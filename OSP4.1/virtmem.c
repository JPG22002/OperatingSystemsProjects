#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PAGES 1000
#define MAX_FRAMES 500

typedef struct {
    int frame_number;
    int in_memory;
    int last_access_time; // for LRU, to keep track of the last access time
} PageTableEntry;

PageTableEntry page_table[MAX_PAGES];
int fifo_queue[MAX_FRAMES];
int current_time = 0;

// initialize the memory management structures
void initialize_memory_management(int nframes) {
    // initialize the page table entries
    for (int i = 0; i < MAX_PAGES; i++) {
        page_table[i].frame_number = -1;
        page_table[i].in_memory = 0;
        page_table[i].last_access_time = -1;
    }
    // initialize the FIFO queue
    for (int i = 0; i < nframes; i++) {
        fifo_queue[i] = -1;
    }
}
//WOOO IT WORKS
// find a victim frame using FIFO page replacement strategy
int find_fifo_victim_frame(int nframes) {
    int victim_frame = fifo_queue[current_time % nframes];
    fifo_queue[current_time % nframes] = -1; // clear the frame from FIFO queue
    current_time++;
    return victim_frame;
}

// find a victim frame using LRU page replacement strategy
int find_lru_victim_frame(int nframes) {
    int lru_frame = -1, oldest_time = current_time;
    // find the page with the oldest last access time
    for (int i = 0; i < MAX_PAGES; i++) {
        if (page_table[i].in_memory && page_table[i].last_access_time < oldest_time) {
            oldest_time = page_table[i].last_access_time;
            lru_frame = page_table[i].frame_number;
        }
    }
    return lru_frame;
}

// handle a page fault by replacing a victim frame
void page_fault_handler(int page_number, int nframes, char* page_replacement_strategy) {
    int victim_frame = -1;

    // determine the victim frame based on the page replacement strategy
    if (strcmp(page_replacement_strategy, "rand") == 0) {
        victim_frame = rand() % nframes;
    } else if (strcmp(page_replacement_strategy, "fifo") == 0) {
        victim_frame = find_fifo_victim_frame(nframes);
    } else if (strcmp(page_replacement_strategy, "lru") == 0) {
        victim_frame = find_lru_victim_frame(nframes);
    }

    if (victim_frame == -1) {
        // ff all frames are empty, find the first available frame
        for (int i = 0; i < nframes; i++) {
            if (fifo_queue[i] == -1) {
                victim_frame = i;
                break;
            }
        }
    }

    if (victim_frame == -1) {
        printf("Error: No victim frame found.\n");
        exit(1);
    }

    // evict the page in the victim frame from memory
    for (int i = 0; i < MAX_PAGES; i++) {
        if (page_table[i].frame_number == victim_frame) {
            page_table[i].in_memory = 0;
            break;
        }
    }

    // load the new page into the victim frame
    page_table[page_number].frame_number = victim_frame;
    page_table[page_number].in_memory = 1;
    page_table[page_number].last_access_time = current_time;

    // update FIFO queue for the new frame
    if (strcmp(page_replacement_strategy, "fifo") == 0) {
        fifo_queue[victim_frame] = page_number;
    }

    current_time++;
}
//special thanks to stackoverflow for helping me with this lol
// function to generate the page reference pattern based on the locality
void generate_page_reference_pattern(int* pattern, int nrefs, int npages, char* locality) {
    int range = (strcmp(locality, "ll") == 0) ? npages :
                (strcmp(locality, "ml") == 0) ? npages / 20 :
                npages / 33; // For 'hl'

    int last_page = rand() % npages;
    for (int i = 0; i < nrefs; i++) {
        if (strcmp(locality, "ll") == 0) {
            pattern[i] = rand() % npages;
        } else {
            int offset = rand() % (2 * range + 1) - range;
            pattern[i] = (last_page + offset + npages) % npages;
            last_page = pattern[i];
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 6) {
        printf("Usage: virtmem npages nframes rand|fifo|lru nrefs ll|ml|hl\n");
        return 1;
    }

    int npages = atoi(argv[1]);
    int nframes = atoi(argv[2]);
    char* page_replacement_strategy = argv[3];
    int nrefs = atoi(argv[4]);
    char* locality = argv[5];

    // check for valid input params
    if (npages < 0 || npages > MAX_PAGES || nframes < 0 || nframes > MAX_FRAMES || nrefs < 0 || nrefs > 5000) {
        printf("Error: Invalid parameters.\n");
        return 1;
    }

    srand(time(NULL)); // seed the random number gen
    initialize_memory_management(nframes);

    int* page_references = (int*)malloc(sizeof(int) * nrefs);
    if (page_references == NULL) {
        printf("Error: Memory allocation failed.\n");
        return 1;
    }
//THE LRU FINALLY WORKS (i think) lol
    // generate the page reference pattern
    generate_page_reference_pattern(page_references, nrefs, npages, locality);

    int page_faults = 0;
    for (int i = 0; i < nrefs; i++) {
        int page_number = page_references[i];
        if (!page_table[page_number].in_memory) {
            page_faults++;
            page_fault_handler(page_number, nframes, page_replacement_strategy);
        }
    }

    printf("Total number of page faults: %d\n", page_faults);

    free(page_references); // free allocated memory
    return 0;//BOOM
}