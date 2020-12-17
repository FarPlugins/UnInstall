#include "guid.hpp"

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

const TCHAR* GetMsg(int MsgId)
{
#ifdef FARAPI3
	return(Info.GetMsg(&MainGuid,MsgId));
#else
	return(Info.GetMsg(Info.ModuleNumber,MsgId));
#endif
}

void ShowHelp(const TCHAR * HelpTopic)
{
	Info.ShowHelp(Info.ModuleName,HelpTopic,0);
}

int EMessage(const TCHAR * const * s, int nType, int n)
{
#ifdef FARAPI3
	return Info.Message(&MainGuid, &AnyMessage, FMSG_ALLINONE|nType, NULL, s,
	                    0, //���� �������� ��� FMSG_ALLINONE ������������
	                    n); //���������� ������
#else
	return Info.Message(Info.ModuleNumber, FMSG_ALLINONE|nType, NULL, s,
	                  0, //���� �������� ��� FMSG_ALLINONE ������������
	                  n); //���������� ������
#endif
}

int DrawMessage(int nType, int n, char *msg, ...)
{
	int total = 0;
	TCHAR * string = NULL;
	va_list ap;
	TCHAR * arg;
	va_start(ap, msg);

	while((arg = va_arg(ap,TCHAR*))!= 0)
	{
		total += lstrlen(arg) + 1; //�� ��� ����� ���������� ������ �������� ������
	}

	va_end(ap);
	total--; //��������� ���� �������� ������ �� ������
	string = (TCHAR *) realloc(string, sizeof(TCHAR)*(total + 1));
	string[0]=_T('\0');
	va_start(ap, msg);

	while((arg = va_arg(ap,TCHAR*))!= NULL)
	{
		StringCchCat(string, total+1, arg);
		StringCchCat(string, total+1, _T("\n"));
	}

	va_end(ap);
	string[total]=_T('\0');
	int result = EMessage((const TCHAR * const *) string, nType, n);
	realloc(string, 0);
	return result;
}

const TCHAR * strstri(const TCHAR *s, const TCHAR *c)
{
	if(c)
	{
		int l = lstrlen(c);

		for(const TCHAR *p = s ; *p ; p++)
			if(FSF.LStrnicmp(p, c, l) == 0)
				return p;
	}

	return NULL;
}

TCHAR * strnstri(TCHAR *s, const TCHAR *c, int n)
{
	if(c)
	{
		int l = min(lstrlen(c), n);

		for(TCHAR *p = s ; *p ; p++)
			if(FSF.LStrnicmp(p, c, l) == 0)
				return p;
	}

	return NULL;
}

TCHAR * unQuote(TCHAR *vStr)
{
	unsigned l;
	l = lstrlen(vStr);

	if(*vStr == _T('\"'))
		memmove(vStr,vStr+1,l*sizeof(TCHAR));

	l = lstrlen(vStr);

	if(vStr[l-1] == _T('\"'))
		vStr[l-1] = _T('\0');

	return(vStr);
}
