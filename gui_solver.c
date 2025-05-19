#include <windows.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_FILES 100
#define MAX_OUTPUT 4096
#define ID_CREATE_FORMULA 1001
#define ID_TEST_FORMULA 1002
#define ID_SHOW_FORMULAS 1003
#define ID_EXIT 1004
#define ID_SUBMIT_CLAUSES 1005
#define ID_FILE_LIST 1006
#define ID_TEST_SELECTED 1007
#define ID_BACK 1008
#define ID_CREDITS 1009
#define ID_RESULT_DIALOG 1010
#define ID_FORMULA_PREVIEW 1011

typedef struct {
    char filename[100];
} FormulaFile;

HFONT hFont, hFontBig;
HWND mainWindow, createButton, testButton, showButton, exitButton, creditsButton;
HWND clauseCountInput, clauseInput, submitButton, fileList, testSelectedButton, backButton;
HWND formulaNameInput, resultText;
HWND titleText;
int currentClause = 0;
int totalClauses = 0;
FILE* currentFile = NULL;
char currentFileName[100];
HWND previewArea;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateMainMenu(HWND hwnd);
void ShowFormulaCreation(HWND hwnd);
void ShowFormulaList(HWND hwnd);
void ShowCredits(HWND hwnd);
void HideAllControls();
void ExecuteLogicSolver(HWND hwnd, const char* filename);

// Function to validate a clause
bool is_valid_clause(const char* clause) {
    if (!clause || strlen(clause) == 0) return false;
    
    char* token;
    char clause_copy[1024];
    strcpy(clause_copy, clause);
    
    token = strtok(clause_copy, " \t");
    if (!token) return false;  // Empty clause
    
    while (token) {
        bool is_negated = (token[0] == '!');
        char* var_name = token + (is_negated ? 1 : 0);
        
        // Check if variable name is valid
        if (!var_name || strlen(var_name) == 0) return false;
        
        // First character must be a letter or underscore
        if (!isalpha(var_name[0]) && var_name[0] != '_') return false;
        
        // Rest can be letters, numbers, or underscores
        for (int i = 1; var_name[i]; i++) {
            if (!isalnum(var_name[i]) && var_name[i] != '_') return false;
        }
        
        token = strtok(NULL, " \t");
    }
    
    return true;
}

