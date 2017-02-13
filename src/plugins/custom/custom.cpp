#define _CRT_SECURE_NO_WARNINGS
#include "custom.h"

#pragma comment(lib, "../../../lib/detours/lib/detours.lib")

CNWRules *&g_pRules = *(CNWRules **) 0x0092DC64;
DWORD GetCreatureOffset = 0x004BE830;
__declspec( naked ) void* __cdecl GetCreature(int n){ __asm{ jmp dword ptr [GetCreatureOffset] }}
DWORD CNWRules__GetClassOffset = 0x0041DC30;
__declspec( naked ) CNWClass_s* __fastcall CNWRules__GetClass(void *rules, int ecx, unsigned __int8 a2){ __asm{ jmp dword ptr [CNWRules__GetClassOffset] } }
DWORD CGuiModalPanel__DeactivateOffset = 0x005E4650;
__declspec( naked ) void __fastcall CGuiModalPanel__Deactivate(void *p_this, int ecx, int a2){ __asm{ jmp dword ptr [CGuiModalPanel__DeactivateOffset] } }
DWORD CGuiModalPanel__ActivateOffset = 0x005E4620;
__declspec( naked ) void __fastcall CGuiModalPanel__Activate(void *p_this, int ecx){ __asm{ jmp dword ptr [CGuiModalPanel__ActivateOffset] } }
DWORD CCharacterSpellsPanel__CCharacterSpellsPanelOffset = 0x005AAEB0;
__declspec( naked ) void* __fastcall CCharacterSpellsPanel__CCharacterSpellsPanel(void *p_this,int ecx, int a2, int a3){ __asm{ jmp dword ptr [CCharacterSpellsPanel__CCharacterSpellsPanelOffset] } }

void InitPlugin();

FILE *logFile;
char logFileName[] = "logs/nwncx_custom.txt";

PLUGINLINK *pluginLink = 0;
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"NWNCX Custom",
	PLUGIN_MAKE_VERSION(2,0,0,0),
	"",
	"zelkin",
	"",
	"© 2013-2014 virusman & addicted2rpg",
	"http://www.nwnx.org/",
	0		//not transient
};

extern "C" __declspec(dllexport) PLUGININFO* GetPluginInfo(DWORD nwnxVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int InitPlugin(PLUGINLINK *link)
{
	pluginLink=link;
	InitPlugin();
	return 0;
}

void (__fastcall *CCharacterFeatsPanel__HandleOkButton)(void *pThis, int edx);
void __fastcall CCharacterFeatsPanel__HandleOkButton_Hook(void *pThis, int edx){
	
	int cre_id = *(int*)((int)pThis + 108);
	void *cre = GetCreature(cre_id);
	void *cre_stats = *(void **)((int)cre + 696);
	char cls_pos = *((char*)cre_stats + 50);
	char cls = *((char *)cre_stats + 256 * cls_pos + 473);
	CExoArrayList<unsigned short> feat_list = *(CExoArrayList<unsigned short>*) ((int)cre_stats + 184);

	int len = *(int*)((int)pThis + 708);

	for (int i = 0; i < len; i++){
		int feat = *(int*)(*(int*)((int)pThis + 1068) + 4 * i);
		if(!feat_list.Contains(feat)){
			feat_list.Add(feat);
			fprintf(logFile, "Feat Adicionada %d\n", feat);
		}
	}
	
	int chargen = *(int*)((int)pThis + 1764);

	CNWClass_s *c = CNWRules__GetClass(g_pRules, 0, cls);

	CGuiModalPanel__Deactivate(pThis, 0, 32782);

	void *spell_panel = nwncx_malloc(0xA24u);
	if(spell_panel){
		spell_panel = CCharacterSpellsPanel__CCharacterSpellsPanel(spell_panel, 0, cre_id, chargen);
		*(int*)((int)spell_panel + 32) = *(int*)((int)pThis + 32);
		*(int*)((int)spell_panel + 32) = *(int*)((int)pThis + 36);
		CGuiModalPanel__Activate(spell_panel, 0);
	}

	fprintf(logFile, "Teste %d\n", c->name_lower_tlk);
	fflush(logFile);

	CCharacterFeatsPanel__HandleOkButton(pThis, edx);
}

void HookFunctions(){
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	*(DWORD*)&CCharacterFeatsPanel__HandleOkButton = 0x005A6E00;

	if(DetourAttach(&(PVOID&)CCharacterFeatsPanel__HandleOkButton, CCharacterFeatsPanel__HandleOkButton_Hook) == 0){
		fprintf(logFile, "CCharacterFeatsPanel__HandleOkButton Hooked\n");
	}else
		fprintf(logFile, "CCharacterFeatsPanel__HandleOkButton Failed\n");


	fflush(logFile);
	DetourTransactionCommit();
}

void InitPlugin()
{
	logFile = fopen(logFileName, "w");
	fprintf(logFile, "NWCX Custom plugin 0.0.1\n");
	fprintf(logFile, "(c) 2017 by zelkin\n");
	fflush(logFile);

	HookFunctions();

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		fprintf(logFile, "* Plugin exit.  Cleaning up....\n");
		fflush(logFile);
	}
	return TRUE;
}