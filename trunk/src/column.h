/*
  Copyright (C) 2005 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

class column_display
{
	public:
		column_display(int columns, char separator);
		void add(char * text, int column = 0);
		void dump();
		void clear();
		const int cols;
		const char sep;
	private:
		vector<string> flesh[5];
		
};

