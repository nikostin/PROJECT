#include <stdio.h>
#include <string.h>

int partA_main(int argc, char *argv[]);
int partB_main(int argc, char *argv[]);
int assignment2_main(int argc, char *argv[]); // Declaration for the second assignment's main function

int main(int argc, char *argv[]) {
    if (strcmp(argv[0], "./lsh") == 0 || strcmp(argv[0], "./cube") == 0) {
        return partA_main(argc, argv); // Call the main function from partA.c
    } else if (strcmp(argv[0], "./cluster") == 0) {
        return partB_main(argc, argv); // Call the main function from partB.c
    } else if (strcmp(argv[0], "./graph_search") == 0) {
        return assignment2_main(argc, argv); // Call the main function for the second assignment
    } else {
        fprintf(stderr, "Invalid command.\n");
        return 1;
    }
}
