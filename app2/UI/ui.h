#ifndef __UI_H
#define __UI_H

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"
#include "ICONVIEW.h"
#include "TREEVIEW.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"

WM_HWIN CreateMainWindow(void);
WM_HWIN CreateKeyWindow(void);

//int showKeyBoard(void(*cb)(char *,uint8_t),int sizex,int sizey);
int showKeyBoard(int sizex,int sizey);

void TP_Adjust(void);
#endif 
