//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of main wxApp class
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

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/image.h"
#include "wx/splash.h"

#include "game/nfg.h"
#include "game/nfstrat.h"
#include "game/nfgciter.h"

#include "gambit.h"
#include "dlabout.h"
#include "dlnewgame.h"
#include "efgshow.h"
#include "nfgshow.h"

class Game {
public:
  efgGame *m_efg;
  EfgShow *m_efgShow;
  Nfg *m_nfg;
  NfgShow *m_nfgShow;
  gText m_fileName;

  Game(efgGame *p_efg) : m_efg(p_efg), m_efgShow(0), m_nfg(0), m_nfgShow(0) { }
  Game(Nfg *p_nfg) : m_efg(0), m_efgShow(0), m_nfg(p_nfg), m_nfgShow(0) { }
};

GambitApp::GambitApp(void)
  : m_fileHistory(5)
{ }

bool GambitApp::OnInit(void)
{
#include "bitmaps/gambit.xpm"

  wxConfig config("Gambit");
  m_fileHistory.Load(config);

  wxBitmap bitmap(wxBITMAP(gambit));
  wxSplashScreen *splash =
    new wxSplashScreen(bitmap,
		       wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
		       2000, NULL, -1, wxDefaultPosition, wxDefaultSize,
		       wxSIMPLE_BORDER | wxSTAY_ON_TOP);
#ifdef __WXMSW__
  wxYield();
#else  // !__WXMSW__
  while (splash->IsShown()) {
    wxYield();
  }
#endif  // !__WXMSW__

  efgGame *efg = new efgGame;
  efg->NewPlayer()->SetName("Player 1");
  efg->NewPlayer()->SetName("Player 2");
  efg->SetTitle("Untitled Extensive Form Game");

  EfgShow *efgShow = new EfgShow(*efg, 0);
  efgShow->SetFilename("");
  AddGame(efg, efgShow);

  // Set up the help system.
  wxInitAllImageHandlers();

  // Process command line arguments, if any.
#ifdef UNUSED
  if (argc > 1) { 
    gambitFrame->LoadFile(argv[1]);
  }
#endif  // UNUSED

  // Set current directory.
  SetCurrentDir(wxGetWorkingDirectory());

  return true;
}

GambitApp::~GambitApp()
{
  wxConfig config("Gambit");
  m_fileHistory.Save(config);

  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfg != 0) {
      delete m_gameList[i]->m_nfg;
    }
    if (m_gameList[i]->m_efg != 0) {
      delete m_gameList[i]->m_efg;
    }
    delete m_gameList[i];
  }
}

void GambitApp::OnFileNew(wxWindow *p_parent)
{
  dialogNewGame dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.CreateEfg()) {
      efgGame *efg = new efgGame;
      efg->SetTitle("Untitled Extensive Form Game");
      for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
	efg->NewPlayer()->SetName(gText("Player") + ToText(pl));
      }
      EfgShow *efgShow = new EfgShow(*efg, 0);
      efgShow->SetFilename("");
      AddGame(efg, efgShow);
    }
    else {
      Nfg *nfg = new Nfg(dialog.NumStrategies());
      nfg->SetTitle("Untitled Normal Form Game");
      for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
	nfg->Players()[pl]->SetName(gText("Player") + ToText(pl));
      }
      if (dialog.CreateOutcomes()) {
	NFSupport support(*nfg);
	NfgContIter iter(support);
	iter.First();
	do {
	  gbtNfgOutcome outcome = nfg->NewOutcome();
	  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
	    nfg->SetPayoff(outcome, pl, 0);
	    nfg->SetLabel(outcome, outcome.GetLabel() +
			  ToText(iter.Profile()[pl]->Number()));
	  }
	  nfg->SetOutcome(iter.Profile(), outcome);
	} while (iter.NextContingency());
      }
      NfgShow *nfgShow = new NfgShow(*nfg, 0);
      nfgShow->SetFilename("");
      AddGame(nfg, nfgShow);
    }
  }
}

void GambitApp::OnFileOpen(wxWindow *p_parent)
{
  wxFileDialog dialog(p_parent, "Choose file", "", "", "*.?fg");

  if (dialog.ShowModal() == wxID_OK) {
    SetCurrentDir(wxPathOnly(dialog.GetPath()));
    LoadFile(dialog.GetPath());
  }
}

void GambitApp::OnFileMRUFile(wxCommandEvent &p_event)
{
  LoadFile(m_fileHistory.GetHistoryFile(p_event.GetId() - wxID_FILE1));
}

void GambitApp::OnHelpContents(void)
{
}

void GambitApp::OnHelpIndex(void)
{
}

void GambitApp::OnHelpAbout(wxWindow *p_parent)
{
  dialogAbout dialog(p_parent, "About Gambit...",
		     "Gambit Graphical User Interface",
		     "Version " VERSION);
  dialog.ShowModal();
}

