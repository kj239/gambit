//
// FILE: $Source$
// REVISION: $Revision$
// DATE: $Date$
//
// DESCRIPTION:
// Implementation of extensive form outcome palette window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgoutcome.h"

//=====================================================================
//            Grid cell editor for float/rational data
//=====================================================================

class NumberEditor : public wxGridCellTextEditor {
private:
  wxString m_valueOld;

protected:
  // string representation of m_valueOld
  wxString GetString() const;

public:
  NumberEditor(void);

  virtual void Create(wxWindow* parent, wxWindowID id,
		      wxEvtHandler* evtHandler);

  virtual bool IsAcceptedKey(wxKeyEvent& event);
  virtual void BeginEdit(int row, int col, wxGrid* grid);
  virtual bool EndEdit(int row, int col, wxGrid* grid);

  virtual void Reset();
  virtual void StartingKey(wxKeyEvent& event);

  virtual wxGridCellEditor *Clone() const
  { return new NumberEditor; }

  // parameters string format is "width,precision"
  virtual void SetParameters(const wxString& params);
};


NumberEditor::NumberEditor(void)
{ }

void NumberEditor::Create(wxWindow* parent,
			  wxWindowID id,
			  wxEvtHandler* evtHandler)
{
  wxGridCellTextEditor::Create(parent, id, evtHandler);
}

void NumberEditor::BeginEdit(int row, int col, wxGrid* grid)
{
  // first get the value
  wxGridTableBase *table = grid->GetTable();
  m_valueOld = table->GetValue(row, col);

  DoBeginEdit(GetString());
}

bool NumberEditor::EndEdit(int row, int col, wxGrid* grid)
{
  grid->GetTable()->SetValue(row, col, Text()->GetValue());
  return true;
}

void NumberEditor::Reset()
{
  DoReset(GetString());
}

void NumberEditor::StartingKey(wxKeyEvent& event)
{
  int keycode = (int)event.KeyCode();
  if ( isdigit(keycode) ||
       keycode == '+' || keycode == '-' || keycode == '.' ) {
    wxGridCellTextEditor::StartingKey(event);

    // skip Skip() below
    return;
  }

  event.Skip();
}

void NumberEditor::SetParameters(const wxString &/*params*/)
{ }

wxString NumberEditor::GetString(void) const
{
  return m_valueOld;
}

bool NumberEditor::IsAcceptedKey(wxKeyEvent& event)
{
  if (wxGridCellEditor::IsAcceptedKey(event)) {
    int keycode = event.GetKeyCode();
    switch ( keycode ) {
    case WXK_NUMPAD0:
    case WXK_NUMPAD1:
    case WXK_NUMPAD2:
    case WXK_NUMPAD3:
    case WXK_NUMPAD4:
    case WXK_NUMPAD5:
    case WXK_NUMPAD6:
    case WXK_NUMPAD7:
    case WXK_NUMPAD8:
    case WXK_NUMPAD9:
    case WXK_ADD:
    case WXK_NUMPAD_ADD:
    case WXK_SUBTRACT:
    case WXK_NUMPAD_SUBTRACT:
    case WXK_DECIMAL:
    case WXK_NUMPAD_DECIMAL:
      return true;

    default:
      if ((keycode < 128) &&
	  (isdigit(keycode) || tolower(keycode) == '/') )
	return true;
    }
  }

  return false;
}


BEGIN_EVENT_TABLE(EfgOutcomeWindow, wxGrid)
  EVT_GRID_CELL_CHANGE(EfgOutcomeWindow::OnCellChanged)
END_EVENT_TABLE()

EfgOutcomeWindow::EfgOutcomeWindow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow)
{
  Show(false);
  CreateGrid(p_efgShow->Game()->NumOutcomes(),
	     p_efgShow->Game()->NumPlayers() + 1);
  for (int row = 0; row < GetRows(); row++) {
    for (int col = 1; col < GetCols(); col++) {
      SetCellEditor(row, col, new NumberEditor);
    }
  }
      
  EnableEditing(true);

  SetLabelValue(wxHORIZONTAL, "Name", 0);
  for (int i = 1; i <= p_efgShow->Game()->NumPlayers(); i++) {
    SetLabelValue(wxHORIZONTAL, 
		  (char *) p_efgShow->Game()->Players()[i]->GetName(), i);
  }
  
  SetLabelSize(wxVERTICAL, 0);
  SetDefaultCellAlignment(wxCENTER, wxCENTER);
  AdjustScrollbars();
}

void EfgOutcomeWindow::UpdateValues(void)
{
  const FullEfg &efg = *m_parent->Game();

  if (GetRows() > 0 && GetRows() != efg.NumOutcomes()) {
    DeleteRows(0, GetRows());
    AppendRows(efg.NumOutcomes());

    for (int row = 0; row < GetRows(); row++) {
      for (int col = 1; col < GetCols(); col++) {
	SetCellEditor(row, col, new NumberEditor);
      }
    }
  }

  for (int outc = 1; outc <= efg.NumOutcomes(); outc++) {
    Efg::Outcome outcome = efg.GetOutcome(outc);

    SetCellValue((char *) efg.GetOutcomeName(outcome), outc - 1, 0);

    for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
      SetCellValue((char *) ToText(efg.Payoff(outcome, efg.Players()[pl])),
		   outc - 1, pl);
    }
  }
}

void EfgOutcomeWindow::OnCellChanged(wxGridEvent &p_event)
{
  int row = p_event.GetRow();
  int col = p_event.GetCol();

  if (col == 0) { 
    // Edited cell label
    m_parent->Game()->SetOutcomeName(m_parent->Game()->GetOutcome(row + 1),
				     GetCellValue(row, col).c_str());
  }
  else {
    // Edited payoff
    m_parent->Game()->SetPayoff(m_parent->Game()->GetOutcome(row + 1), col,
				ToNumber(GetCellValue(row, col).c_str()));
  }

  m_parent->OnOutcomesEdited();
}

