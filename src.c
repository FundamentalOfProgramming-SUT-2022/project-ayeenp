// Ayeen Poostforoushan 401105742

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <windows.h>
#include <dirent.h>

#define MAX_BRACKET_COUNT 1000
#define MAX_CMD_LINE_LENGTH 400
#define MAX_GREP_FILECOUNT 50
#define MAX_STREAM_LENGTH 50000
#define MAX_FILE_LINE_LENGTH 1000
#define MAX_INT_LENGTH 10
#define TEMP_ADDRESS "root/.VIMTMP"
#define UNDO_SUFFIX ".VIMTMP"
#define CMD_REM 1
#define CMD_COPY 2
#define CMD_CUT 3
#define NOT_IN_BRACKET_RANGE -1

void run();
void quit();
void cmdCreatefile(char *input);
void cmdGrep(char *input);
void cmdTree(char *input);
void cmdCompare(char *input);
void cmdInsert(char *input);
void cmdRemCopyCut(char *input, int mode);
void cmdUndo(char *input);
void cmdAutoIndent(char *input);
void cmdFind(char *input);
void cmdReplace(char *input);
void cmdPaste(char *input);
void cmdCat(char *input);
bool cat(char *fileName);
void showTree(const char *dirName, int depth, int maxDepth);
void compare(char *fileName1, char *fileName2);
bool insertText(char *fileName, char *text, int linePos, int charPos, int seekPos);
bool removeText(char *fileName, int linePos, int charPos, int seekPos, int size, bool isForward);
void grep(char (*paths)[MAX_PATH], char *toBeFound, bool lMode, bool cMode);
void find(char *fileName, char *toBeFound, int at, bool isCount, bool isByWord, bool isAll);
int findMatchCount(char *fileString, char *toBeFound);
int findAt(const char *fileString, char *toBeFound, int at, bool isByWord, int *foundWordSize);
bool replace(char *fileName, char *toBeFound, char *toBeReplaced, int at, bool isAll);
bool replaceAt(char *fileName, char *fileString, char *toBeFound, char *toBeReplaced, int at);
bool undo(char *fileName);
bool autoIndent(char *fileName);
void bracketsAutoIndent(char *text, int leadingSpaceCount);
bool copyFileContentToClipboard(char *fileName, int linePos, int charPos, int size, bool isForward);
bool cutFileContentToClipboard(char *fileName, int linePos, int charPos, int size, bool isForward);
bool pasteFromClipboard(char *fileName, int linePos, int charPos);
void copyStrToClipboard(const char *str);
void retrieveStrFromClipboard(char *str);
void autoIndentInitialize(char *text);
void fillBracketsArray(const char *text, int (*brackets)[2]);
int findBracketRange(int index, const int (*brackets)[2]);
void deleteAllBackups(const char *dirName);
void deleteLastBackup(const char *fileName);
void backupForUndo(const char *fileName);
int getLastUndoNumber(const char *fileName);
void generateUndoPath(char *undoPath, const char *fileName, int num);
void handleWildCards(char *str, bool *leadingWC, bool *endingWC);
void handleNewlines(char *str);
void splitPaths(const char *str, char (*paths)[MAX_PATH]);
bool readAndWriteNlines(int n, FILE *tempptr, FILE *sourceptr);
bool readAndWriteNchars(int n, FILE *tempptr, FILE *sourceptr);
bool readAndWriteNseeks(int n, FILE *tempptr, FILE *sourceptr);
bool seekNlines(int n, FILE *sourceptr);
bool seekNchars(int n, bool isForward, FILE *sourceptr);
void writeStrToFile(char *text, FILE *tempptr, FILE *sourceptr);
bool copyFile(const char *sourceFileName, const char *destFileName);
bool createFileAndDirs(char *fileName);
bool createFile(const char *fileName);
void createAllDirs(const char *dirName);
bool directoryExists(const char *path);
void makeFileHidden(const char *fileName);
void makeFileNotHidden(const char *fileName);
void printTreeItem(const char *path, int depth);
void printCompareComplex(const char *line, int wordStart, int wordEnd);
void inputLine(char *str);
void inputLineFromFile(FILE *fp, char *str);
void fileToString(FILE *fp, char *str);
bool handleDoubleQuotation(char *str);
bool removeDoubleQuotations(char *str);
int findMatchingWord(const char *str, const char *match);
bool findMatchFromIndex(const char *str, const char *match, int startingIndex, bool isForward);
void copyStringRange(char *dest, const char *source, int start, int end);
bool copyNthWord(char *dest, const char *str, int n);
bool findNthWord(const char *str, int n, int *startIndex, int *endIndex);
void fixPathString(char *path);

int main()
{
    run();
}

void run()
{
    char *input = (char *)malloc(MAX_CMD_LINE_LENGTH * sizeof(char));
    char command[20];
    while (1)
    {
        inputLine(input);
        copyNthWord(command, input, 1);
        if (strcmp(command, "quit") == 0)
        {
            quit();
            break;
        }
        else if (strcmp(command, "createfile") == 0)
            cmdCreatefile(input);
        else if (strcmp(command, "insertstr") == 0)
            cmdInsert(input);
        else if (strcmp(command, "removestr") == 0)
            cmdRemCopyCut(input, CMD_REM);
        else if (strcmp(command, "copystr") == 0)
            cmdRemCopyCut(input, CMD_COPY);
        else if (strcmp(command, "cutstr") == 0)
            cmdRemCopyCut(input, CMD_CUT);
        else if (strcmp(command, "undo") == 0)
            cmdUndo(input);
        else if (strcmp(command, "pastestr") == 0)
            cmdPaste(input);
        else if (strcmp(command, "find") == 0)
            cmdFind(input);
        else if (strcmp(command, "replace") == 0)
            cmdReplace(input);
        else if (strcmp(command, "grep") == 0)
            cmdGrep(input);
        else if (strcmp(command, "tree") == 0)
            cmdTree(input);
        else if (strcmp(command, "compare") == 0)
            cmdCompare(input);
        else if (strcmp(command, "auto-indent") == 0)
            cmdAutoIndent(input);
        else if (strcmp(command, "cat") == 0)
            cmdCat(input);
    }
    free(input);
}

void quit()
{
    deleteAllBackups("root/");
}

void cmdCreatefile(char *input)
{
    int arg1index = findMatchingWord(input, " --file");
    if (arg1index == -1)
    {
        printf("Required: --file\n");
        return;
    }
    char fileName[MAX_PATH];
    copyStringRange(fileName, input, arg1index + 1, -1);
    if (!handleDoubleQuotation(fileName))
    {
        printf("Invalid path input\n");
        return;
    }
    fixPathString(fileName);
    createFileAndDirs(fileName);
}

