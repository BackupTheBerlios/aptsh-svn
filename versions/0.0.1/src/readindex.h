/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

int read_index1(char * filename, char * hm_file);
int read_index2(char * filename, char * hm_file);


int hm;
int hm_i;

char ** pkgs; /* all packages */
char ** pkgs_i; /* installed packages */

