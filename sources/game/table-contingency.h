//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Specialization of normal form contingency for table games
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef TABLE_CONTINGENCY_H
#define TABLE_CONTINGENCY_H

#include "game-contingency.h"

//!
//! Specialization of contingency concept to games represented by tables
//! of outcomes.  The implementation avails itself of tracking the index
//! into the N-dimensional table of outcomes generated by the set of
//! strategies.
//!
class gbtTableContingencyRep : public gbtGameContingencyRep {
public:
  int m_refCount;
  gbtTableGameRep *m_nfg; 
  /// The index (minus one) of the outcome corresponding to the profile
  long m_index; 
  gbtArray<gbtTableStrategyRep *> m_profile;

  /// @name Constructors and destructor
  //@{
  gbtTableContingencyRep(gbtTableGameRep *);
  gbtGameContingencyRep *Copy(void) const;
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  bool IsDeleted(void) const { return false; }
  //@}

  /// @name Accessing the state
  //@{
  gbtGameStrategy GetStrategy(const gbtGamePlayer &) const;
  void SetStrategy(const gbtGameStrategy &);
  void SetOutcome(const gbtGameOutcome &);
  gbtGameOutcome GetOutcome(void) const;
  gbtRational GetPayoff(const gbtGamePlayer &) const;
  //@}
};

//!
//! This class sequentially visits all the contingencies (configurations
//! of individual strategies) in a game.
//!
class gbtTableContingencyIteratorRep : public gbtGameContingencyIteratorRep {
public:
  int m_refCount, m_frozen;
  gbtTableGameRep *m_nfg;
  long m_index;
  gbtArray<gbtTableStrategyRep *> m_profile;

  /// @name Constructor and destructor
  //@{
  gbtTableContingencyIteratorRep(gbtTableGameRep *);
  gbtTableContingencyIteratorRep(gbtTableGameRep *, gbtTableStrategyRep *);
  virtual ~gbtTableContingencyIteratorRep();
  gbtGameContingencyIteratorRep *Copy(void) const;
  //@}
  
  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  bool IsDeleted(void) const { return false; }
  //@}

  /// @name Iteration
  //@{
  void First(void);
  bool NextContingency(void);
  //@}

  /// @name Accessing the state
  //@{
  gbtGameStrategy GetStrategy(const gbtGamePlayer &) const;
  gbtGameOutcome GetOutcome(void) const;
  gbtRational GetPayoff(const gbtGamePlayer &p_player) const; 
  //@}
};


#endif  // TABLE_CONTINGENCY_H