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

FILE *file = NULL;
char unsigned *fileBuffer = NULL;
int long fileSize = 0;
char unsigned pressKey = '\0';
int short unsigned mode = 0; // 0 - normal mode, 1 - hex mode, 2 - ascii mode
int short unsigned quit = 1; // 1 - don't quit, 0 - quit
int long curX = 0;
int long curY = 0;
int long offset = 0; // dividend of width
int short unsigned digit = 1; // 1 - high 4 bits, 0 - low 4 bits
int short unsigned width = WIDTH;
int short unsigned height = HEIGHT;
int long borderTop = 0;
int long borderBottom = HEIGHT - 1;

// For ChangeLog(), Undo(), Redo() functions
struct _byte_t
{
	long int offset;
	char unsigned byte;
} changeLog[ALLOC_SIZE_CL] = {0};
int long indexChangeLog = -1; // -1 - mean nothing changes
struct _byte_t lastUndo = {-1};

// For FindUp(), FindDown() functions
char word[ALLOC_SIZE_FW + 1] = {0};
int long wordLength = 0;
int long findWord = {-1};

// For CursorJumpTo() function
int long address = 0;
char address_str[9] = {0};

void OpenFile(int const argc, char const **argv);
void InitScreen();
void GetKey();
void KeyLogic();
void WriteOut();
void WriteIn();
void ChangeLog();
void Undo();
void Redo();

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

int main(int const argc, char const **argv)
{
	OpenFile(argc, argv);
	WriteOut();

	while (quit)
	{
		GetKey();
		KeyLogic();
		WriteOut();
	}

	return 0;
}

