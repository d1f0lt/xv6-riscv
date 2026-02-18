#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

const int MAX_NUM_LENGTH = 10;
const int MAX_INPUT_STRING_LENGTH = 512;

int readLine(char *buffer) {
    char curChar;
    int curPos = 0;
    int cnt = read(0, &curChar, 1);
    
    while (curChar != '\n' && cnt != 0) {
        if (cnt < 0) {
            fprintf(2, "Error while read\n");
            return 1;
        }
        if (curPos >= MAX_INPUT_STRING_LENGTH) {
            fprintf(2, "Too long string\n");
            return 1;
        }
        buffer[curPos++] = curChar;
        cnt = read(0, &curChar, 1);
    }
    buffer[curPos] = '\0';
    return 0;
}

int isDigit(char ch) {
    if (ch >= '0' && ch <= '9') return 1;
    return 0;
}

int readInt(char *buffer, int *bufferPos, int *res) {
    int sign = 1, curPos = 0;
    char numBuffer[MAX_NUM_LENGTH + 1];

    while (buffer[*bufferPos] == ' ') *bufferPos = *bufferPos + 1;
    
    if (buffer[*bufferPos] == '-') {
        sign = -1;
        *bufferPos = *bufferPos + 1;
    }

    char curChar = buffer[*bufferPos];
    while (curChar != ' ' && curChar != '\0') {
        if (curPos >= MAX_NUM_LENGTH) {
            fprintf(2, "Too long num\n");
            return -1;
        }
        if (isDigit(curChar) == 1) 
            numBuffer[curPos++] = curChar;
    
        else {
            fprintf(2, "Invalid char\n");
            return -1;
        }
        *bufferPos = *bufferPos + 1;
        curChar = buffer[*bufferPos];
    }
    if (curPos == 0) {
        fprintf(2, "Invalid input\n");
        return -1;
    }

    numBuffer[curPos] = '\0';
    *res = atoi(numBuffer) * sign;
    return 0;
}

int readToTheEnd(char *buffer, int *bufferPos) { // right trim
    char curChar = ' ';

    while (curChar != '\0') {
        if (curChar != ' ') {
            fprintf(2, "Invalid input\n");
            return -1;
        }
        curChar = buffer[*bufferPos];
        *bufferPos = *bufferPos + 1;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    int num1 = 0, num2 = 0, curPos = 0;
    char buffer[MAX_INPUT_STRING_LENGTH + 1];
    printf("Enter 2 numbers separated by a space: ");
    if (readLine(buffer) != 0)
        return 1;

    // printf("Your input: \"%s\"\n", buffer);

    if (readInt(buffer, &curPos, &num1) != 0)
        return 1;
    if (readInt(buffer, &curPos, &num2) != 0)
        return 1;

    // printf("1: %d, 2: %d\n", num1, num2);

    if (readToTheEnd(buffer, &curPos) != 0)
        return 1;

    int res = add(num1, num2);
    printf("Result: %d\n", res);
    return 0;
}