// Result dialog procedure
LRESULT CALLBACK ResultDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                DestroyWindow(hwnd);
                return TRUE;
            }
            break;
            
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return TRUE;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "ESI-EX-INI Solver";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);

    // Create fonts
    hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    hFontBig = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    mainWindow = CreateWindowEx(
        0,
        CLASS_NAME,
        "ESI-EX-INI Logic Formula Solver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 700,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (mainWindow == NULL) return 0;

    // Create title text
    titleText = CreateWindow(
        "STATIC", "ESI-EX-INI Logic Formula Solver",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 30, 800, 30,
        mainWindow, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(titleText, WM_SETFONT, (WPARAM)hFontBig, TRUE);

    ShowWindow(mainWindow, nCmdShow);
    CreateMainMenu(mainWindow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hFont);
    DeleteObject(hFontBig);
    return 0;
}

void CreateMainMenu(HWND hwnd) {
    HideAllControls();

    createButton = CreateWindow(
        "BUTTON", "Create New Formula",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 120, 300, 45,
        hwnd, (HMENU)ID_CREATE_FORMULA,
        GetModuleHandle(NULL), NULL
    );

    testButton = CreateWindow(
        "BUTTON", "Test Existing Formula",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 180, 300, 45,
        hwnd, (HMENU)ID_TEST_FORMULA,
        GetModuleHandle(NULL), NULL
    );

    showButton = CreateWindow(
        "BUTTON", "Show Available Formulas",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 240, 300, 45,
        hwnd, (HMENU)ID_SHOW_FORMULAS,
        GetModuleHandle(NULL), NULL
    );

    creditsButton = CreateWindow(
        "BUTTON", "Show Credits",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 300, 300, 45,
        hwnd, (HMENU)ID_CREDITS,
        GetModuleHandle(NULL), NULL
    );

    exitButton = CreateWindow(
        "BUTTON", "Exit",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 360, 300, 45,
        hwnd, (HMENU)ID_EXIT,
        GetModuleHandle(NULL), NULL
    );

    SendMessage(createButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(testButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(showButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(creditsButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(exitButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void ShowFormulaCreation(HWND hwnd) {
    HideAllControls();

    // Title label
    HWND titleLabel = CreateWindow(
        "STATIC", "Create New Formula",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 80, 800, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(titleLabel, WM_SETFONT, (WPARAM)hFontBig, TRUE);

    // Formula name label and input
    CreateWindow(
        "STATIC", "Enter Formula Name (without .cnf):",
        WS_VISIBLE | WS_CHILD,
        250, 140, 300, 25,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    formulaNameInput = CreateWindow(
        "EDIT", "",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        250, 170, 300, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Clause count label and input
    CreateWindow(
        "STATIC", "Number of Clauses:",
        WS_VISIBLE | WS_CHILD,
        250, 220, 300, 25,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    clauseCountInput = CreateWindow(
        "EDIT", "",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        250, 250, 300, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Buttons
    submitButton = CreateWindow(
        "BUTTON", "Start Adding Clauses",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 300, 300, 45,
        hwnd, (HMENU)ID_SUBMIT_CLAUSES,
        GetModuleHandle(NULL), NULL
    );

    backButton = CreateWindow(
        "BUTTON", "Back to Main Menu",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 360, 300, 45,
        hwnd, (HMENU)ID_BACK,
        GetModuleHandle(NULL), NULL
    );

    // Apply fonts
    SendMessage(formulaNameInput, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(clauseCountInput, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(submitButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(backButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    SetFocus(formulaNameInput);
}

void ShowClauseInput(HWND hwnd) {
    HideAllControls();

    // Title and instructions
    HWND titleLabel = CreateWindow(
        "STATIC", "Enter Clause",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 80, 800, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(titleLabel, WM_SETFONT, (WPARAM)hFontBig, TRUE);

    char instructionText[256];
    sprintf(instructionText, "Enter clause %d of %d\nUse space between literals, ! for negation (e.g., P !Q R):",
            currentClause + 1, totalClauses);

    CreateWindow(
        "STATIC", instructionText,
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        150, 130, 500, 40,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Clause input
    clauseInput = CreateWindow(
        "EDIT", "",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        250, 180, 300, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Result text (for feedback)
    resultText = CreateWindow(
        "STATIC", "",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        150, 220, 500, 60,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Buttons
    submitButton = CreateWindow(
        "BUTTON", "Add Clause",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 290, 300, 45,
        hwnd, (HMENU)ID_SUBMIT_CLAUSES,
        GetModuleHandle(NULL), NULL
    );

    backButton = CreateWindow(
        "BUTTON", "Back to Main Menu",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 350, 300, 45,
        hwnd, (HMENU)ID_BACK,
        GetModuleHandle(NULL), NULL
    );

    // Apply fonts
    SendMessage(clauseInput, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(submitButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(backButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(resultText, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    SetFocus(clauseInput);
}

void ShowFormulaList(HWND hwnd) {
    HideAllControls();
    
    // Title
    HWND titleLabel = CreateWindow(
        "STATIC", "Available Formulas",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 80, 800, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(titleLabel, WM_SETFONT, (WPARAM)hFontBig, TRUE);
    
    // Create formula list on the left side
    fileList = CreateWindow(
        "LISTBOX",
        NULL,
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY | WS_BORDER,
        50, 130, 300, 400,
        hwnd,
        (HMENU)ID_FILE_LIST,
        GetModuleHandle(NULL),
        NULL
    );

    // Create preview area on the right side
    HWND previewLabel = CreateWindow(
        "STATIC", "Formula Content:",
        WS_VISIBLE | WS_CHILD,
        400, 130, 300, 25,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(previewLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    previewArea = CreateWindow(
        "EDIT",
        "",
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | WS_BORDER,
        400, 160, 300, 370,
        hwnd,
        (HMENU)ID_FORMULA_PREVIEW,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(previewArea, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Load files
    DIR *d;
    struct dirent *dir;
    FormulaFile files[MAX_FILES];
    int count = 0;
    
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL && count < MAX_FILES) {
            if (strstr(dir->d_name, ".cnf")) {
                SendMessage(fileList, LB_ADDSTRING, 0, (LPARAM)dir->d_name);
                strcpy(files[count].filename, dir->d_name);
                count++;
            }
        }
        closedir(d);
    }
    
    if (count == 0) {
        SendMessage(fileList, LB_ADDSTRING, 0, (LPARAM)"No CNF files found");
        EnableWindow(fileList, FALSE);
    }
    
    // Buttons at the bottom
    testSelectedButton = CreateWindow(
        "BUTTON", "Test Selected Formula",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        50, 550, 300, 45,
        hwnd, (HMENU)ID_TEST_SELECTED,
        GetModuleHandle(NULL), NULL
    );
    
    backButton = CreateWindow(
        "BUTTON", "Back to Main Menu",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        400, 550, 300, 45,
        hwnd, (HMENU)ID_BACK,
        GetModuleHandle(NULL), NULL
    );
    
    // Apply fonts
    SendMessage(fileList, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(testSelectedButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(backButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    EnableWindow(testSelectedButton, FALSE);
}

void ShowCredits(HWND hwnd) {
    HideAllControls();

    // Title
    HWND titleText = CreateWindow(
        "STATIC", "Logic Resolution Solver",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 80, 800, 40,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(titleText, WM_SETFONT, (WPARAM)hFontBig, TRUE);

    // Developed by
    HWND devText = CreateWindow(
        "STATIC", "Developed by:",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 150, 800, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    SendMessage(devText, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Names with white background
    HWND name1 = CreateWindow(
        "STATIC", "LASFER MOHAMMED DJAWED",
        WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
        200, 210, 400, 40,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    
    HWND name2 = CreateWindow(
        "STATIC", "MOUSLIM ALI",
        WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
        200, 270, 400, 40,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );
    
    HWND name3 = CreateWindow(
        "STATIC", "HECHEHOUCHE ABOUBEKR",
        WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
        200, 330, 400, 40,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Project info
    HWND projectText = CreateWindow(
        "STATIC", "Advanced Logic Project",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 400, 800, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    HWND yearText = CreateWindow(
        "STATIC", "ESI 2024",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 430, 800, 30,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    // Description
    HWND descText1 = CreateWindow(
        "STATIC", "A powerful tool for logical formula analysis",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 470, 800, 25,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    HWND descText2 = CreateWindow(
        "STATIC", "and automated theorem proving using resolution",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 495, 800, 25,
        hwnd, NULL,
        GetModuleHandle(NULL), NULL
    );

    backButton = CreateWindow(
        "BUTTON", "Back to Main Menu",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 550, 300, 45,
        hwnd, (HMENU)ID_BACK,
        GetModuleHandle(NULL), NULL
    );

    // Apply fonts to all text
    SendMessage(name1, WM_SETFONT, (WPARAM)hFontBig, TRUE);
    SendMessage(name2, WM_SETFONT, (WPARAM)hFontBig, TRUE);
    SendMessage(name3, WM_SETFONT, (WPARAM)hFontBig, TRUE);
    SendMessage(projectText, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(yearText, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(descText1, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(descText2, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(backButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void HideAllControls() {
    HWND child = GetWindow(mainWindow, GW_CHILD);
    while (child) {
        if (child != titleText) {
            ShowWindow(child, SW_HIDE);
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

void ExecuteLogicSolver(HWND hwnd, const char* filename) {
    char command[512];
    char output[MAX_OUTPUT];
    FILE* pipe;
    
    // Create the command with output redirection
    sprintf(command, "logic_solver.exe \"%s\" 2>&1", filename);
    
    // Execute the command and capture output
    pipe = _popen(command, "r");
    if (!pipe) {
        MessageBox(hwnd, "Error executing logic solver!", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    // Read the output
    size_t bytesRead = fread(output, 1, sizeof(output) - 1, pipe);
    output[bytesRead] = '\0';
    _pclose(pipe);
    
    // Register dialog class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = ResultDialogProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ResultDialog";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);
    
    // Create a dialog to show the results
    HWND resultDialog = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "ResultDialog",
        "Logic Solver Results",
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600, 400,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    // Add result text
    HWND resultOutput = CreateWindow(
        "EDIT",
        output,
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
        10, 10,
        570, 300,
        resultDialog,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    // Add close button
    HWND closeButton = CreateWindow(
        "BUTTON",
        "Close",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        250, 320,
        100, 30,
        resultDialog,
        (HMENU)IDOK,
        GetModuleHandle(NULL),
        NULL
    );
    
    // Apply fonts
    SendMessage(resultOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(closeButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // Set focus to close button
    SetFocus(closeButton);
    
    // Update and show the dialog
    UpdateWindow(resultDialog);
    ShowWindow(resultDialog, SW_SHOW);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_CREATE_FORMULA:
                    ShowFormulaCreation(hwnd);
                    break;

                case ID_TEST_FORMULA:
                case ID_SHOW_FORMULAS:
                    ShowFormulaList(hwnd);
                    break;

                case ID_CREDITS:
                    ShowCredits(hwnd);
                    break;

                case ID_EXIT:
                    DestroyWindow(hwnd);
                    break;

                case ID_SUBMIT_CLAUSES: {
                    char clauseCountStr[16] = {0};
                    char formulaName[100] = {0};
                    
                    if (currentFile == NULL) {
                        // First submission - initialize file
                        GetWindowText(formulaNameInput, formulaName, sizeof(formulaName));
                        GetWindowText(clauseCountInput, clauseCountStr, sizeof(clauseCountStr));
                        
                        if (strlen(formulaName) == 0) {
                            MessageBox(hwnd, "Please enter a formula name!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        
                        // Validate formula name
                        bool valid_name = true;
                        for (int i = 0; formulaName[i]; i++) {
                            if (!isalnum(formulaName[i]) && formulaName[i] != '_' && formulaName[i] != '-') {
                                MessageBox(hwnd, "Formula name can only contain letters, numbers, underscores, and hyphens!", 
                                         "Error", MB_OK | MB_ICONERROR);
                                valid_name = false;
                                break;
                            }
                        }
                        if (!valid_name) break;
                        
                        totalClauses = atoi(clauseCountStr);
                        if (totalClauses < 0) {
                            MessageBox(hwnd, "Please enter a valid number of clauses (0 or greater)!", 
                                     "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        
                        sprintf(currentFileName, "%s.cnf", formulaName);
                        currentFile = fopen(currentFileName, "w");
                        if (!currentFile) {
                            MessageBox(hwnd, "Could not create formula file!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        
                        fprintf(currentFile, "# Formula created using ESI-EX-INI GUI Solver\n");
                        
                        // Handle 0 clauses case
                        if (totalClauses == 0) {
                            fclose(currentFile);
                            currentFile = NULL;
                            char message[256];
                            sprintf(message, "Empty formula saved to %s\nWould you like to test it now?", currentFileName);
                            if (MessageBox(hwnd, message, "Success", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                                ExecuteLogicSolver(hwnd, currentFileName);
                            }
                            CreateMainMenu(hwnd);
                            break;
                        }
                        
                        currentClause = 0;
                        ShowClauseInput(hwnd);
                    } else {
                        // Process clause input
                        char clause[1024] = {0};
                        GetWindowText(clauseInput, clause, sizeof(clause));
                        
                        // Validate clause format
                        if (!is_valid_clause(clause)) {
                            SetWindowText(resultText, "Invalid clause format! Use space-separated literals (e.g., P !Q R)");
                            break;
                        }
                        
                        fprintf(currentFile, "%s\n", clause);
                        currentClause++;
                        
                        if (currentClause >= totalClauses) {
                            // All clauses entered
                            fclose(currentFile);
                            currentFile = NULL;
                            char message[256];
                            sprintf(message, "Formula saved to %s\nWould you like to test it now?", currentFileName);
                            if (MessageBox(hwnd, message, "Success", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                                ExecuteLogicSolver(hwnd, currentFileName);
                            }
                            CreateMainMenu(hwnd);
                        } else {
                            // Show input for next clause
                            SetWindowText(clauseInput, "");
                            char progress[64];
                            sprintf(progress, "Clause %d added successfully!", currentClause);
                            SetWindowText(resultText, progress);
                            ShowClauseInput(hwnd);
                        }
                    }
                    break;
                }

                case ID_TEST_SELECTED: {
                    int selectedIndex = SendMessage(fileList, LB_GETCURSEL, 0, 0);
                    if (selectedIndex != LB_ERR) {
                        char filename[MAX_PATH];
                        SendMessage(fileList, LB_GETTEXT, selectedIndex, (LPARAM)filename);
                        ExecuteLogicSolver(hwnd, filename);
                    }
                    break;
                }

                case ID_FILE_LIST: {
                    if (HIWORD(wParam) == LBN_SELCHANGE) {
                        int selectedIndex = SendMessage(fileList, LB_GETCURSEL, 0, 0);
                        EnableWindow(testSelectedButton, selectedIndex != LB_ERR);

                        // Update preview area with file contents
                        if (selectedIndex != LB_ERR) {
                            char filename[MAX_PATH];
                            SendMessage(fileList, LB_GETTEXT, selectedIndex, (LPARAM)filename);
                            
                            // Read and display file contents
                            FILE* file = fopen(filename, "r");
                            if (file) {
                                char content[8192] = {0};
                                char line[1024];
                                bool firstClause = true;
                                
                                // Skip the first line (comment)
                                fgets(line, sizeof(line), file);
                                
                                // Read and format each clause
                                while (fgets(line, sizeof(line), file)) {
                                    // Remove newline if present
                                    line[strcspn(line, "\n")] = 0;
                                    
                                    // Skip empty lines
                                    if (strlen(line) == 0) continue;
                                    
                                    // Add AND symbol between clauses
                                    if (!firstClause) {
                                        strcat(content, " ^\n");
                                    }
                                    firstClause = false;
                                    
                                    // Format the clause
                                    char* token = strtok(line, " \t");
                                    bool firstLiteral = true;
                                    
                                    while (token) {
                                        // Add OR symbol between literals
                                        if (!firstLiteral) {
                                            strcat(content, " V ");
                                        }
                                        firstLiteral = false;
                                        
                                        // Add the literal
                                        strcat(content, token);
                                        
                                        token = strtok(NULL, " \t");
                                    }
                                }
                                
                                SetWindowText(previewArea, content);
                                fclose(file);
                            } else {
                                SetWindowText(previewArea, "Error: Could not open file");
                            }
                        }
                    }
                    break;
                }

                case ID_BACK:
                    if (currentFile) {
                        fclose(currentFile);
                        currentFile = NULL;
                    }
                    CreateMainMenu(hwnd);
                    break;
            }
            return 0;

        case WM_CTLCOLORSTATIC:
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
} 