void cmdGrep(char *input)
{
    char textToBeFound[MAX_STREAM_LENGTH];
    char pathsString[MAX_CMD_LINE_LENGTH];
    char(*paths)[MAX_PATH] = (char(*)[MAX_PATH])calloc(MAX_GREP_FILECOUNT, sizeof(char[MAX_PATH]));
    bool lMode = false, cMode = false;
    if (findMatchingWord(input, "-c") != -1)
        cMode = true;
    else if (findMatchingWord(input, "-l") != -1)
        lMode = true;
    int arg1index = findMatchingWord(input, " --str");
    int arg2index = findMatchingWord(input, " --files");

    if (arg1index == -1 || arg2index == -1)
    {
        printf("Required: --str, --files\n");
        return;
    }
    copyStringRange(textToBeFound, input, arg1index + 1, arg2index - 8);
    copyStringRange(pathsString, input, arg2index + 1, -1);
    if (!handleDoubleQuotation(textToBeFound))
    {
        printf("Invalid text input\n");
        return;
    }
    splitPaths(pathsString, paths);
    grep(paths, textToBeFound, lMode, cMode);
    free(paths);
}

void cmdTree(char *input)
{
    int depth;
    if (sscanf(input, "tree %d", &depth) != 1)
    {
        printf("Type the depth properly after -pos\n");
        return;
    }
    if (depth < -1)
    {
        printf("Depth can't be smaller than -1\n");
        return;
    }
    showTree("root/", 1, depth);
}

void cmdCompare(char *input)
{
    char(*paths)[MAX_PATH] = (char(*)[MAX_PATH])calloc(3, sizeof(char[MAX_PATH]));
    char pathsString[MAX_CMD_LINE_LENGTH];
    splitPaths(input, paths);
    if (paths[1][0] == '\0' || paths[2][0] == '\0')
    {
        printf("Please input the paths properly\n");
        return;
    }
    compare(paths[1], paths[2]);
}

void cmdInsert(char *input)
{
    char path[MAX_PATH];
    char textToInsert[MAX_STREAM_LENGTH];
    char pos[MAX_INT_LENGTH];
    int linePos, charPos;

    int arg1index = findMatchingWord(input, " --file");
    int arg2index = findMatchingWord(input, " --str");
    int arg3index = findMatchingWord(input, " -pos");
    if (arg1index == -1 || arg2index == -1 || arg3index == -1)
    {
        printf("Required: --file, --str, -pos\n");
        return;
    }
    copyStringRange(path, input, arg1index + 1, arg2index - 6);
    copyStringRange(textToInsert, input, arg2index + 1, arg3index - 5);
    copyStringRange(pos, input, arg3index + 1, -1);
    if (!handleDoubleQuotation(path))
    {
        printf("Invalid path input\n");
        return;
    }
    if (!handleDoubleQuotation(textToInsert))
    {
        printf("Invalid text input\n");
        return;
    }
    if (sscanf(pos, "%d:%d", &linePos, &charPos) != 2)
    {
        printf("Type the position properly after -pos\n");
        return;
    }
    fixPathString(path);
    backupForUndo(path);
    if (!insertText(path, textToInsert, linePos, charPos, -1))
        deleteLastBackup(path);
}

void cmdRemCopyCut(char *input, int mode)
{
    char path[MAX_PATH];
    char pos[MAX_INT_LENGTH];
    char sizeStr[MAX_INT_LENGTH];
    int linePos, charPos, size;
    bool isForward = true;

    int arg1index = findMatchingWord(input, " --file");
    int arg2index = findMatchingWord(input, " -pos");
    int arg3index = findMatchingWord(input, " -size");
    int arg4index = findMatchingWord(input, " -f");
    if (arg4index == -1)
    {
        isForward = false;
        arg4index = findMatchingWord(input, " -b");
    }
    if (arg1index == -1 || arg2index == -1 || arg3index == -1 || arg4index == -1)
    {
        printf("Required: --file, -pos, -size, -b or -f\n");
        return;
    }
    copyStringRange(path, input, arg1index + 1, arg2index - 5);
    copyStringRange(pos, input, arg2index + 1, arg3index - 5);
    copyStringRange(sizeStr, input, arg3index + 1, arg4index - 3);

    if (!handleDoubleQuotation(path))
    {
        printf("Invalid path input\n");
        return;
    }
    fixPathString(path);
    if (sscanf(pos, "%d:%d", &linePos, &charPos) != 2)
    {
        printf("Type the position properly after -pos\n");
        return;
    }
    if (sscanf(sizeStr, "%d", &size) != 1)
    {
        printf("Type the size properly after -size\n");
        return;
    }
    switch (mode)
    {
    case CMD_REM:
        backupForUndo(path);
        if (!removeText(path, linePos, charPos, -1, size, isForward))
            deleteLastBackup(path);
        break;
    case CMD_COPY:
        copyFileContentToClipboard(path, linePos, charPos, size, isForward);
        break;
    case CMD_CUT:
        backupForUndo(path);
        if (!cutFileContentToClipboard(path, linePos, charPos, size, isForward))
            deleteLastBackup(path);
        break;
    }
}

void cmdUndo(char *input)
{
    int arg1index = findMatchingWord(input, " --file");
    if (arg1index == -1)
    {
        printf("Required: --file\n");
        return;
    }
    char fileName[MAX_PATH];
    copyStringRange(fileName, input, arg1index + 1, -1);
    if (!handleDoubleQuotation(fileName))
    {
        printf("Invalid path input\n");
        return;
    }
    fixPathString(fileName);
    undo(fileName);
}

void cmdAutoIndent(char *input)
{
    int arg1index = findMatchingWord(input, "auto-indent");
    if (arg1index == -1)
    {
        return;
    }
    char fileName[MAX_PATH];
    copyStringRange(fileName, input, arg1index + 1, -1);
    if (!handleDoubleQuotation(fileName))
    {
        printf("Invalid path input\n");
        return;
    }
    fixPathString(fileName);
    backupForUndo(fileName);
    if (!autoIndent(fileName))
        deleteLastBackup(fileName);
}

