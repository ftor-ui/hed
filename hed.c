#include <string.h>
#include <stdlib.h>

#include "ffuncs.h"

int main(int const argc, char const **argv)
{
	if (argc > 2)
		for (int i = 1; i < argc; i++)
		{
			if (!strcmp(argv[i], "-w"))
				width = atoi(argv[i]);
			else if (!strcmp(argv[i], "-h"))
				height = atoi(argv[i]);
		}
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