void GambitApp::LoadFile(const wxString &p_filename)
{    
  try {
    gFileInput infile(p_filename);
    Nfg *nfg = 0;

    ReadNfgFile(infile, nfg);

    if (nfg) {
      m_fileHistory.AddFileToHistory(p_filename);
      NfgShow *nfgShow = new NfgShow(*nfg, 0);
      nfgShow->SetFilename(p_filename);
      AddGame(nfg, nfgShow);
      SetFilename(nfgShow, p_filename);
      return;
    }
  }
  catch (gFileInput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open '%s' for reading",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;
  }

  try {
    gFileInput infile(p_filename);
    efgGame *efg = ReadEfgFile(infile);
                
    if (!efg) {
      wxMessageBox(wxString::Format("'%s' is not in a recognized format.",
				    p_filename.c_str()),
		   "Error", wxOK, 0);
      return;
    }

    m_fileHistory.AddFileToHistory(p_filename);
    EfgShow *efgShow = new EfgShow(*efg, 0);
    efgShow->SetFilename(p_filename);
    AddGame(efg, efgShow);
    SetFilename(efgShow, p_filename);
  }
  catch (gFileInput::OpenFailed &) { 
    wxMessageBox(wxString::Format("Could not open '%s' for reading",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;
  }
}


IMPLEMENT_APP(GambitApp)

void GambitApp::AddGame(efgGame *p_efg, EfgShow *p_efgShow)
{
  Game *game = new Game(p_efg);
  game->m_efgShow = p_efgShow;
  m_gameList.Append(game);
  m_fileHistory.UseMenu(p_efgShow->GetMenuBar()->GetMenu(0));
  m_fileHistory.AddFilesToMenu(p_efgShow->GetMenuBar()->GetMenu(0));
}

void GambitApp::AddGame(Nfg *p_nfg, NfgShow *p_nfgShow)
{
  Game *game = new Game(p_nfg);
  game->m_nfgShow = p_nfgShow;
  m_gameList.Append(game);
  m_fileHistory.UseMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
  m_fileHistory.AddFilesToMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
}

void GambitApp::AddGame(efgGame *p_efg, Nfg *p_nfg, NfgShow *p_nfgShow)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efg == p_efg) {
      m_gameList[i]->m_nfg = p_nfg;
      m_gameList[i]->m_nfgShow = p_nfgShow;
      break;
    }
  }
  m_fileHistory.UseMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
  m_fileHistory.AddFilesToMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
}

void GambitApp::RemoveGame(efgGame *p_efg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efg == p_efg) {
      m_fileHistory.RemoveMenu(m_gameList[i]->m_efgShow->GetMenuBar()->GetMenu(0));
      if (m_gameList[i]->m_nfg) {
	m_fileHistory.RemoveMenu(m_gameList[i]->m_nfgShow->GetMenuBar()->GetMenu(0));
	m_gameList[i]->m_nfgShow->Close();
	delete m_gameList[i]->m_nfg;
      }
      delete m_gameList.Remove(i);
      delete p_efg;
      break;
    }
  }
}

void GambitApp::RemoveGame(Nfg *p_nfg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfg == p_nfg) {
      m_fileHistory.RemoveMenu(m_gameList[i]->m_nfgShow->GetMenuBar()->GetMenu(0));
      if (m_gameList[i]->m_efg == 0) {
	delete m_gameList.Remove(i);
      }
      else {
	m_gameList[i]->m_nfg = 0;
	m_gameList[i]->m_nfgShow = 0;
      }
      delete p_nfg;
    }
  }
}

EfgShow *GambitApp::GetWindow(const efgGame *p_efg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efg == p_efg) {
      return m_gameList[i]->m_efgShow;
    }
  }
  return 0;
}

NfgShow *GambitApp::GetWindow(const Nfg *p_nfg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfg == p_nfg) {
      return m_gameList[i]->m_nfgShow;
    }
  }
  return 0;
}

void GambitApp::SetFilename(EfgShow *p_efgShow, const wxString &p_file)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efgShow == p_efgShow) {
      m_gameList[i]->m_fileName = p_file;
      return;
    }
  }
}

void GambitApp::SetFilename(NfgShow *p_nfgShow, const wxString &p_file)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfgShow == p_nfgShow) {
      m_gameList[i]->m_fileName = p_file;
      return;
    }
  }
}


//
// A general-purpose dialog box to display the description of the exception
//
void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
            long p_style /*= wxOK | wxCENTRE*/)
{
  gText message = "An internal error occurred in Gambit:\n" + p_message;
  wxMessageBox((char *) message, "Gambit Error", p_style, p_parent);
}

#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<Game *>;
template class gBlock<Game *>;
