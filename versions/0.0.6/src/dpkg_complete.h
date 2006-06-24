/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

class dpkg_complete
{
	public:
	dpkg_complete(char * word, char * text, int index);
		
	// Actions like -i, -s, -S
	static char * normal(const char * text, int state);
	
	// Long version of above, --install etc.
	static char * normal_double(const char * text, int state);
	
	// Completing only names of files with .deb extension
	static char * fs_deb(const char * text, int state);
		
	// This function returns pointer to the completion function we
	// must use. Actually, this is essential.
	//char* (*check(char * word, char * text, int index))(const char*, int);

	char*(*completion)(const char*, int);
};

