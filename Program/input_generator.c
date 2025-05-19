#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <dirent.h>

#define MAX_LINE 1024
#define COLOR_CYAN 11
#define COLOR_GREEN 10
#define COLOR_RED 12
#define COLOR_YELLOW 14
#define COLOR_MAGENTA 13
#define COLOR_RESET 7
#define MAX_FILES 100

typedef struct {
    char filename[100];
} FormulaFile;

void clear_screen() {
    system("cls");
}

void set_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void print_credits() {
    set_color(COLOR_CYAN);
    printf("\n");
    printf("███████╗███████╗██╗    ███████╗██╗  ██╗    ██╗███╗   ██╗██╗\n");
    printf("██╔════╝██╔════╝██║    ██╔════╝╚██╗██╔╝    ██║████╗  ██║██║\n");
    printf("█████╗  ███████╗██║    █████╗   ╚███╔╝     ██║██╔██╗ ██║██║\n");
    printf("██╔══╝  ╚════██║██║    ██╔══╝   ██╔██╗     ██║██║╚██╗██║██║\n");
    printf("███████╗███████║██║    ███████╗██╔╝ ██╗    ██║██║ ╚████║██║\n");
    printf("╚══════╝╚══════╝╚═╝    ╚══════╝╚═╝  ╚═╝    ╚═╝╚═╝  ╚═══╝╚═╝\n");
    
    set_color(COLOR_YELLOW);
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║                   Developed by:                        ║\n");
    set_color(COLOR_GREEN);
    printf("║           LASFER  •  MOUSLIM  •  HECHEHOUCHE         ║\n");
    set_color(COLOR_YELLOW);
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    set_color(COLOR_MAGENTA);
    printf("\n              ESI-EX-INI Project - 2024                \n");
    printf("               Advanced Logic Project                   \n\n");
    set_color(COLOR_RESET);
}

void print_fancy_header() {
    set_color(COLOR_CYAN);
    printf("\n╔═════════════════════════════════════════╗\n");
    printf("║      Logic Formula Input Generator      ║\n");
    printf("╚═════════════════════════════════════════╝\n\n");
    set_color(COLOR_RESET);
}

void print_menu() {
    set_color(COLOR_CYAN);
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║             MAIN MENU                 ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");
    
    set_color(COLOR_YELLOW);
    printf("   [1] "); 
    set_color(COLOR_RESET);
    printf("Create New Formula\n");
    
    set_color(COLOR_YELLOW);
    printf("   [2] ");
    set_color(COLOR_RESET);
    printf("Test Existing Formula\n");
    
    set_color(COLOR_YELLOW);
    printf("   [3] ");
    set_color(COLOR_RESET);
    printf("Show Available Formulas\n");
    
    set_color(COLOR_RED);
    printf("   [4] ");
    set_color(COLOR_RESET);
    printf("Exit\n\n");
    
    set_color(COLOR_CYAN);
    printf("Enter your choice (1-4): ");
    set_color(COLOR_RESET);
}

