//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to objects representing players in extensive forms
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef EFPLAYER_H
#define EFPLAYER_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

struct gbt_efg_player_rep;
class efgGame;
class Infoset;

class gbtEfgPlayer {
friend class efgGame;
protected:
  struct gbt_efg_player_rep *rep;

public:
  gbtEfgPlayer(void);
  gbtEfgPlayer(gbt_efg_player_rep *);
  gbtEfgPlayer(const gbtEfgPlayer &);
  ~gbtEfgPlayer();

  gbtEfgPlayer &operator=(const gbtEfgPlayer &);

  bool operator==(const gbtEfgPlayer &) const;
  bool operator!=(const gbtEfgPlayer &) const;

  bool IsNull(void) const;

  efgGame *GetGame(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);
  int GetId(void) const;

  bool IsChance(void) const;
  int NumInfosets(void) const;
  Infoset *GetInfoset(int p_index) const;
};

class gbtEfgPlayerIterator {
private:
  int m_index;
  efgGame &m_efg;

public:
  gbtEfgPlayerIterator(efgGame &p_efg);
  
  gbtEfgPlayer operator*(void) const;
  gbtEfgPlayerIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};

class gbtEfgInfosetIterator {
private:
  int m_index;
  gbtEfgPlayer m_player;

public:
  gbtEfgInfosetIterator(const gbtEfgPlayer &p_player);
  
  Infoset *operator*(void) const;
  gbtEfgInfosetIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};

#endif    // EFPLAYER_H


