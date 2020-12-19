#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#include <tchar.h>
#include <strsafe.h>
#include "plugin.hpp"
#include "memory.h"
#ifndef nullptr
#define nullptr NULL
#endif
#ifndef _ASSERTE
#define _ASSERTE(x)
#endif
#include "DlgBuilder.hpp"
#include "eplugin.cpp"
#include "farcolor.hpp"
#include "farkeys.hpp"
#include "farlang.h"
#include "uninstall.hpp"

#  define SetStartupInfo SetStartupInfoW
#  define GetPluginInfo GetPluginInfoW
#  define Configure ConfigureW

int WINAPI GetMinFarVersionW(void)
{
  #define MAKEFARVERSION_OLD(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
  
  return MAKEFARVERSION_OLD(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;

	Info->Version = 	MAKEFARVERSION(1,10,17,0,VS_RELEASE);
	Info->Guid = MainGuid;
	Info->Title = L"UnInstall";
	Info->Description = L"UnInstall";
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}

void WINAPI SetStartupInfo(const struct PluginStartupInfo *psInfo)
{
	Info = *psInfo;
	FSF = *psInfo->FSF;
	Info.FSF = &FSF;
	ReadRegistry();
}

void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
	static const TCHAR *PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MPlugIn);
	Info -> StructSize = sizeof(*Info);

	if(Opt.WhereWork & 2)
		Info -> Flags |= PF_EDITOR;

	if(Opt.WhereWork & 1)
		Info -> Flags |= PF_VIEWER;

	Info ->PluginMenu.Strings = PluginMenuStrings;
	Info ->PluginMenu.Count = ARRAYSIZE(PluginMenuStrings);
	Info ->PluginMenu.Guids = &PluginMenuGuid;
	Info ->PluginConfig.Strings = PluginMenuStrings;
	Info ->PluginConfig.Count = ARRAYSIZE(PluginMenuStrings);
	Info ->PluginConfig.Guids = &PluginConfigGuid;
}

void ResizeDialog(HANDLE hDlg)
{
	CONSOLE_SCREEN_BUFFER_INFO con_info;
	GetConsoleScreenBufferInfo(hStdout, &con_info);
	unsigned con_sx = con_info.srWindow.Right - con_info.srWindow.Left + 1;
	int max_items = con_info.srWindow.Bottom - con_info.srWindow.Top + 1 - 7;
	int s = ((ListSize>0) && (ListSize<max_items) ? ListSize : (ListSize>0 ? max_items : 0));
	SMALL_RECT NewPos = { 2, 1, con_sx - 7, s + 2 };
	SMALL_RECT OldPos;

	Info.SendDlgMessage(hDlg,DM_GETITEMPOSITION,LIST_BOX,&OldPos);

	if(NewPos.Right!=OldPos.Right || NewPos.Bottom!=OldPos.Bottom)
	{
		COORD coord;
		coord.X = con_sx - 4;
		coord.Y = s + 4;
		Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,&coord);

		coord.X = -1;
		coord.Y = -1;

		Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,&coord);
		Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,LIST_BOX,&NewPos);
	}
}

