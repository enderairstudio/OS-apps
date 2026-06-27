#pragma once

#include <windows.h>

// Action Menu Identifiers
#define IDM_FILE_NEW         2001
#define IDM_FILE_OPEN        2002
#define IDM_FILE_SAVE        2003
#define IDM_FILE_SAVE_AS     2004
#define IDM_FILE_EXIT        2005

#define IDM_EDIT_UNDO        2011
#define IDM_EDIT_CUT         2012
#define IDM_EDIT_COPY        2013
#define IDM_EDIT_PASTE       2014
#define IDM_EDIT_DELETE      2015
#define IDM_EDIT_SELECT_ALL  2016

#define IDM_SEARCH_FIND      2021

#define IDM_VIEW_SIDEBAR     2031

#define IDM_HELP_ABOUT       2041

namespace PaperPad {

class MenuBar {
public:
    static HMENU create();
};

} // namespace PaperPad
