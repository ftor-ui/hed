#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32 
	#include <conio.h>
	#include <io.h>
#elif __linux__
	#include <unistd.h>
	#include <termios.h>
#endif

#define WIDTH  16
#define HEIGHT 20
#define ALLOC_SIZE_FW 50
#define ALLOC_SIZE_CL 50

extern FILE *file;
extern char *filePath;
extern char unsigned *fileBuffer; 
extern int long fileSize; 
extern char unsigned pressKey; 
extern int short unsigned mode; 
extern short unsigned quit; 
extern int long curX; 
extern int long curY; 
extern int long offset; 
extern int short unsigned digit; 
extern int short unsigned width; 
extern int short unsigned height; 
extern int long borderTop; 
extern int long borderBottom; 

// For ChangeLog(), Undo(), Redo() functions
struct _byte_t
{
	long int offset;
	char unsigned byte;
	int cycleOffset;
} changeLog[ALLOC_SIZE_CL] = {0};
int long indexChangeLog = -1; // -1 - mean nothing changes
struct _byte_t lastUndo = {-1};

// For FindUp(), FindDown() functions
extern char word[ALLOC_SIZE_FW + 1]; 
extern int long wordLength; 
extern int long findWord; 

// For CursorJumpTo() function
extern int long address; 
extern char address_str[9]; 

void OpenFile(int const argc, char const **argv);
void InitScreen();
void GetKey();
void KeyLogic();
void WriteOut();
void WriteIn();
void ChangeLog();
void Undo();
void Redo();
void ByteDelete();
void ByteInsert();

// SMART Move cursor
void CursorUp();
void CursorDown();
void CursorLeft();
void CursorRight();
void CursorJumpTo();
void FindUp();
void FindDown();

#ifdef __linux__
char unsigned getch();
#endif