void OpenFile(int const argc, char const **argv)
{
	if (argc < 2)
	{
		printf("ERROR: File doesn't specified!\n");
		exit(EXIT_FAILURE);
	}

	file = fopen(argv[1], "r+b");
	if (file == NULL)
	{
		printf("ERROR: File cannot be open or file doesn't exist\n");
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fileBuffer = (char unsigned *)malloc(sizeof(char unsigned) * fileSize);
	fseek(file, 0, SEEK_SET);
	for (long int i = 0; i < fileSize; i++)
		if ((fileBuffer[i] = fgetc(file)) == EOF)
			break;
}

void GetKey()
{
	pressKey = getch();
}

void KeyLogic()
{
	if (mode != 0)
	{
		if (pressKey == 27)
		{
			digit = 1;
			mode = 0;
			return;
		}

		else if (pressKey == 8)
		{
			CursorLeft();	
			return;
		}

		if (curY * width + curX > fileSize)
			return;
	}

	if (mode == 0)
	switch (pressKey)
	{
		case 's':
			mode = 1;
			return;
		case 'a':
			mode = 2;
			return;
		case 'q':
			quit = 0;
			return;
		case 'w':
			WriteIn();
			return;
		case 'u':
			Undo();
			return;
		case 'r':
			Redo();
			return;
		case '@':
			printf("@");
			scanf("%8s", address_str);
			address = strtol(address_str, NULL, 16);
			CursorJumpTo();
			return;
		case '?':
			FindUp();
			return;
		case '/':
			FindDown();
			return;
		case 'h':
			CursorLeft();
			return;
		case 'j':
			CursorDown();	
			return;
		case 'k':
			CursorUp();	
			return;
		case 'l':
			CursorRight();
			return;
	}

	else if (mode == 1)
	{
		ChangeLog();

		if (digit == 1)
		{
			fileBuffer[curY * width + curX] %= width;
			fileBuffer[curY * width + curX] += strtol(&pressKey, NULL, 16) * width;
			digit = 0;
		}
		else
		{
			fileBuffer[curY * width + curX] -= fileBuffer[curY * width + curX] % width;
			fileBuffer[curY * width + curX] += strtol(&pressKey, NULL, 16);
			digit = 1;

			CursorRight();	
		}
	}

	else if (mode == 2)
	{
		ChangeLog();

		fileBuffer[curY * width + curX] = pressKey;

		CursorRight();	
	}
}

void WriteOut()
{
	// I fix this shit later :/
	#ifdef _WIN32
	system("cls");
	#elif __linux__
	system("clear");
	#endif

	// The Super Fucking Awesome Identation
	for (int i = 0; i < 5 * width / 2; i++)
		printf(" ");

	switch (mode)
	{
		case 0:
			printf("NORMAL MODE\n");
			break;
		case 1:
			printf(" HEX MODE\n");
			break;
		case 2:
			printf("ASCII MODE\n");
	}

	for (int long i = 0; i < width * height; i++)
	{
		if (i % width == 0)
			printf("%08X: ", offset + i);

		if (offset + i + 1 > fileSize)
			if (curY * width + curX == offset + i)
				printf(">XX<");
			else
				printf(" XX ");

		else if (curY * width + curX == offset + i) 
			printf(">%02X<", fileBuffer[offset + i]);

		else
			printf(" %02X ", fileBuffer[offset + i]);

		if ((i + 1) % width == 0)
		{
			printf(" ");

			for (int long k = i + 1 - width; k <= i; k++)
			{
				if (fileBuffer[offset + k] > 31 && fileBuffer[offset + k] != 127 && offset + k + 1 <= fileSize)
					printf("%c", fileBuffer[offset + k]);
				else
					printf(".");
			}

			printf("\n");
		}
	}
}

void WriteIn()
{
	fseek(file, 0, SEEK_SET);
	for (int i = 0; i < fileSize; i++)
		fputc(fileBuffer[i], file);
}

void ChangeLog()
{
	if (indexChangeLog == ALLOC_SIZE_CL - 1)
	{
		for (int i = 0; i < ALLOC_SIZE_CL - 1; i++)
			changeLog[i] = changeLog[i + 1];
		changeLog[indexChangeLog].offset = curY * width + curX;
		changeLog[indexChangeLog].byte = fileBuffer[curY * width + curX];

		return;
	}

	changeLog[indexChangeLog + 1].offset = curY * width + curX;
	changeLog[indexChangeLog + 1].byte = fileBuffer[curY * width + curX];
	indexChangeLog++;

	lastUndo.offset = -1;
}

void Undo()
{
	if (indexChangeLog == -1)
		return;

	address = changeLog[indexChangeLog].offset;
	CursorJumpTo();

	lastUndo.offset = changeLog[indexChangeLog].offset;
	lastUndo.byte = fileBuffer[changeLog[indexChangeLog].offset];

	fileBuffer[changeLog[indexChangeLog].offset] = changeLog[indexChangeLog].byte;
	indexChangeLog--;
}

void Redo()
{
	if (lastUndo.offset == -1)
		return;

	address = lastUndo.offset;
	CursorJumpTo();

	if (indexChangeLog == ALLOC_SIZE_CL - 1)
	{
		for (int i = 0; i < ALLOC_SIZE_CL - 1; i++)
			changeLog[i] = changeLog[i + 1];
		changeLog[indexChangeLog].offset = lastUndo.offset;
		changeLog[indexChangeLog].byte = fileBuffer[lastUndo.offset];
		fileBuffer[lastUndo.offset] = lastUndo.byte;
		return;
	}
	changeLog[indexChangeLog + 1].offset = lastUndo.offset;
	changeLog[indexChangeLog + 1].byte = fileBuffer[lastUndo.offset];
	indexChangeLog++;
	fileBuffer[lastUndo.offset] = lastUndo.byte;

	lastUndo.offset = -1;
}

void FindUp()
{
	printf("?");
	scanf("%50s", word);
	for (int long i = 0; word[i] != '\0'; i++)
		wordLength++;
	
	if (wordLength > curY * width + curX || wordLength > fileSize)
		return;

	for (int long i = curY * width + curX - wordLength; i >= 0; i--)
	{
		for (int long k = 0; k < wordLength; k++)
		{
			if (word[k] != fileBuffer[i + k])
				break;

			if (k == wordLength - 1)
			{
				findWord = i;

				address = findWord;
				CursorJumpTo();
				WriteOut();
				printf("?%s", word);

				pressKey = 0;
				while (pressKey != 10 && pressKey != 13 && pressKey != 27)
					GetKey();

				if (pressKey == 27)
					return;
			}
		}
	}		

	wordLength = 0;
}

void FindDown()
{
	printf("/");
	scanf("%50s", word);
	for (int long i = 0; word[i] != '\0'; i++)
		wordLength++;

	if (wordLength > fileSize - (curY * width + curX))
		return;

	for (int long i = curY * width + curX; i < fileSize - wordLength; i++)
	{
		for (int long k = 0; k < wordLength; k++)
		{
			if (word[k] != fileBuffer[i + k])
				break;

			if (k == wordLength - 1)
			{
				findWord = i;

				address = findWord;
				CursorJumpTo();
				WriteOut();
				printf("/%s", word);

				pressKey = 0;
				while (pressKey != 10 && pressKey != 13 && pressKey != 27)
					GetKey();

				if (pressKey == 27)
					return;
			}
		}
	}

	wordLength = 0;
}

void CursorUp()
{
	curY = curY == 0 ? 0 : --curY;
	if (borderTop != 0 && curY < borderTop)
	{
		offset -= width;
		borderTop--;
		borderBottom--;
	}
}

void CursorDown()
{
	curY = curY == fileSize / width  ? curY : ++curY;
	if (borderBottom != fileSize / width && curY > borderBottom)
	{
		offset += width;
		borderTop++;
		borderBottom++;
	}

}

void CursorLeft()
{
	if (curX == 0)
	{
		curX = curY > 0 ? width - 1 : curX; 
		curY = curY > 0 ? --curY : curY;
	}
	else
		curX--;
	
	if (borderTop != 0 && curY < borderTop)
	{
		offset -= 16;
		borderTop--;
		borderBottom--;
	}
}

void CursorRight()
{
	curY = curX == width - 1 ? ++curY : curY;
	curX = curX == width - 1 ? 0 : ++curX;
	curY = curY > fileSize / width ? --curY : curY;
	if (borderBottom != fileSize / width && curY > borderBottom)
	{
		offset += width;
		borderTop++;
		borderBottom++;
	}
}

void CursorJumpTo()
{
	if (address > fileSize - 1 || address < 0)
		return;
	
	curY = address / width;
	curX = address % width;
	offset = curY * width;

	borderTop = curY;
	borderBottom = curY + height - 1;
}

#ifdef __linux__
char unsigned getch()
{
	struct termios oldt, newt;
	char unsigned ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}
#endif
