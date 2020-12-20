#include "guid.hpp"

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

const wchar_t* GetMsg(int MsgId)
{
	return(Info.GetMsg(&MainGuid, MsgId));
}

void ShowHelp(const wchar_t* HelpTopic)
{
	Info.ShowHelp(Info.ModuleName, HelpTopic, 0);
}

int EMessage(const wchar_t* const* s, int nType, int n)
{
	return Info.Message(&MainGuid, &AnyMessage, FMSG_ALLINONE | nType, NULL, s,
		0, //этот параметр при FMSG_ALLINONE игнорируется
		n); //количество кнопок
}

int DrawMessage(int nType, int n, char* msg, ...)
{
	int total = 0;
	wchar_t* string = NULL;
	va_list ap;
	wchar_t* arg;
	va_start(ap, msg);

	while ((arg = va_arg(ap, wchar_t*)) != 0)
	{
		total += lstrlen(arg) + 1; //мы еще будем записывать символ перевода строки
	}

	va_end(ap);
	total--; //последний знак перевода строки мы сотрем
	string = (wchar_t*)realloc(string, sizeof(wchar_t) * (total + 1));
	string[0] = L'\0';
	va_start(ap, msg);

	while ((arg = va_arg(ap, wchar_t*)) != NULL)
	{
		StringCchCat(string, total + 1, arg);
		StringCchCat(string, total + 1, L"\n");
	}

	va_end(ap);
	string[total] = L'\0';
	int result = EMessage((const wchar_t* const*)string, nType, n);
	realloc(string, 0);
	return result;
}

const wchar_t* strstri(const wchar_t* s, const wchar_t* c)
{
	if (c)
	{
		int l = lstrlen(c);

		for (const wchar_t* p = s; *p; p++)
			if (FSF.LStrnicmp(p, c, l) == 0)
				return p;
	}

	return NULL;
}

wchar_t* strnstri(wchar_t* s, const wchar_t* c, int n)
{
	if (c)
	{
		int l = min(lstrlen(c), n);

		for (wchar_t* p = s; *p; p++)
			if (FSF.LStrnicmp(p, c, l) == 0)
				return p;
	}

	return NULL;
}

wchar_t* unQuote(wchar_t* vStr)
{
	unsigned l;
	l = lstrlen(vStr);

	if (*vStr == L'\"')
		memmove(vStr, vStr + 1, l * sizeof(wchar_t));

	l = lstrlen(vStr);

	if (vStr[l - 1] == L'\"')
		vStr[l - 1] = L'\0';

	return(vStr);
}