void cmdFind(char *input)
{
    char path[MAX_PATH];
    char textToBeFound[MAX_STREAM_LENGTH];
    bool isCount = true, isByword = true, isAll = true;
    int at = 1;
    int leastArgIndex = MAX_CMD_LINE_LENGTH;
    int arg1index = findMatchingWord(input, "--str");
    int arg2index = findMatchingWord(input, " --file");
    int arg3index = findMatchingWord(input, " -count");
    if (arg3index == -1)
        isCount = false;
    else
        leastArgIndex = arg3index - 7;
    arg3index = findMatchingWord(input, " -at");
    if (arg3index != -1)
    {
        char atstr[MAX_INT_LENGTH];
        copyStringRange(atstr, input, arg3index + 1, arg3index + 3);
        if (sscanf(atstr, "%d ", &at) != 1)
        {
            printf("Type the number properly after -at\n");
            return;
        }
        if (arg3index <= leastArgIndex)
            leastArgIndex = arg3index - 4;
    }
    arg3index = findMatchingWord(input, " -byword");
    if (arg3index == -1)
        isByword = false;
    else if (arg3index <= leastArgIndex)
        leastArgIndex = arg3index - 8;
    arg3index = findMatchingWord(input, " -all");
    if (arg3index == -1)
        isAll = false;
    else if (arg3index <= leastArgIndex)
        leastArgIndex = arg3index - 5;
    if (arg1index == -1 || arg2index == -1)
    {
        printf("Required: --str, --file\n");
        return;
    }
    if (leastArgIndex == MAX_CMD_LINE_LENGTH)
        leastArgIndex = -1;
    copyStringRange(textToBeFound, input, arg1index + 1, arg2index - 7);
    copyStringRange(path, input, arg2index + 1, leastArgIndex);
    if (!handleDoubleQuotation(path))
    {
        printf("Invalid path input\n");
        return;
    }
    if (!handleDoubleQuotation(textToBeFound))
    {
        printf("Invalid text input\n");
        return;
    }
    fixPathString(path);
    find(path, textToBeFound, at, isCount, isByword, isAll);
}

void cmdReplace(char *input)
{
    char path[MAX_PATH];
    char textToBeFound[MAX_STREAM_LENGTH];
    char textToBeReplaced[MAX_STREAM_LENGTH];
    bool isAll = true;
    int at = 1;
    int leastArgIndex = MAX_CMD_LINE_LENGTH;
    int arg1index = findMatchingWord(input, "--str1");
    int arg2index = findMatchingWord(input, " --str2");
    int arg3index = findMatchingWord(input, " --file");
    int arg4index = findMatchingWord(input, " -all");
    if (arg4index == -1)
        isAll = false;
    else
        leastArgIndex = arg4index - 5;
    arg4index = findMatchingWord(input, " -at");
    if (arg4index != -1)
    {
        char atstr[MAX_INT_LENGTH];
        copyStringRange(atstr, input, arg4index + 1, arg4index + 3);
        if (sscanf(atstr, "%d ", &at) != 1)
        {
            printf("Type the number properly after -at\n");
            return;
        }
        if (arg4index <= leastArgIndex)
            leastArgIndex = arg4index - 4;
    }
    if (arg1index == -1 || arg2index == -1 || arg3index == -1)
    {
        printf("Required: --str, --file\n");
        return;
    }
    if (leastArgIndex == MAX_CMD_LINE_LENGTH)
        leastArgIndex = -1;
    copyStringRange(textToBeFound, input, arg1index + 1, arg2index - 7);
    copyStringRange(textToBeReplaced, input, arg2index + 1, arg3index - 7);
    copyStringRange(path, input, arg3index + 1, leastArgIndex);
    if (!handleDoubleQuotation(path))
    {
        printf("Invalid path input\n");
        return;
    }
    if (!handleDoubleQuotation(textToBeFound))
    {
        printf("Invalid text input\n");
        return;
    }
    if (!handleDoubleQuotation(textToBeReplaced))
    {
        printf("Invalid text input\n");
        return;
    }
    fixPathString(path);
    backupForUndo(path);
    if (!replace(path, textToBeFound, textToBeReplaced, at, isAll))
        deleteLastBackup(path);
}

void cmdPaste(char *input)
{
    char path[MAX_PATH];
    char pos[MAX_INT_LENGTH];
    int linePos, charPos;

    int arg1index = findMatchingWord(input, " --file");
    int arg2index = findMatchingWord(input, " -pos");
    if (arg1index == -1 || arg2index == -1)
    {
        printf("Required: --file, -pos\n");
        return;
    }
    copyStringRange(path, input, arg1index + 1, arg2index - 5);
    copyStringRange(pos, input, arg2index + 1, -1);
    if (!handleDoubleQuotation(path))
    {
        printf("Invalid path input\n");
        return;
    }
    if (sscanf(pos, "%d:%d", &linePos, &charPos) != 2)
    {
        printf("Type the position properly after -pos\n");
        return;
    }
    fixPathString(path);
    backupForUndo(path);
    if (!pasteFromClipboard(path, linePos, charPos))
        deleteLastBackup(path);
}

void cmdCat(char *input)
{
    int arg1index = findMatchingWord(input, " --file");
    if (arg1index == -1)
    {
        printf("Required: --file\n");
        return;
    }
    char fileName[MAX_PATH];
    copyStringRange(fileName, input, arg1index + 1, -1);
    if (!handleDoubleQuotation(fileName))
    {
        printf("Invalid path input\n");
        return;
    }
    fixPathString(fileName);
    cat(fileName);
}

bool cat(char *fileName)
{
    if (access(fileName, R_OK) == -1)
    {
        printf("File doesn't exist\n");
        return false;
    }
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("Error occured while reading the file\n");
        return false;
    }
    char c;
    int lineNum = 1;
    printf("%d  >", lineNum++);
    while ((c = fgetc(fp)) != EOF)
    {
        if (c == '\n')
        {
            printf("\n%d  >", lineNum++);
            continue;
        }
        printf("%c", c);
    }
    printf("\n");
    fclose(fp);
    return true;
}

void showTree(const char *dirName, int depth, int maxDepth)
{
    if (depth == maxDepth + 1)
        return;
    DIR *dp = opendir(dirName);
    if (dp == NULL)
    {
        printf("Directory doesn't exist\n");
        return;
    }
    struct dirent *dir;
    while ((dir = readdir(dp)) != NULL)
    {
        if (dir->d_type != DT_DIR && strncmp(dir->d_name, UNDO_SUFFIX, strlen(UNDO_SUFFIX)) != 0)
            printTreeItem(dir->d_name, depth);
        else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
        {
            printTreeItem(dir->d_name, depth);
            char dPath[MAX_PATH];
            strcpy(dPath, "");
            sprintf(dPath, "%s/%s", dirName, dir->d_name);
            showTree(dPath, depth + 1, maxDepth);
        }
    }
}