static INT_PTR WINAPI DlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	static TCHAR Filter[MAX_PATH];
	static TCHAR spFilter[MAX_PATH];
	static FarListTitles ListTitle;

	INPUT_RECORD* record=nullptr;

	switch(Msg)
	{
		case DN_RESIZECONSOLE:
		{
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			ResizeDialog(hDlg);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
		}
		return TRUE;
		case DMU_UPDATE:
		{
			int OldPos = static_cast<int>(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,LIST_BOX,0));

			if(Param1)
				UpDateInfo();

			ListSize = 0;
			int NewPos = -1;

			if(OldPos >= 0 && OldPos < nCount)
			{
				if(!*Filter || strstri(p[OldPos].Keys[DisplayName],Filter))  //без учета регистра в OEM кодировке
					NewPos = OldPos;
			}

			for(int i = 0; i < nCount; i++)
			{
				const TCHAR* DispName = p[i].Keys[DisplayName], *Find;

				if(*Filter)
					Find = strstri(DispName,Filter);
				else
					Find = DispName;

				if(Find != nullptr)  //без учета регистра в OEM кодировке
				{
					FLI[i].Flags &= ~LIF_HIDDEN;

					if(Param2 && (i == OldPos))
					{
						if(FLI[i].Flags & LIF_CHECKED)
						{
							FLI[i].Flags &= ~LIF_CHECKED;
						}
						else
						{
							FLI[i].Flags |= LIF_CHECKED;
						}
					}

                    //без учета регистра - а кодировка ANSI
					if(NewPos == -1 && Find == DispName)
						NewPos = i;

					ListSize++;
				}
				else
					FLI[i].Flags |= LIF_HIDDEN;
			}

			if(Param1 == 0 && Param2)
			{
                // Снятие или установка пометки (Ins)
				if(*(int*)Param2 == 1)
				{
					for(int i = (OldPos+1); i < nCount; i++)
					{
						if(!(FLI[i].Flags & LIF_HIDDEN))
						{
							OldPos = i; break;
						}
					}

					NewPos = OldPos;
				}
				// Снятие или установка пометки (RClick)

				else if(*(int*)Param2  == 2)
				{
					NewPos = OldPos;
				}
			}
			else if(NewPos == -1)
			{
				NewPos = OldPos;
			}

			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			Info.SendDlgMessage(hDlg,DM_LISTSET,LIST_BOX,&FL);

			StringCchPrintf(spFilter,ARRAYSIZE(spFilter), GetMsg(MFilter),Filter,ListSize,nCount);
			ListTitle.Title = spFilter;
			ListTitle.TitleSize = lstrlen(spFilter);
			ListTitle.StructSize = sizeof(FarListTitles);
			Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,LIST_BOX,&ListTitle);

			ResizeDialog(hDlg);
			struct FarListPos FLP;
			FLP.SelectPos = NewPos;
			FLP.TopPos = -1;
			FLP.StructSize = sizeof(FarListPos);
			Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,LIST_BOX,&FLP);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
		}
		break;
		case DN_INITDIALOG:
		{
			StringCchCopy(Filter,ARRAYSIZE(Filter),_T(""));
			ListTitle.Bottom = const_cast<TCHAR*>(GetMsg(MBottomLine));
			ListTitle.BottomSize = lstrlen(GetMsg(MBottomLine));

            //подстраиваемся под размеры консоли
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
			ResizeDialog(hDlg);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
            //заполняем диалог
			Info.SendDlgMessage(hDlg,DMU_UPDATE,1,0);
		}
		break;
		case DN_CONTROLINPUT:
			{
				record=(INPUT_RECORD *)Param2;
				if (record->EventType == MOUSE_EVENT) {
					if (Param1 == LIST_BOX)
					{
						MOUSE_EVENT_RECORD *mer = &record->Event.MouseEvent;

					if(mer->dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						// find list on-screen coords (excluding frame and border)
						SMALL_RECT list_rect;
						Info.SendDlgMessage(hDlg, DM_GETDLGRECT, 0, &list_rect);
						list_rect.Left += 2;
						list_rect.Top += 1;
						list_rect.Right -= 2;
						list_rect.Bottom -= 1;

						if((mer->dwEventFlags == 0) && (mer->dwMousePosition.X > list_rect.Left) && (mer->dwMousePosition.X < list_rect.Right) && (mer->dwMousePosition.Y > list_rect.Top) && (mer->dwMousePosition.Y < list_rect.Bottom))
						{
							INPUT_RECORD irecord;
							irecord.EventType=KEY_EVENT;
							KEY_EVENT_RECORD key = { false,0,KEY_ENTER,0,0 };
							irecord.Event.KeyEvent = key;
							DlgProc(hDlg, DN_CONTROLINPUT, LIST_BOX, &irecord);

							return TRUE;
						}

						// pass message to scrollbar if needed
						if((mer->dwMousePosition.X == list_rect.Right) && (mer->dwMousePosition.Y > list_rect.Top) && (mer->dwMousePosition.Y < list_rect.Bottom)) return FALSE;

						return TRUE;
					}
					else if(mer->dwButtonState == RIGHTMOST_BUTTON_PRESSED)
					{
						int k=2;
						Info.SendDlgMessage(hDlg,DMU_UPDATE,0,&k);
						return TRUE;
					}
					return false;
				}
			}
		else if (record->EventType == KEY_EVENT) {
			switch(record->Event.KeyEvent.wVirtualKeyCode)
			{
			case VK_F8:
				{
					if(ListSize)
					{
						TCHAR DlgText[MAX_PATH + 200];
						StringCchPrintf(DlgText, ARRAYSIZE(DlgText), GetMsg(MConfirm), p[Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,LIST_BOX,NULL)].Keys[DisplayName]);

						if(EMessage((const TCHAR * const *) DlgText, 0, 2) == 0)
						{
							if(!DeleteEntry(static_cast<int>(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,LIST_BOX,NULL))))
								DrawMessage(FMSG_WARNING, 1, "%s",GetMsg(MPlugIn),GetMsg(MDelRegErr),GetMsg(MBtnOk),NULL);

							Info.SendDlgMessage(hDlg,DMU_UPDATE,1,0);
						}
					}
				}
				return TRUE;
			case VK_F9:
				{
					Configure(0);
				}
				return TRUE;
			case VK_RETURN:
				{
					if(ListSize)
					{
						int liChanged = 0;
						int liSelected = 0, liFirst = -1;

						for(int i = 0; i < nCount; i++)
						{
							if(FLI[i].Flags & LIF_CHECKED)
							{
								if(liFirst == -1)
									liFirst = i;

								liSelected++;
							}
						}

						if(liSelected <= 1)
						{
							int liAction;
							if ((record->Event.KeyEvent.wVirtualKeyCode  == VK_RETURN) && (record->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)) {
									liAction = Opt.ShiftEnterAction;
							}else{
								liAction = Opt.EnterAction;
							}
							int pos = (liFirst == -1) ? static_cast<int>(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,LIST_BOX,NULL)) : liFirst;
							liChanged = ExecuteEntry(pos, liAction, (Opt.RunLowPriority!=0));
						}
						else
						{
							int liAction;
							if ((record->Event.KeyEvent.wVirtualKeyCode  == VK_RETURN) && (record->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)) {
									liAction = Opt.ShiftEnterAction;
							}else{
								liAction = Opt.EnterAction;
							}

							bool LowPriority = (Opt.RunLowPriority!=0);

                            // Обязательно ожидание - два инсталлятора сразу недопускаются
							if(liAction == Action_Menu)
							{
								if(EntryMenu(0, liAction, LowPriority, liSelected) < 0)
									return TRUE;
							}
							else if(liAction == Action_Uninstall)
								liAction = Action_UninstallWait;
							else if(liAction == Action_Modify)
								liAction = Action_ModifyWait;
							else if(liAction == Action_Repair)
								liAction = Action_RepairWait;

							for(int pos = 0; pos < nCount; pos++)
							{
								if(!(FLI[pos].Flags & LIF_CHECKED))
									continue;

								struct FarListPos FLP;
								FLP.SelectPos = pos;
								FLP.TopPos = -1;
								FLP.StructSize = sizeof(FarListPos);
								Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,LIST_BOX,&FLP);

								int li = ExecuteEntry(pos, liAction, LowPriority);

								if(li == -1)
									break; // отмена

								if(li == 1)
									liChanged = 1;
							}
						}

						if(liChanged == 1)
						{
							Info.SendDlgMessage(hDlg,DMU_UPDATE,1,0);
						}
					}
				}
				return TRUE;
			case VK_DELETE:
				{
					if(lstrlen(Filter) > 0)
					{
						StringCchCopy(Filter,ARRAYSIZE(Filter),_T(""));
						Info.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
					}
				}
				return TRUE;
			case VK_F3:
			case VK_F4:
				{
					if(ListSize)
					{
						DisplayEntry(static_cast<int>(Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,LIST_BOX,NULL)));
						Info.SendDlgMessage(hDlg,DM_REDRAW,NULL,NULL);
					}
				}
				return TRUE;
			case VK_F2:
				{
					Opt.SortByDate = !Opt.SortByDate;
					Info.SendDlgMessage(hDlg,DMU_UPDATE,1,0);
				}
				return TRUE;
			case VK_BACK:
				{
					if(lstrlen(Filter))
					{
						Filter[lstrlen(Filter)-1] = '\0';
						Info.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
					}
				}
				return TRUE;
			default:
				{
					//case KEY_CTRLR:
					if ((record->Event.KeyEvent.wVirtualKeyCode=='R')&&((record->Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)||(record->Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED))) {
						Info.SendDlgMessage(hDlg,DMU_UPDATE,1,0);
						return TRUE;
					}
					// KEY_INS

					if((record->Event.KeyEvent.wVirtualKeyCode==VK_INSERT)&&(!(record->Event.KeyEvent.dwControlKeyState &(LEFT_ALT_PRESSED |LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED |RIGHT_CTRL_PRESSED |SHIFT_PRESSED ))))
					{
						int k=1;
						Info.SendDlgMessage(hDlg,DMU_UPDATE,0,&k);
						return TRUE;
					}

						//-- KEY_SHIFTINS  KEY_CTRLV

						if (
							((record->Event.KeyEvent.wVirtualKeyCode=='V')&&((record->Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)||(record->Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED)))
							||((record->Event.KeyEvent.wVirtualKeyCode==VK_INSERT)&&(record->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED))){


							static TCHAR bufData[MAX_PATH];
							size_t size = FSF.PasteFromClipboard(FCT_STREAM,nullptr,0);
							TCHAR *bufP = new TCHAR[size];
							FSF.PasteFromClipboard(FCT_STREAM,bufP,size);

							if(bufP)
							{
								StringCchCopy(bufData,ARRAYSIZE(bufData),bufP);
								delete[] bufP;

								unQuote(bufData);
								FSF.LStrlwr(bufData);

								for(int i = lstrlen(bufData); i >= 1; i--)
									for(int j = 0; j < nCount; j++)
										if(strnstri(p[j].Keys[DisplayName],bufData,i))
										{
											lstrcpyn(Filter,bufData,i+1);
											Info.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
											return TRUE;
										}
							}
							return TRUE;
						}
						//--default

						if(record->Event.KeyEvent.wVirtualKeyCode >= VK_SPACE && record->Event.KeyEvent.wVirtualKeyCode <= VK_DIVIDE && record->Event.KeyEvent.uChar.UnicodeChar!=0)
						{
							struct FarListInfo ListInfo;
							ListInfo.StructSize = sizeof(FarListInfo);
							Info.SendDlgMessage(hDlg,DM_LISTINFO,LIST_BOX,&ListInfo);

							if((lstrlen(Filter) < sizeof(Filter)) && ListInfo.ItemsNumber)
							{
								int filterL = lstrlen(Filter);

								Filter[filterL] = FSF.LLower(record->Event.KeyEvent.uChar.UnicodeChar);
								Filter[filterL+1] = '\0';
								Info.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
								return TRUE;
							}
						}
					}
				}
				}
			}

			return FALSE;
		case DN_CTLCOLORDIALOG:

			Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_MENUTEXT, Param2);
			return true;

		case DN_CTLCOLORDLGLIST:
			if(Param1 == LIST_BOX)
			{
				FarDialogItemColors *fdic=(FarDialogItemColors *)Param2;
				FarColor *Colors = fdic->Colors;

				int ColorIndex[] = { COL_MENUBOX, COL_MENUBOX, COL_MENUTITLE, COL_MENUTEXT, COL_MENUHIGHLIGHT, COL_MENUBOX, COL_MENUSELECTEDTEXT, COL_MENUSELECTEDHIGHLIGHT, COL_MENUSCROLLBAR, COL_MENUDISABLEDTEXT, COL_MENUARROWS, COL_MENUARROWSSELECTED, COL_MENUARROWSDISABLED };
				int Count = ARRAYSIZE(ColorIndex);
				if(Count > fdic->ColorsCount)
					Count = fdic->ColorsCount;

				for(int i = 0; i < Count; i++){
					FarColor fc;
					if (static_cast<BYTE>(Info.AdvControl(&MainGuid, ACTL_GETCOLOR, ColorIndex[i],&fc))) {
						Colors[i] = fc;
					}
				}
				return TRUE;
			}
			break;
	}

	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

