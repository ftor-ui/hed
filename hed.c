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

FILE *file = NULL;
char unsigned *fileBuffer = NULL;
int long fileSize = 0;
char unsigned pressKey = '\0';
int short unsigned mode = 0; // 0 - normal mode, 1 - insert mode
int short unsigned quit = 1; // 1 - don't quit, 0 - quit
int long curX = 0;
int long curY = 0;
int long offset = 0; // dividend of width
int short unsigned digit = 1; // 1 - high 4 bits, 0 - low 4 bits
int short unsigned width = WIDTH;
int short unsigned height = HEIGHT;
int long borderTop = 0;
int long borderBottom = HEIGHT - 1;

void OpenFile(int const argc, char const **argv);
void InitScreen();
void GetKey();
void KeyLogic();
void WriteOut();
void WriteIn();

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
	switch (mode)
	{
		case 0:
			pressKey = getch();
			break;
		case 1:
			pressKey = getch();
			break;
	}
}

void KeyLogic()
{
	switch (pressKey)
	{
		case 27:
			mode = 0;
			digit = 1;
			return;
		case 'i':
			mode = 1;
			return;
	}

	if (mode == 0)
	switch (pressKey)
	{
		case 'q':
			quit = 0;
			return;
		case 's':
			WriteIn();
			return;
		case 'h':
			curX = curX == 0 ? 0 : --curX;
			return;
		case 'j':
			curY = curY == fileSize / width  ? curY : ++curY;
			if (borderBottom != fileSize / width && curY > borderBottom)
			{
				offset += width;
				borderTop++;
				borderBottom++;
			}
			return;
		case 'k':
			curY = curY == 0 ? 0 : --curY;
			if (borderTop != 0 && curY < borderTop)
			{
				offset -= width;
				borderTop--;
				borderBottom--;
			}
			return;
		case 'l':
			curX = curX == width - 1 ? curX : ++curX;
			return;
	}

	else
	{
		if (pressKey == 8)
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

			return;
		}
		if (curY * width + curX > fileSize)
			return;
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

	switch (mode)
	{
		case 0:
			printf("                                       NORMAL MODE\n");
			break;
		case 1:
			printf("                                       INSERT MODE\n");
			break;
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

	printf("                           q - quit, s - save; (In normal mode)");
}

void WriteIn()
{
	fseek(file, 0, SEEK_SET);
	for (int i = 0; i < fileSize; i++)
		fputc(fileBuffer[i], file);
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