void compare(char *fileName1, char *fileName2)
{
    FILE *fp1;
    fp1 = fopen(fileName1, "r");
    if (fp1 == NULL)
    {
        printf("File %s doesn't exist\n", fileName1);
        return;
    }
    FILE *fp2;
    fp2 = fopen(fileName2, "r");
    if (fp2 == NULL)
    {
        printf("File %s doesn't exist\n", fileName1);
        return;
    }
    int checkedLineCounter = 0;
    char f1Line[MAX_FILE_LINE_LENGTH];
    char f2Line[MAX_FILE_LINE_LENGTH];
    while (!feof(fp1) && !feof(fp2))
    {
        checkedLineCounter++;
        inputLineFromFile(fp1, f1Line);
        inputLineFromFile(fp2, f2Line);
        if (strcmp(f1Line, f2Line) == 0)
            continue;
        int wordDifferCount = 0, finals1, finals2, finale1, finale2;
        for (int word = 1; 1; word++)
        {
            int s1, s2, e1, e2;
            bool line1state = findNthWord(f1Line, word, &s1, &e1);
            bool line2state = findNthWord(f2Line, word, &s2, &e2);
            if (!line1state && !line2state)
                break;
            else if ((line1state && !line2state) || (!line1state && line2state))
            {
                // Cancel the compare complex
                wordDifferCount += 10;
                break;
            }
            if (e2 - s2 != e1 - s1)
            {
                wordDifferCount++;
                finals1 = s1;
                finals2 = s2;
                finale1 = e1;
                finale2 = e2;
            }
            else
            {
                for (int i = 0; i < e1 - s1; i++)
                {
                    if (f1Line[s1 + i] != f2Line[s2 + i])
                    {
                        finals1 = s1;
                        finals2 = s2;
                        finale1 = e1;
                        finale2 = e2;
                        wordDifferCount++;
                        break;
                    }
                }
            }
            if (wordDifferCount > 1)
                break;
        }
        printf("========= #%d =========\n", checkedLineCounter);
        if (wordDifferCount == 1)
        {
            printCompareComplex(f1Line, finals1, finale1);
            printCompareComplex(f2Line, finals2, finale2);
        }
        else
        {
            printf("%s\n", f1Line);
            printf("%s\n", f2Line);
        }
    }
    if (!feof(fp1))
    {
        long int currentPos = ftell(fp1);
        int newLineCounter = 0;
        while (!feof(fp1))
        {
            if (fgetc(fp1) == '\n')
                newLineCounter++;
        }
        printf(">>>>>>>>>> File 1 | #%d-#%d <<<<<<<<<<\n", checkedLineCounter + 1, checkedLineCounter + newLineCounter + 1);
        fseek(fp1, currentPos, SEEK_SET);
        while (!feof(fp1))
        {
            inputLineFromFile(fp1, f1Line);
            printf("%s\n");
        }
    }
    if (!feof(fp2))
    {
        long int currentPos = ftell(fp2);
        int newLineCounter = 0;
        while (!feof(fp2))
        {
            if (fgetc(fp2) == '\n')
                newLineCounter++;
        }
        printf(">>>>>>>>>> File 2 | #%d-#%d <<<<<<<<<<\n", checkedLineCounter, newLineCounter + 1);
        fseek(fp2, currentPos, SEEK_SET);
        while (!feof(fp2))
        {
            inputLineFromFile(fp2, f2Line);
            printf("%s\n");
        }
    }
    fclose(fp1);
    fclose(fp2);
}

bool insertText(char *fileName, char *text, int linePos, int charPos, int seekPos)
{
    FILE *sourceptr = fopen(fileName, "r");
    if (sourceptr == NULL)
    {
        printf("File doesn't exist\n");
        return false;
    }
    FILE *tempptr = fopen(TEMP_ADDRESS, "w");
    if (seekPos == -1)
    {
        // writing first lines
        if (!readAndWriteNlines(linePos, tempptr, sourceptr))
        {
            printf("Line pos too big\n");
            fclose(tempptr);
            fclose(sourceptr);
            remove(TEMP_ADDRESS);
            return false;
        }
        // writing first chars in the line
        if (!readAndWriteNchars(charPos, tempptr, sourceptr))
        {
            printf("Char pos too big\n");
            fclose(tempptr);
            fclose(sourceptr);
            remove(TEMP_ADDRESS);
            return false;
        }
    }
    else
        readAndWriteNseeks(seekPos, tempptr, sourceptr);
    // writing the string being inserted
    writeStrToFile(text, tempptr, sourceptr);
    // writing the rest of the file
    char c;
    while ((c = fgetc(sourceptr)) != EOF)
        fprintf(tempptr, "%c", c);
    fclose(tempptr);
    fclose(sourceptr);
    if (remove(fileName) != 0)
    {
        printf("Couldn't compelete the insertion (can't remove the source file)\n");
        return false;
    }
    if (rename(TEMP_ADDRESS, fileName) != 0)
    {
        printf("Couldn't compelete the insertion (can't rename the temp file)\n");
        return false;
    }
    return true;
}

bool removeText(char *fileName, int linePos, int charPos, int seekPos, int size, bool isForward)
{
    FILE *sourceptr = fopen(fileName, "r");
    if (sourceptr == NULL)
    {
        printf("File doesn't exist\n");
        return false;
    }
    FILE *tempptr = fopen(TEMP_ADDRESS, "w+");
    if (seekPos == -1)
    {
        // writing first lines
        if (!readAndWriteNlines(linePos, tempptr, sourceptr))
        {
            printf("Line pos too big\n");
            fclose(tempptr);
            fclose(sourceptr);
            remove(TEMP_ADDRESS);
            return false;
        }
        // writing first chars in the line
        if (!readAndWriteNchars(charPos, tempptr, sourceptr))
        {
            printf("Char pos too big\n");
            fclose(tempptr);
            fclose(sourceptr);
            remove(TEMP_ADDRESS);
            return false;
        }
    }
    else
        readAndWriteNseeks(seekPos, tempptr, sourceptr);
    if (!isForward)
        seekNchars(size, false, tempptr);
    else
        seekNchars(size, true, sourceptr);
    // writing the rest of the file
    char c;
    while ((c = fgetc(sourceptr)) != EOF)
        fprintf(tempptr, "%c", c);
    fclose(tempptr);
    fclose(sourceptr);
    remove(fileName);
    rename(TEMP_ADDRESS, fileName);
    return true;
}

void grep(char (*paths)[MAX_PATH], char *toBeFound, bool lMode, bool cMode)
{
    if (lMode && cMode)
    {
        printf("-l and -c can't be simultaneously used\n");
        return;
    }
    int matchesFound = 0;
    for (int fileIndex = 0; paths[fileIndex][0] != '\0'; fileIndex++)
    {
        bool foundAMatch = false;
        FILE *fp;
        fp = fopen(paths[fileIndex], "r");
        if (fp == NULL)
        {
            printf("File %s doesn't exist\n", paths[fileIndex]);
            continue;
        }
        while (!feof(fp))
        {
            char currentLine[MAX_FILE_LINE_LENGTH];
            inputLineFromFile(fp, currentLine);
            for (int i = 0; currentLine[i] != '\0'; i++)
            {
                if (!findMatchFromIndex(currentLine, toBeFound, i, 1))
                    continue;
                if (lMode)
                    foundAMatch = true;
                else if (!cMode)
                    printf("%d- %s: %s\n", matchesFound + 1, paths[fileIndex], currentLine);
                matchesFound++;
                break;
            }
            if (lMode && foundAMatch)
                break;
        }
        if (lMode && foundAMatch)
            printf("%s\n", paths[fileIndex]);
        fclose(fp);
    }
    if (cMode)
        printf("%d\n", matchesFound);
}

