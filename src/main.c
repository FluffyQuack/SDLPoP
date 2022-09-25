/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2022  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#include "common.h"
#include "Print.h"

int main(int argc, char *argv[])
{
	//Fluffy (Multiplayer)
	int i = 1, startNetwork = 0, port = -1;
	char *ipString = 0;
	char *nameString = 0;
	unsigned char colourRed = 252;
	unsigned char colourGreen = 252;
	unsigned char colourBlue = 216;
	while(argc > i)
	{
		if (argv[i][0] == '-')
		{
			if (_stricmp(argv[i], "-ip") == 0)
			{
				if (argc > i)
				{
					ipString = argv[i + 1];
					i++;
				}
			}
			else if (_stricmp(argv[i], "-port") == 0)
			{
				if (argc > i)
				{
					port = atoi(argv[i + 1]);
					i++;
				}
			}
			else if (_stricmp(argv[i], "-color") == 0)
			{
				if (argc > i + 2)
				{
					colourRed = atoi(argv[i + 1]);
					colourGreen = atoi(argv[i + 2]);
					colourBlue = atoi(argv[i + 3]);
					i += 3;
				}
			}
			else if (_stricmp(argv[i], "-server") == 0)
				startNetwork = 1;
			else if (_stricmp(argv[i], "-client") == 0)
				startNetwork = 2;
			else if (_stricmp(argv[i], "-name") == 0)
			{
				if (argc > i)
				{
					nameString = argv[i + 1];
					i++;
				}
			}
		}
		i++;
	}
	Network_Intermediate_Start(startNetwork, port, ipString, nameString, colourRed, colourGreen, colourBlue);
	currentKidCharTable = id_chtab_2_kid; //Default player sprite data to what's used bye the local player

	g_argc = argc;
	g_argv = argv;
	pop_main();
	
	//Fluffy (Multiplayer)
	Network_Intermediate_Stop();

	return 0;
}