HANDLE WINAPI OpenW(const struct OpenInfo *oInfo)
{
	ReadRegistry();
	struct FarDialogItem DialogItems[1];
	ZeroMemory(DialogItems, sizeof(DialogItems));
	p = NULL;
	FLI = NULL;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	UpDateInfo();
	DialogItems[0].Type = DI_LISTBOX;
	DialogItems[0].Flags = DIF_LISTNOAMPERSAND ;
	DialogItems[0].Flags |=DIF_LISTTRACKMOUSEINFOCUS;
	DialogItems[0].X1 = 2;
	DialogItems[0].Y1 = 1;

	HANDLE h_dlg = Info.DialogInit(&MainGuid,&ContentsGuid,-1,-1,0,0,L"Contents",DialogItems,ARRAYSIZE(DialogItems),0,0,DlgProc,0);

	if(h_dlg != INVALID_HANDLE_VALUE)
	{
		Info.DialogRun(h_dlg);
		Info.DialogFree(h_dlg);
	}


	FLI = (FarListItem *) realloc(FLI, 0);
	p = (KeyInfo *) realloc(p, 0);
	return NULL;
}

intptr_t WINAPI Configure(const struct ConfigureInfo *cInfo)
{
	PluginDialogBuilder Config(Info, MainGuid,ConfigGuid, MPlugIn, _T("Configuration"));
	FarDialogItem *p1, *p2;
	BOOL bShowInViewer = (Opt.WhereWork & 1) != 0;
	BOOL bShowInEditor = (Opt.WhereWork & 2) != 0;
	//BOOL bEnterWaitCompletion = (Opt.EnterFunction != 0);
	BOOL bUseElevation = (Opt.UseElevation != 0);
	BOOL bLowPriority = (Opt.RunLowPriority != 0);
	BOOL bForceMsiUse = (Opt.ForceMsiUse != 0);
	Config.AddCheckbox(MShowInEditor, &bShowInEditor);
	Config.AddCheckbox(MShowInViewer, &bShowInViewer);
	//Config.AddCheckbox(MEnterWaitCompletion, &bEnterWaitCompletion);
	Config.AddCheckbox(MUseElevation, &bUseElevation);
	Config.AddCheckbox(MLowPriority, &bUseElevation);
	Config.AddCheckbox(MForceMsiUse, &bForceMsiUse);
	Config.AddSeparator();
	FarList AEnter, AShiftEnter;
	AEnter.ItemsNumber = AShiftEnter.ItemsNumber = 7;
	AEnter.Items = (FarListItem*)calloc(AEnter.ItemsNumber,sizeof(FarListItem));
	AShiftEnter.Items = (FarListItem*)calloc(AEnter.ItemsNumber,sizeof(FarListItem));

	AEnter.StructSize = sizeof(FarList);
	AShiftEnter.StructSize = sizeof(FarList);
	for(size_t i = 0; i < AEnter.ItemsNumber; i++)
	{
		AEnter.Items[i].Text = GetMsg(MActionUninstallWait+i);
		AShiftEnter.Items[i].Text = AEnter.Items[i].Text;
	}

	p1 = Config.AddText(MEnterAction); p2 = Config.AddComboBox(23, &AEnter, &Opt.EnterAction);
	Config.MoveItemAfter(p1,p2);
	p1 = Config.AddText(MShiftEnterAction); p2 = Config.AddComboBox(23, &AShiftEnter, &Opt.ShiftEnterAction);
	Config.MoveItemAfter(p1,p2);
	Config.AddOKCancel(MBtnOk, MBtnCancel);

	if(Config.ShowDialog())
	{
		Opt.WhereWork = (bShowInViewer ? 1 : 0) | (bShowInEditor ? 2 : 0);
		//Opt.EnterFunction = bEnterWaitCompletion;
		Opt.UseElevation = bUseElevation;
		Opt.RunLowPriority = bLowPriority;
		Opt.ForceMsiUse = bForceMsiUse;
		PluginSettings settings(MainGuid,Info.SettingsControl);
		settings.Set(0,L"WhereWork",Opt.WhereWork);
		settings.Set(0,L"EnterAction",Opt.EnterAction);
		settings.Set(0,L"ShiftEnterAction",Opt.ShiftEnterAction);
		settings.Set(0,L"UseElevation",Opt.UseElevation);
		settings.Set(0,L"RunLowPriority",Opt.RunLowPriority);
		settings.Set(0,L"ForceMsiUse",Opt.ForceMsiUse);
	}

	return FALSE;
}