void find(char *fileName, char *toBeFound, int at, bool isCount, bool isByWord, bool isAll)
{
    if ((isAll && isCount) || (isAll && at != 1) || (isCount && isByWord) || (isCount && at != 1))
    {
        printf("Wrong combination of arguments for find\n");
        return;
    }
    if (access(fileName, R_OK) == -1)
    {
        printf("File doesn't exist\n");
        return;
    }
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("Error occured while reading the file\n");
        return;
    }
    char fileString[MAX_STREAM_LENGTH];
    fileToString(fp, fileString);
    fclose(fp);

    int size;
    if (isCount)
    {
        printf("%d\n", findMatchCount(fileString, toBeFound));
        return;
    }
    int foundPosition = findAt(fileString, toBeFound, at, isByWord, &size);
    if (foundPosition == -1)
    {
        printf("No matches found\n");
        return;
    }
    if (isAll)
    {
        while (1)
        {
            int index = findAt(fileString, toBeFound, at, isByWord, &size);
            if (index == -1)
                break;
            printf("%d ", index);
            at++;
        }
        printf("\n");
        return;
    }

    printf("%d\n", findAt(fileString, toBeFound, at, isByWord, &size));
    return;
}

int findMatchCount(char *fileString, char *toBeFound)
{
    bool leadingWC = false, endingWC = false;
    handleWildCards(toBeFound, &leadingWC, &endingWC);
    handleNewlines(toBeFound);
    int count = 0;
    for (int i = 0; fileString[i] != '\0'; i++)
        if (findMatchFromIndex(fileString, toBeFound, i, 1))
            count++;
    return count;
}

int findAt(const char *fileString, char *toBeFound, int at, bool isByWord, int *foundWordSize)
{
    bool leadingWC = false, endingWC = false;
    handleWildCards(toBeFound, &leadingWC, &endingWC);
    handleNewlines(toBeFound);
    // if (!leadingWC && !endingWC)
    //     *foundWordSize = strlen(toBeFound);
    *foundWordSize = strlen(toBeFound);
    int startIndex, endIndex, count = 0;
    for (int wordCount = 1; 1; wordCount++)
    {
        if (!findNthWord(fileString, wordCount, &startIndex, &endIndex))
            break;
        for (int i = startIndex; i <= endIndex; i++)
        {
            if (!findMatchFromIndex(fileString, toBeFound, i, 1))
                continue;
            count++;
            if (at == count)
            {
                if (isByWord)
                    return wordCount;
                else
                    return i;
            }
        }
    }
    return -1;
}

bool replace(char *fileName, char *toBeFound, char *toBeReplaced, int at, bool isAll)
{
    if (isAll && at != 1)
    {
        printf("Wrong combination of arguments for find\n");
        return false;
    }
    if (access(fileName, R_OK) == -1)
    {
        printf("File doesn't exist\n");
        return false;
    }
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("Error occured while reading the file\n");
        return false;
    }
    char fileString[MAX_STREAM_LENGTH];
    fileToString(fp, fileString);
    fclose(fp);

    if (isAll)
    {
        int matchCount = findMatchCount(fileString, toBeFound);
        if (matchCount == 0)
        {
            printf("Couldn't find the expression\n");
            return false;
        }
        while (replaceAt(fileName, fileString, toBeFound, toBeReplaced, matchCount))
            matchCount--;
        printf("Successfully replaced all matches\n");
        return true;
    }
    if (!replaceAt(fileName, fileString, toBeFound, toBeReplaced, at))
    {
        printf("Couldn't find the expression\n");
        return false;
    }
    printf("Successfully replaced\n");
    return true;
}

bool replaceAt(char *fileName, char *fileString, char *toBeFound, char *toBeReplaced, int at)
{
    int foundWordSize;
    int wordIndex = findAt(fileString, toBeFound, at, 0, &foundWordSize);
    if (wordIndex == -1)
        return false;
    removeText(fileName, 0, 0, wordIndex, foundWordSize, 1);
    insertText(fileName, toBeReplaced, 0, 0, wordIndex);
    return true;
}

bool undo(char *fileName)
{
    char undoPath[MAX_PATH];
    generateUndoPath(undoPath, fileName, getLastUndoNumber(fileName) - 1);
    if (access(undoPath, F_OK) != 0)
        return false;
    if (remove(fileName) != 0)
    {
        printf("Can't remove the file\n");
        return false;
    }
    if (rename(undoPath, fileName) != 0)
    {
        printf("Can't rename the backup file\n");
        return false;
    }
    makeFileNotHidden(fileName);
    return true;
}

bool autoIndent(char *fileName)
{
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("Error occured while reading the file\n");
        return false;
    }
    FILE *fptmp;
    fptmp = fopen(TEMP_ADDRESS, "w");
    char *textCopy = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
    fileToString(fp, textCopy);
    fclose(fp);
    autoIndentInitialize(textCopy);
    char *text = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
    int(*brackets)[2] = (int(*)[2])calloc(MAX_BRACKET_COUNT, sizeof(int[2]));
    int bracketDifference = 0, currentBracketsIndex = 0;
    for (int i = 0; textCopy[i] != '\0'; i++)
    {
        if (textCopy[i] == '{')
        {
            bracketDifference++;
            if (bracketDifference == 1)
                brackets[currentBracketsIndex][0] = i;
        }
        if (textCopy[i] == '}')
        {
            bracketDifference--;
            if (bracketDifference == 0)
            {
                brackets[currentBracketsIndex][1] = i;
                currentBracketsIndex++;
            }
        }
    }

    int length;
    for (int index = 0; textCopy[index] != '\0'; index++)
    {
        length = strlen(text);
        int currentBracket = findBracketRange(index, brackets);
        if (currentBracket == NOT_IN_BRACKET_RANGE)
        {
            text[length++] = textCopy[index];
            text[length] = '\0';
            continue;
        }
        char *bracketText = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
        copyStringRange(bracketText, textCopy, brackets[currentBracket][0], brackets[currentBracket][1] + 1);
        bracketsAutoIndent(bracketText, 0);
        strcat(text, bracketText);
        free(bracketText);
        index = brackets[currentBracket][1];
    }
    free(textCopy);
    // for (int i = 0; text[i] != '\0'; i++)
    // {
    fprintf(fptmp, "%s", text);
    // }
    fclose(fptmp);
    if (remove(fileName) != 0)
    {
        printf("Couldn't compelete the autoindent (can't remove the source file)\n");
        return false;
    }
    if (rename(TEMP_ADDRESS, fileName) != 0)
    {
        printf("Couldn't compelete the autoindent (can't rename the temp file)\n");
        return false;
    }
    return true;
}