void create_new_formula() {
    char filename[100];
    int num_clauses;
    char input[MAX_LINE];
    FILE *file;

    clear_screen();
    print_credits();
    
    set_color(COLOR_CYAN);
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║         CREATE NEW FORMULA           ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");
    set_color(COLOR_RESET);

    printf("Enter the formula name (without .cnf extension): ");
    scanf("%s", filename);
    getchar();

    // Add .cnf extension
    char full_filename[120];
    sprintf(full_filename, "%s.cnf", filename);

    file = fopen(full_filename, "w");
    if (!file) {
        set_color(COLOR_RED);
        printf("Error: Could not create file %s\n", full_filename);
        set_color(COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    printf("Enter the number of clauses: ");
    scanf("%d", &num_clauses);
    getchar();

    fprintf(file, "# Formula created by user input\n");

    for (int i = 0; i < num_clauses; i++) {
        set_color(COLOR_CYAN);
        printf("\nEnter clause %d (use space between literals, ! for negation):\n", i + 1);
        printf("Example: P !Q R\n> ");
        set_color(COLOR_RESET);
        
        if (fgets(input, MAX_LINE, stdin)) {
            input[strcspn(input, "\n")] = 0;
            fprintf(file, "%s\n", input);
        }
    }

    fclose(file);

    set_color(COLOR_GREEN);
    printf("\n✓ Formula saved to %s\n", full_filename);
    set_color(COLOR_RESET);

    printf("\nWould you like to test this formula now? (y/n): ");
    char choice;
    scanf(" %c", &choice);
    getchar();

    if (choice == 'y' || choice == 'Y') {
        char command[200];
        sprintf(command, "logic_solver %s", full_filename);
        system(command);
    }

    printf("\nPress Enter to continue...");
    getchar();
}

void test_existing_formula() {
    DIR *d;
    struct dirent *dir;
    FormulaFile files[MAX_FILES];
    int count = 0;
    
    clear_screen();
    print_credits();
    
    set_color(COLOR_CYAN);
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║         TEST EXISTING FORMULA        ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");
    set_color(COLOR_RESET);

    // List all CNF files with numbers
    d = opendir(".");
    if (d) {
        printf("Available formulas:\n\n");
        while ((dir = readdir(d)) != NULL && count < MAX_FILES) {
            if (strstr(dir->d_name, ".cnf")) {
                strcpy(files[count].filename, dir->d_name);
                set_color(COLOR_YELLOW);
                printf("[%d] ", count + 1);
                set_color(COLOR_RESET);
                printf("%s\n", files[count].filename);
                count++;
            }
        }
        closedir(d);
    }

    if (count == 0) {
        set_color(COLOR_RED);
        printf("No CNF formula files found!\n");
        set_color(COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    printf("\nEnter the number of the formula to test (1-%d): ", count);
    int choice;
    while (scanf("%d", &choice) != 1 || choice < 1 || choice > count) {
        set_color(COLOR_RED);
        printf("Invalid choice! Please enter a number between 1 and %d: ", count);
        set_color(COLOR_RESET);
        while (getchar() != '\n'); // Clear input buffer
    }
    getchar(); // consume newline

    char command[200];
    sprintf(command, "logic_solver %s", files[choice - 1].filename);
    system(command);

    printf("\nPress Enter to continue...");
    getchar();
}

void show_formulas() {
    DIR *d;
    struct dirent *dir;
    FormulaFile files[MAX_FILES];
    int count = 0;
    
    while (1) {
        clear_screen();
        print_credits();
        
        set_color(COLOR_CYAN);
        printf("\n╔═══════════════════════════════════════╗\n");
        printf("║         AVAILABLE FORMULAS           ║\n");
        printf("╚═══════════════════════════════════════╝\n\n");
        set_color(COLOR_RESET);

        d = opendir(".");
        if (d) {
            while ((dir = readdir(d)) != NULL && count < MAX_FILES) {
                if (strstr(dir->d_name, ".cnf")) {
                    strcpy(files[count].filename, dir->d_name);
                    set_color(COLOR_YELLOW);
                    printf("[%d] ", count + 1);
                    set_color(COLOR_RESET);
                    printf("%s\n", files[count].filename);
                    count++;
                }
            }
            closedir(d);
        }

        if (count == 0) {
            set_color(COLOR_RED);
            printf("No CNF formula files found!\n");
            set_color(COLOR_RESET);
        }

        printf("\nOptions:\n");
        set_color(COLOR_YELLOW);
        printf("[1] ");
        set_color(COLOR_RESET);
        printf("Test a formula\n");
        set_color(COLOR_RED);
        printf("[2] ");
        set_color(COLOR_RESET);
        printf("Return to main menu\n\n");
        
        printf("Enter your choice (1-2): ");
        char menu_choice;
        scanf(" %c", &menu_choice);
        getchar();

        if (menu_choice == '1') {
            if (count > 0) {
                printf("\nEnter the number of the formula to test (1-%d): ", count);
                int file_choice;
                while (scanf("%d", &file_choice) != 1 || file_choice < 1 || file_choice > count) {
                    set_color(COLOR_RED);
                    printf("Invalid choice! Please enter a number between 1 and %d: ", count);
                    set_color(COLOR_RESET);
                    while (getchar() != '\n'); // Clear input buffer
                }
                getchar(); // consume newline

                char command[200];
                sprintf(command, "logic_solver %s", files[file_choice - 1].filename);
                system(command);
                
                printf("\nPress Enter to continue...");
                getchar();
            } else {
                set_color(COLOR_RED);
                printf("\nNo formulas available to test!\n");
                set_color(COLOR_RESET);
                printf("\nPress Enter to continue...");
                getchar();
            }
        } else if (menu_choice == '2') {
            return;
        }
        count = 0; // Reset count for next iteration
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    char choice;

    while (1) {
        clear_screen();
        print_credits();
        print_menu();

        scanf(" %c", &choice);
        getchar(); // consume newline

        switch (choice) {
            case '1':
                create_new_formula();
                break;
            case '2':
                test_existing_formula();
                break;
            case '3':
                show_formulas();
                break;
            case '4':
                clear_screen();
                print_credits();
                set_color(COLOR_GREEN);
                printf("\nThank you for using ESI-EX-INI Logic Solver!\n");
                set_color(COLOR_RESET);
                return 0;
            default:
                set_color(COLOR_RED);
                printf("\nInvalid choice! Press Enter to continue...");
                set_color(COLOR_RESET);
                getchar();
        }
    }

    return 0;
} 