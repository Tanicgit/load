/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.40                          *
*        Compiled Jun 22 2017, 10:13:26                              *
*        (c) 2017 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END

#include "DIALOG.h"
#include "ui.h"
#include "touch.h"
#include "comm.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0             (GUI_ID_USER + 0x01)
#define ID_TEXT_0             (GUI_ID_USER + 0x02)
#define ID_TEXT_1             (GUI_ID_USER + 0x03)
#define ID_TEXT_2             (GUI_ID_USER + 0x04)
#define ID_TEXT_3             (GUI_ID_USER + 0x05)
#define ID_EDIT_0             (GUI_ID_USER + 0x06)
#define ID_EDIT_1             (GUI_ID_USER + 0x07)
#define ID_EDIT_2             (GUI_ID_USER + 0x08)
#define ID_EDIT_3             (GUI_ID_USER + 0x09)
#define ID_BUTTON_0             (GUI_ID_USER + 0x0A)
#define ID_BUTTON_1             (GUI_ID_USER + 0x0B)
#define ID_TEXT_4             (GUI_ID_USER + 0x0C)
#define ID_BUTTON_2             (GUI_ID_USER + 0x0D)
#define ID_TEXT_5             (GUI_ID_USER + 0x0E)

// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)

extern char SSID[20];//="TP-LINK_2052";//20
extern char KEY[20];//="ysjoowxf0713";//20
extern char IP[20];//="192.168.0.100";//20
extern char port[10];//="3200";//10
extern uint8_t wifi_sta;
extern uint8_t server_sta;

// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 240, 320, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "SSID", ID_TEXT_0, 20, 20, 40, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "KEY", ID_TEXT_1, 20, 50, 40, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "IP", ID_TEXT_2, 20, 100, 40, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "PORT", ID_TEXT_3, 20, 130, 40, 20, 0, 0x0, 0 },
  { EDIT_CreateIndirect, "Edit", ID_EDIT_0, 75, 20, 120, 20, 0, 50, 0 },
  { EDIT_CreateIndirect, "Edit", ID_EDIT_1, 75, 50, 120, 20, 0, 50, 0 },
  { EDIT_CreateIndirect, "Edit", ID_EDIT_2, 75, 100, 120, 20, 0, 50, 0 },
  { EDIT_CreateIndirect, "Edit", ID_EDIT_3, 75, 130, 120, 20, 0, 50, 0 },
  { BUTTON_CreateIndirect, "SAVE", ID_BUTTON_0, 75, 180, 50, 20, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "DFSET", ID_BUTTON_1, 145, 180, 50, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "CONNECT", ID_TEXT_4, 90, 220, 80, 20, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "CONNECT", ID_TEXT_5, 90, 240, 80, 20, 0, 0x0, 0 },
	 { BUTTON_CreateIndirect, "TOUCH", ID_BUTTON_2, 5, 300, 50, 20, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
static WM_HWIN hEditItem[4];

static WM_HTIMER timer;
void dispUpdata(WM_MESSAGE * pMsg)
{
    hEditItem[0] = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
    EDIT_SetText(hEditItem[0], SSID);
    EDIT_SetTextAlign(hEditItem[0], GUI_TA_HCENTER | GUI_TA_VCENTER);
    //
    // Initialization of 'Edit'
    //
    hEditItem[1] = WM_GetDialogItem(pMsg->hWin, ID_EDIT_1);
    EDIT_SetTextAlign(hEditItem[1], GUI_TA_HCENTER | GUI_TA_VCENTER);
		EDIT_SetText(hEditItem[1], KEY);
    //
    // Initialization of 'Edit'
    //
    hEditItem[2] = WM_GetDialogItem(pMsg->hWin, ID_EDIT_2);
    EDIT_SetTextAlign(hEditItem[2], GUI_TA_HCENTER | GUI_TA_VCENTER);
    EDIT_SetText(hEditItem[2], IP);
    //
    // Initialization of 'Edit'
    //
    hEditItem[3] = WM_GetDialogItem(pMsg->hWin, ID_EDIT_3);
    EDIT_SetTextAlign(hEditItem[3], GUI_TA_HCENTER | GUI_TA_VCENTER);
    EDIT_SetText(hEditItem[3], port);
}
void wifi_sta_updata(WM_MESSAGE * pMsg)
{
		WM_HWIN hItem;
    
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
		if(wifi_sta&0x80)
		{
			TEXT_SetText(hItem,"wifi ok");
		}
		else if((wifi_sta&0x7f)<3)
		{
			TEXT_SetText(hItem,"wifi ing");
		}
		else
		{
			TEXT_SetText(hItem,"wifi err");
		}
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
		if(server_sta&0x80)
		{
			TEXT_SetText(hItem,"server ok");
		}
		else if((server_sta&0x7f)<3)
		{
			TEXT_SetText(hItem,"server ing");
		}
		else
		{
			TEXT_SetText(hItem,"server err");
		}
}
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
//  int     x,y;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Window'
    //
    hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0x008080FF));
	
	
		timer = WM_CreateTimer(pMsg->hWin,0,2000,0);
    //
    // Initialization of 'SSID'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    //
    // Initialization of 'KEY'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    //
    // Initialization of 'IP'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    //
    // Initialization of 'PORT'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    //
    // Initialization of 'Edit'
    //

    //
    // Initialization of 'CONNECT'
    //
		dispUpdata(pMsg);
		wifi_sta_updata(pMsg);
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
//    if(Id== hKeyBoardWin)
//    {
//
//    }

    switch(Id) {

    case ID_EDIT_0: // Notifications sent by 'Edit'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        showKeyBoard(240,320);
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_EDIT_1: // Notifications sent by 'Edit'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        showKeyBoard(240,320);
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_EDIT_2: // Notifications sent by 'Edit'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        showKeyBoard(240,320);
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_EDIT_3: // Notifications sent by 'Edit'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        showKeyBoard(240,320);
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_0: // Notifications sent by 'SET'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
			
				EDIT_GetText(hEditItem[0],SSID,20);
				EDIT_GetText(hEditItem[1],KEY,20);
				EDIT_GetText(hEditItem[2],IP,20);
				EDIT_GetText(hEditItem[3],port,10);
				SaveNetSet();
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'RESET'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)

        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
					SaveNetReset();
					dispUpdata(pMsg);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
			
      case ID_BUTTON_2: // Notifications sent by 'RESET'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        

        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
					hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_2);
					ClearAdjdata();
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    }
    break;
  // USER START (Optionally insert additional message handling)
	case WM_TIMER:
		switch(WM_GetTimerId(pMsg->Data.v))
		{
			case 0:
						wifi_sta_updata(pMsg);
						WM_RestartTimer(timer,3000);	
			break;
		}
	break;
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateWindow
*/
//WM_HWIN CreateMainWindow(void);
WM_HWIN CreateMainWindow(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/