void bracketsAutoIndent(char *text, int leadingSpaceCount)
{
    char *textCopy = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
    strcpy(textCopy, text);
    strcpy(text, "");
    int(*brackets)[2] = (int(*)[2])calloc(MAX_BRACKET_COUNT, sizeof(int[2]));
    fillBracketsArray(textCopy, brackets);
    int length = 0;
    text[length++] = '{';
    text[length++] = '\n';
    for (int i = 0; i < leadingSpaceCount + 4; i++)
        text[length++] = ' ';
    text[length] = '\0';
    for (int index = 1; textCopy[index + 1] != '\0'; index++)
    {
        length = strlen(text);
        if (textCopy[index - 1] == '}')
            for (int i = 0; i < leadingSpaceCount + 4; i++)
                text[length++] = ' ';
        int currentBracket = findBracketRange(index, brackets);
        if (currentBracket == NOT_IN_BRACKET_RANGE)
        {
            text[length++] = textCopy[index];
            if (textCopy[index] == '\n')
                for (int i = 0; i < leadingSpaceCount + 4; i++)
                    text[length++] = ' ';
            text[length] = '\0';
            continue;
        }
        char *bracketText = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
        copyStringRange(bracketText, textCopy, brackets[currentBracket][0], brackets[currentBracket][1] + 1);
        bracketsAutoIndent(bracketText, leadingSpaceCount + 4);
        strcat(text, bracketText);
        free(bracketText);
        index = brackets[currentBracket][1];
    }
    length = strlen(text);
    if (textCopy[strlen(textCopy) - 2] != '}')
        text[length++] = '\n';
    for (int i = 0; i < leadingSpaceCount; i++)
        text[length++] = ' ';
    text[length++] = '}';
    text[length++] = '\n';
    text[length] = '\0';
    free(textCopy);
}

bool copyFileContentToClipboard(char *fileName, int linePos, int charPos, int size, bool isForward)
{
    FILE *sourceptr = fopen(fileName, "r");
    char copyingStr[MAX_STREAM_LENGTH];
    if (!seekNlines(linePos - 1, sourceptr))
    {
        printf("Line position too big\n");
        return false;
    }
    if (!seekNchars(charPos, true, sourceptr))
    {
        printf("Char position invalid\n");
        return false;
    }
    if (!isForward)
        seekNchars(size, false, sourceptr);
    char c;
    int i;
    for (i = 0; i < size && (c = fgetc(sourceptr)) != EOF; i++)
        copyingStr[i] = c;
    copyingStr[i] = '\0';
    copyStrToClipboard(copyingStr);
    fclose(sourceptr);
    return true;
}

bool cutFileContentToClipboard(char *fileName, int linePos, int charPos, int size, bool isForward)
{
    if (!copyFileContentToClipboard(fileName, linePos, charPos, size, isForward))
        return false;
    if (!removeText(fileName, linePos, charPos, -1, size, isForward))
        return false;
    return true;
}

bool pasteFromClipboard(char *fileName, int linePos, int charPos)
{
    char clipboardText[MAX_STREAM_LENGTH];
    retrieveStrFromClipboard(clipboardText);
    if (!insertText(fileName, clipboardText, linePos, charPos, -1))
        return false;
    return true;
}

void copyStrToClipboard(const char *str)
{
    const int len = strlen(str) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(char));
    memcpy(GlobalLock(hMem), str, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

void retrieveStrFromClipboard(char *str)
{
    OpenClipboard(0);
    HANDLE hClipboardData = GetClipboardData(CF_TEXT);
    char *pchData = (char *)GlobalLock(hClipboardData);
    strcpy(str, pchData);
    GlobalUnlock(hClipboardData);
    CloseClipboard();
}

void autoIndentInitialize(char *text)
{
    char textCopy[MAX_STREAM_LENGTH];
    strcpy(textCopy, text);
    strcpy(text, "");
    int lastCopiedIndex = -1;

    for (int mainIterator = 0; textCopy[mainIterator] != '\0'; mainIterator++)
    {
        if (textCopy[mainIterator] == '{' || textCopy[mainIterator] == '}')
        {
            // before the { or }
            if (mainIterator != 0)
            {
                bool foundNonWhiteChar = false;
                int lastNonWhiteChar;
                for (lastNonWhiteChar = mainIterator - 1; lastNonWhiteChar >= 0; lastNonWhiteChar--)
                {
                    if (textCopy[lastNonWhiteChar] == '{' || textCopy[lastNonWhiteChar] == '}')
                        break;
                    if (textCopy[lastNonWhiteChar] != '\n' && textCopy[lastNonWhiteChar] != '\t' && textCopy[lastNonWhiteChar] != ' ')
                    {
                        foundNonWhiteChar = true;
                        break;
                    }
                }
                char *tempText = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
                copyStringRange(tempText, textCopy, lastCopiedIndex + 1, lastNonWhiteChar + 1);
                strcat(text, tempText);
                free(tempText);
                if (foundNonWhiteChar && textCopy[mainIterator] == '{')
                    strcat(text, " ");
            }
            int length = strlen(text);
            text[length] = textCopy[mainIterator];
            text[length + 1] = '\0';

            // after the { or }
            int nextNonWhiteChar;
            for (nextNonWhiteChar = mainIterator + 1; textCopy[nextNonWhiteChar] != '\0'; nextNonWhiteChar++)
            {
                if (textCopy[nextNonWhiteChar] == '{' || textCopy[nextNonWhiteChar] == '}')
                    break;
                if (textCopy[nextNonWhiteChar] != '\n' && textCopy[nextNonWhiteChar] != '\t' && textCopy[nextNonWhiteChar] != ' ')
                    break;
            }
            lastCopiedIndex = nextNonWhiteChar - 1;
        }
    }
    char *tempText = (char *)calloc(MAX_STREAM_LENGTH, sizeof(char));
    copyStringRange(tempText, textCopy, lastCopiedIndex + 1, -1);
    strcat(text, tempText);
    free(tempText);
}

void fillBracketsArray(const char *text, int (*brackets)[2])
{
    int bracketDifference = 0, currentBracketsIndex = 0;
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '{')
        {
            bracketDifference++;
            if (bracketDifference == 2)
                brackets[currentBracketsIndex][0] = i;
        }
        if (text[i] == '}')
        {
            bracketDifference--;
            if (bracketDifference == 1)
            {
                brackets[currentBracketsIndex][1] = i;
                currentBracketsIndex++;
            }
        }
    }
}

int findBracketRange(int index, const int (*brackets)[2])
{
    for (int i = 0; brackets[i][1] != 0; i++)
        if (index == brackets[i][0])
            return i;
    return NOT_IN_BRACKET_RANGE;
}

void deleteAllBackups(const char *dirName)
{
    DIR *dp = opendir(dirName);
    if (dp == NULL)
    {
        printf("Directory to delete all backups doesn't exist\n");
        return;
    }
    struct dirent *dir;
    while ((dir = readdir(dp)) != NULL)
    {
        char dPath[MAX_PATH];
        strcpy(dPath, "");
        sprintf(dPath, "%s/%s", dirName, dir->d_name);
        if (dir->d_type != DT_DIR && strncmp(dir->d_name, UNDO_SUFFIX, strlen(UNDO_SUFFIX)) == 0)
            remove(dPath);
        else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            deleteAllBackups(dPath);
    }
}

void deleteLastBackup(const char *fileName)
{
    char undoPath[MAX_PATH];
    generateUndoPath(undoPath, fileName, getLastUndoNumber(fileName) - 1);
    remove(undoPath);
}

void backupForUndo(const char *fileName)
{
    char undoPath[MAX_PATH];
    generateUndoPath(undoPath, fileName, getLastUndoNumber(fileName));
    copyFile(fileName, undoPath);
    makeFileHidden(undoPath);
}

int getLastUndoNumber(const char *fileName)
{
    int num = 1;
    for (; num < 100; num++)
    {
        char undoPath[MAX_PATH];
        generateUndoPath(undoPath, fileName, num);
        if (access(undoPath, F_OK) != 0)
            return num;
    }
    return 1;
}

void generateUndoPath(char *undoPath, const char *fileName, int num)
{
    strcpy(undoPath, "");
    num %= 100;
    char *lastSlashPointer = strrchr(fileName, '/');
    int lastSlashIndex = -1;
    if (lastSlashPointer != NULL)
    {
        lastSlashIndex = strrchr(fileName, '/') - fileName;
        copyStringRange(undoPath, fileName, 0, lastSlashIndex + 1);
    }
    strcat(undoPath, UNDO_SUFFIX);
    strcat(undoPath, "_");
    char pureFileName[MAX_PATH];
    copyStringRange(pureFileName, fileName, lastSlashIndex + 1, -1);
    strcat(undoPath, pureFileName);
    strcat(undoPath, "_");
    char numStr[3];
    sprintf(numStr, "%0.2d", num);
    strcat(undoPath, numStr);
    fixPathString(undoPath);
}

void handleWildCards(char *str, bool *leadingWC, bool *endingWC)
{
    if (str[0] == '*')
    {
        *leadingWC = true;
        for (int i = 0; 1; i++)
        {
            str[i] = str[i + 1];
            if (str[i] == '\0')
                break;
        }
    }
    int length = strlen(str);
    if (str[length - 1] == '*' && str[length - 2] != '\\')
    {
        *endingWC = true;
        str[length - 1] = '\0';
    }
    // clearing the "\*"s
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\\' && str[i + 1] == '*')
        {
            for (int j = i; 1; j++)
            {
                str[j] = str[j + 1];
                if (str[j] == '\0')
                    break;
            }
        }
    }
}

void handleNewlines(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\\' && str[i + 1] == 'n')
        {
            str[i] = '\n';
            i++;
            for (int j = i; 1; j++)
            {
                str[j] = str[j + 1];
                if (str[j] == '\0')
                    break;
            }
        }
    }
}

void splitPaths(const char *str, char (*paths)[MAX_PATH])
{
    int pathStartIndex = 0, pathEndIndex, pathsIndex = 0;
    char c;
    while (1)
    {
        if (str[pathStartIndex] == '"')
        {
            pathStartIndex++;
            for (pathEndIndex = pathStartIndex; str[pathEndIndex] != '"'; pathEndIndex++)
                if (str[pathEndIndex] == '\0')
                    return;
            copyStringRange(paths[pathsIndex], str, pathStartIndex, pathEndIndex);
            fixPathString(paths[pathsIndex]);
            pathStartIndex = pathEndIndex + 2;
        }
        else
        {
            for (pathEndIndex = pathStartIndex; str[pathEndIndex] != ' '; pathEndIndex++)
                if (str[pathEndIndex] == '\0')
                    break;
            copyStringRange(paths[pathsIndex], str, pathStartIndex, pathEndIndex);
            fixPathString(paths[pathsIndex]);
            pathStartIndex = pathEndIndex + 1;
        }
        if (str[pathStartIndex - 1] == '\0')
            break;
        pathsIndex++;
    }
}

bool readAndWriteNlines(int n, FILE *tempptr, FILE *sourceptr)
{
    char c;
    int lineCounter = 1;
    while (lineCounter != n)
    {
        while (1)
        {
            c = fgetc(sourceptr);
            if (c == EOF)
                return false;
            if (c == '\n')
            {
                lineCounter++;
                break;
            }
            fprintf(tempptr, "%c", c);
        }
        fprintf(tempptr, "\n");
    }
    return true;
}

bool readAndWriteNchars(int n, FILE *tempptr, FILE *sourceptr)
{
    char c;
    int charCounter = 0;
    while (charCounter != n)
    {
        c = fgetc(sourceptr);
        if (c == EOF || c == '\n')
            return false;
        fprintf(tempptr, "%c", c);
        charCounter++;
    }
    return true;
}

bool readAndWriteNseeks(int n, FILE *tempptr, FILE *sourceptr)
{
    char c;
    int charCounter = 0;
    for (int charCounter = 0; charCounter != n; charCounter++)
    {
        c = fgetc(sourceptr);
        if (c == EOF)
            break;
        fprintf(tempptr, "%c", c);
    }
}

bool seekNlines(int n, FILE *sourceptr)
{
    char c;
    int lineCounter = 0;
    while (lineCounter != n)
    {
        while (1)
        {
            c = fgetc(sourceptr);
            if (c == EOF)
                return false;
            if (c == '\n')
            {
                lineCounter++;
                break;
            }
        }
    }
    return true;
}

bool seekNchars(int n, bool isForward, FILE *sourceptr)
{
    int charCounter = 0;
    char c;
    if (isForward)
    {
        while (charCounter != n)
        {
            c = fgetc(sourceptr);
            charCounter++;
            if (c == EOF)
                return false;
        }
    }
    else
    {
        while (charCounter != n)
        {
            if (fseek(sourceptr, -1, SEEK_CUR) != 0)
                return false;
            c = fgetc(sourceptr);
            if (c == '\n')
                fseek(sourceptr, -2, SEEK_CUR);
            else
                fseek(sourceptr, -1, SEEK_CUR);
            charCounter++;
        }
    }
    return true;
}

void writeStrToFile(char *text, FILE *tempptr, FILE *sourceptr)
{
    bool backSlashMode = false;
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (backSlashMode)
        {
            if (text[i] == '\\')
                fprintf(tempptr, "\\");
            else if (text[i] == 'n')
                fprintf(tempptr, "\n");
            backSlashMode = false;
            continue;
        }
        if (text[i] == '\\')
        {
            backSlashMode = true;
            continue;
        }
        if (text[i] == '\r')
            continue;
        fprintf(tempptr, "%c", text[i]);
    }
}

bool copyFile(const char *sourceFileName, const char *destFileName)
{
    FILE *sourcePtr;
    sourcePtr = fopen(sourceFileName, "r");
    if (sourcePtr == NULL)
    {
        printf("Error occured while reading the source file\n");
        return false;
    }
    FILE *destPtr;
    destPtr = fopen(destFileName, "w");
    if (destPtr == NULL)
    {
        printf("Error occured while creating the destination file\n");
        return false;
    }
    char c = fgetc(sourcePtr);
    while (c != EOF)
    {
        fputc(c, destPtr);
        c = fgetc(sourcePtr);
    }
    fclose(sourcePtr);
    fclose(destPtr);
    return true;
}

bool createFileAndDirs(char *fileName)
{
    createAllDirs(fileName);
    if (access(fileName, F_OK) == 0)
    {
        printf("File already exists\n");
        return false;
    }
    return createFile(fileName);
}

bool createFile(const char *fileName)
{
    FILE *fp;
    fp = fopen(fileName, "w");
    if (fp == NULL)
    {
        printf("Error occured while creating the file\n");
        return false;
    }
    fclose(fp);
    return true;
}

void createAllDirs(const char *dirName)
{
    char pathToMake[MAX_PATH];
    for (int i = 0; dirName[i] != '\0'; i++)
    {
        if (dirName[i] == '/')
        {
            pathToMake[i] = '\0';
            if (!directoryExists(pathToMake))
                mkdir(pathToMake);
        }
        pathToMake[i] = dirName[i];
    }
}

bool directoryExists(const char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode))
        return 1;
    return 0;
}

void makeFileHidden(const char *fileName)
{
    DWORD dwattrs = GetFileAttributes(fileName);
    if ((dwattrs & FILE_ATTRIBUTE_HIDDEN) == 0)
        SetFileAttributes(fileName, dwattrs | FILE_ATTRIBUTE_HIDDEN);
}

void makeFileNotHidden(const char *fileName)
{
    DWORD dwattrs = GetFileAttributes(fileName);
    if ((dwattrs & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
        SetFileAttributes(fileName, dwattrs & (~FILE_ATTRIBUTE_HIDDEN));
}

void printTreeItem(const char *path, int depth)
{
    depth -= 1;
    for (int i = 0; i < 4 * (depth - 1); i++)
        printf(" ");
    if (depth != 0)
        printf("L___");
    printf("%s\n", path);
}

void printCompareComplex(const char *line, int wordStart, int wordEnd)
{
    for (int i = 0; i < wordStart; i++)
        printf("%c", line[i]);
    printf(">>");
    for (int i = wordStart; i <= wordEnd; i++)
        printf("%c", line[i]);
    printf("<<");
    for (int i = wordEnd + 1; line[i] != '\0'; i++)
        printf("%c", line[i]);
    printf("\n");
}

void inputLine(char *str)
{
    char c;
    int inputIndex = 0;
    c = getchar();
    while (c != '\n' && c != EOF && inputIndex != MAX_CMD_LINE_LENGTH)
    {
        str[inputIndex++] = c;
        c = getchar();
    }
    str[inputIndex] = '\0';
}

void inputLineFromFile(FILE *fp, char *str)
{
    char c;
    int inputIndex = 0;
    c = fgetc(fp);
    while (c != '\n' && c != EOF && inputIndex != MAX_FILE_LINE_LENGTH - 1)
    {
        str[inputIndex++] = c;
        c = fgetc(fp);
    }
    str[inputIndex] = '\0';
}

void fileToString(FILE *fp, char *str)
{
    char c;
    int i = 0;
    while ((c = fgetc(fp)) != EOF)
        str[i++] = c;
    str[i] = '\0';
}

bool handleDoubleQuotation(char *str)
{
    if (!removeDoubleQuotations(str) && strchr(str, ' ') != NULL)
        return false;
    return true;
}

bool removeDoubleQuotations(char *str)
{
    if (str[0] != '"' || str[strlen(str) - 1] != '"')
        return false;
    for (int i = 0; 1; i++)
    {
        str[i] = str[i + 1];
        if (str[i] == '\0')
        {
            str[i - 1] = '\0';
            break;
        }
    }
    return true;
}

int findMatchingWord(const char *str, const char *match)
{
    int matchIndex = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == match[matchIndex])
        {
            matchIndex++;
            if (match[matchIndex] == '\0')
                return i + 1;
        }
        else
        {
            matchIndex = 0;
            if (str[i] == match[matchIndex])
                matchIndex++;
        }
    }
    return -1;
}

bool findMatchFromIndex(const char *str, const char *match, int startingIndex, bool isForward)
{
    int matchIndex, iterator = 1;
    int matchLen = strlen(match);
    if (isForward)
        matchIndex = 0;
    else
    {
        iterator = -1;
        matchIndex = matchLen - 1;
    }
    for (int i = startingIndex; matchLen != 0; matchLen--)
    {
        if (i == -1 || matchIndex == -1 || match[matchIndex] == '\0' || str[i] == '\0')
            return false;
        if (str[i] != match[matchIndex])
            return false;
        i += iterator;
        matchIndex += iterator;
    }
    return true;
}

void copyStringRange(char *dest, const char *source, int start, int end)
{
    if (end == -1)
        end = MAX_STREAM_LENGTH;
    if (start >= end)
        return;
    int destIndex = 0;
    for (int i = start; i < end && source[i] != '\0'; i++)
        dest[destIndex++] = source[i];
    dest[destIndex] = '\0';
}

bool copyNthWord(char *dest, const char *str, int n)
{
    int start, end;
    if (!findNthWord(str, n, &start, &end))
        return false;
    copyStringRange(dest, str, start, end + 1);
    return true;
}

bool findNthWord(const char *str, int n, int *startIndex, int *endIndex)
{
    int wordNum = 1;
    int i = 0;
    while (str[i] == ' ' || str[i] == '\n' || str[i] == '\t')
        i++;
    while (wordNum < n)
    {
        if (str[i] == '\0')
            return false;
        if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t')
        {
            while (str[i] == ' ' || str[i] == '\n' || str[i] == '\t')
                i++;
            wordNum++;
            if (str[i] == '\0')
                return false;
            continue;
        }
        i++;
    }
    *startIndex = i;
    while (str[i] != ' ' && str[i] != '\0' && str[i] != '\n' && str[i] != '\t')
        i++;
    *endIndex = i - 1;
    return true;
}

void fixPathString(char *path)
{
    if (path[0] != '/')
        return;
    for (int i = 0; 1; i++)
    {
        path[i] = path[i + 1];
        if (path[i] == '\0')
        {
            if (path[i - 1] == '/')
                path[i - 1] = '\0';
            break;
        }
    }
}
