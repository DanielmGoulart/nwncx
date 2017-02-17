#define _CRT_SECURE_NO_WARNINGS
#include "custom.h"

#pragma comment(lib, "../../../lib/detours/lib/detours.lib")

//Globals
CNWRules *&g_pRules = *(CNWRules **) 0x0092DC64;

int g_caster_cls = 0;
int g_override_class = 0;
int g_total_level = 0;
int g_lvlup_clspos = 0;

//NWMain Calls
DWORD GetCreatureOffset = 0x004BE830;
__declspec( naked ) void* __cdecl GetCreature(int n){ __asm{ jmp dword ptr [GetCreatureOffset] }}
DWORD CNWRules__GetClassOffset = 0x0041DC30;
__declspec( naked ) CNWClass_s* __fastcall CNWRules__GetClass(void *rules, int edx, unsigned __int8 a2){ __asm{ jmp dword ptr [CNWRules__GetClassOffset] } }
DWORD CGuiModalPanel__DeactivateOffset = 0x005E4650;
__declspec( naked ) void __fastcall CGuiModalPanel__Deactivate(void *p_this, int edx, int a2){ __asm{ jmp dword ptr [CGuiModalPanel__DeactivateOffset] } }
DWORD CGuiModalPanel__ActivateOffset = 0x005E4620;
__declspec( naked ) void __fastcall CGuiModalPanel__Activate(void *p_this, int edx){ __asm{ jmp dword ptr [CGuiModalPanel__ActivateOffset] } }
DWORD CCharacterSpellsPanel__CCharacterSpellsPanelOffset = 0x005AAEB0;
__declspec( naked ) void* __fastcall CCharacterSpellsPanel__CCharacterSpellsPanel(void *p_this,int edx, int a2, int a3){ __asm{ jmp dword ptr [CCharacterSpellsPanel__CCharacterSpellsPanelOffset] } }
DWORD CNWCLevelUpStats__CalcNumberFeatsOffset = 0x004F2CF0;
__declspec( naked ) void* __fastcall CNWCLevelUpStats__CalcNumberFeats(void *p_this,int edx, char *a2, char *a3){__asm{ jmp dword ptr [CNWCLevelUpStats__CalcNumberFeatsOffset] }}

//Plugin
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

char (__fastcall *CNWCCreatureStats__GetClassLevel)(void *pThis, int edx, unsigned char a2);
char __fastcall CNWCCreatureStats__GetClassLevel_Hook(void *pThis, int edx, unsigned char a2){
	if(g_caster_cls){
		return g_total_level;
	}
	return CNWCCreatureStats__GetClassLevel(pThis, edx, a2);
}

char (__fastcall *CNWCCreatureStats__GetClass)(void *pThis, int edx, unsigned __int8 a2);
char __fastcall CNWCCreatureStats__GetClass_Hook(void *pThis, int edx, unsigned __int8 a2){

	if(g_caster_cls){
		return g_caster_cls;
	}

	return CNWCCreatureStats__GetClass(pThis, edx, a2);
}

void (__fastcall *CCharacterSpellsPanel__HandleOkButton)(void *pThis, int edx);
void __fastcall CCharacterSpellsPanel__HandleOkButton_Hook(void *pThis, int edx){

	int cre_id = *(int*)((int)pThis + 2408);
	void *cre = GetCreature(cre_id);
	
	if(g_caster_cls){
		g_caster_cls = 0;
		void *cre_stats = *(void **)((int)cre + 696);
		*((char*)cre_stats + 50) = g_lvlup_clspos;
		g_caster_cls = 0;
	}

	CCharacterSpellsPanel__HandleOkButton(pThis, edx);
}

void (__fastcall *CCharacterSpellsPanel__HandleCancelButton)(void *pThis, int edx);
void __fastcall CCharacterSpellsPanel__HandleCancelButton_Hook(void *pThis, int edx){

	int cre_id = *(int*)((int)pThis + 2408);
	void *cre = GetCreature(cre_id);
	
	if(g_caster_cls){
		g_caster_cls = 0;
		void *cre_stats = *(void **)((int)cre + 696);
		*((char*)cre_stats + 50) = g_lvlup_clspos;
		g_caster_cls = 0;
	}

	CCharacterSpellsPanel__HandleCancelButton(pThis, edx);
}

int (__fastcall *CCharacterSpellsPanel__HandleModalEscKey)(void *pThis, int edx);
int __fastcall CCharacterSpellsPanel__HandleModalEscKey_Hook(void *pThis, int edx){

	int cre_id = *(int*)((int)pThis + 2408);
	void *cre = GetCreature(cre_id);
	
	if(g_caster_cls){
		g_caster_cls = 0;
		void *cre_stats = *(void **)((int)cre + 696);
		*((char*)cre_stats + 50) = g_lvlup_clspos;
		g_caster_cls = 0;
	}

	return CCharacterSpellsPanel__HandleModalEscKey(pThis, edx);
}

void (__fastcall *CCharacterFeatsPanel__HandleOkButton)(void *pThis, int edx);
void __fastcall CCharacterFeatsPanel__HandleOkButton_Hook(void *pThis, int edx){
	
	int cre_id = *(int*)((int)pThis + 108);
	void *cre = GetCreature(cre_id);

	if(cre){
		void *cre_stats = *(void **)((int)cre + 696);
		char cls_pos = *((char*)cre_stats + 50);
		char cls = *((char *)cre_stats + 256 * cls_pos + 473);
		char cls_lvl = *((char *)cre_stats + 256 * cls_pos + 474);
		CNWClass_s *c = CNWRules__GetClass(g_pRules, 0, cls);
		char ArcSpelllvlMod = *((byte*)c + 612); 
		

		if(ArcSpelllvlMod){
			int caster_level;
			int gain_spell_level = ((cls_lvl + ArcSpelllvlMod - 1) % ArcSpelllvlMod);
			if(!gain_spell_level){
				g_caster_cls = 0;
				for (int i = 0; i < cls_pos; i++){
					char caster_cls = *((char *)cre_stats + 256 * i + 473);
					if(	caster_cls == CLASS_TYPE_BARD || caster_cls == CLASS_TYPE_SORCERER || caster_cls == CLASS_TYPE_WIZARD){
						g_lvlup_clspos = *((char*)cre_stats + 50);
						*((char*)cre_stats + 50) = i;
						g_caster_cls = caster_cls;
						caster_level = *((char *)cre_stats + 256 * i + 474);
						g_total_level = caster_level + ((cls_lvl + ArcSpelllvlMod - 1) / ArcSpelllvlMod);
						break;
					}
				}
			}
		}else if(cls == CLASS_TYPE_BARD || cls == CLASS_TYPE_SORCERER || cls == CLASS_TYPE_WIZARD){
				int cls_len = *((char*)cre_stats + 49);
				
				if(cls_pos == 2){
					//Só a classe caster "mais da esquerda"
					char caster_cls = *((char *)cre_stats + 473);
					if(caster_cls ==  CLASS_TYPE_BARD || caster_cls == CLASS_TYPE_SORCERER || caster_cls == CLASS_TYPE_WIZARD){
						goto exit;
					}
				}
				for(int i = cls_pos; i < cls_len; i++){
					char prestige_caster_cls = *((char *)cre_stats + 256 * i + 473);
					CNWClass_s *c = CNWRules__GetClass(g_pRules, 0, prestige_caster_cls);
					char ArcSpelllvlMod = *((byte*)c + 612); 
					if(ArcSpelllvlMod){
						int prestige_caster_cls_lvl = *((char *)cre_stats + 256 * i + 474);
						g_total_level = cls_lvl + ((prestige_caster_cls_lvl + ArcSpelllvlMod - 1) / ArcSpelllvlMod);
						g_caster_cls = cls;
						break;
					}
				}

			}
	}

exit:
	CCharacterFeatsPanel__HandleOkButton(pThis, edx);
}

void (__fastcall *CCharacterSkillsPanel__HandleOkButton)(void *pThis, int edx);
void __fastcall CCharacterSkillsPanel__HandleOkButton_Hook(void *pThis, int edx){
	int cre_id = *(int*)((int)pThis + 1512);
	void *cre = GetCreature(cre_id);

	if(cre){
		void *cre_stats = *(void **)((int)cre + 696);
		char cls_pos = *((char*)cre_stats + 50);
		char cls = *((char *)cre_stats + 256 * cls_pos + 473);
		char cls_lvl = *((char *)cre_stats + 256 * cls_pos + 474);
		

		char feat1 = 0;
		char feat2 = 0;
		CNWCLevelUpStats__CalcNumberFeats(cre_stats, 0, &feat1, &feat2);
		if(!feat1 && !feat2){
			CNWClass_s *c = CNWRules__GetClass(g_pRules, 0, cls);
			char ArcSpelllvlMod = *((byte*)c + 612); 

			if(ArcSpelllvlMod){
				int caster_level;
				int gain_spell_level = ((cls_lvl + ArcSpelllvlMod - 1) % ArcSpelllvlMod);
				if(!gain_spell_level){
					g_caster_cls = 0;
					for (int i = 0; i < cls_pos; i++){
						char caster_cls = *((char *)cre_stats + 256 * i + 473);
						if(	caster_cls == CLASS_TYPE_BARD || caster_cls == CLASS_TYPE_SORCERER || caster_cls == CLASS_TYPE_WIZARD){
							g_lvlup_clspos = *((char*)cre_stats + 50);
							*((char*)cre_stats + 50) = i;
							g_caster_cls = caster_cls;
							caster_level = *((char *)cre_stats + 256 * i + 474);
							g_total_level = caster_level + ((cls_lvl + ArcSpelllvlMod - 1) / ArcSpelllvlMod);
							break;
						}
					}
				}
			}else if(cls == CLASS_TYPE_BARD || cls == CLASS_TYPE_SORCERER || cls == CLASS_TYPE_WIZARD){
				int cls_len = *((char*)cre_stats + 49);
				
				if(cls_pos == 2){
					//Só a classe caster "mais da esquerda"
					char caster_cls = *((char *)cre_stats + 473);
					if(caster_cls ==  CLASS_TYPE_BARD || caster_cls == CLASS_TYPE_SORCERER || caster_cls == CLASS_TYPE_WIZARD){
						goto exit;
					}
				}
				for(int i = cls_pos; i < cls_len; i++){
					char prestige_caster_cls = *((char *)cre_stats + 256 * i + 473);
					CNWClass_s *c = CNWRules__GetClass(g_pRules, 0, prestige_caster_cls);
					char ArcSpelllvlMod = *((byte*)c + 612); 
					if(ArcSpelllvlMod){
						int prestige_caster_cls_lvl = *((char *)cre_stats + 256 * i + 474);
						g_total_level = cls_lvl + ((prestige_caster_cls_lvl + ArcSpelllvlMod - 1) / ArcSpelllvlMod);
						g_caster_cls = cls;
						break;
					}
				}

			}
		}
	}

exit:
	CCharacterSkillsPanel__HandleOkButton(pThis, edx);
}


void (__fastcall *CCharPageChar__HandleLevelUpButton)(void *pThis, int edx);
void __fastcall CCharPageChar__HandleLevelUpButton_Hook(void *pThis, int edx){
	fprintf(logFile, "Resetando no começo do level up \n");
	fflush(logFile);
	g_caster_cls = 0;
	CCharPageChar__HandleLevelUpButton(pThis, edx);
}

#define my_hook(addr, pfunc, hook)				\
	*(DWORD*)&pfunc = addr;						\
	if(DetourAttach(&(PVOID&)pfunc, hook) == 0)	\
		fprintf(logFile, #pfunc " Hooked\n");	\
	else										\
		fprintf(logFile, #pfunc " Failed\n")		\

void HookFunctions(){
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	my_hook(0x005A6E00, CCharacterFeatsPanel__HandleOkButton, CCharacterFeatsPanel__HandleOkButton_Hook);
	my_hook(0x004EE300, CNWCCreatureStats__GetClass, CNWCCreatureStats__GetClass_Hook);
	my_hook(0x004EE320, CNWCCreatureStats__GetClassLevel, CNWCCreatureStats__GetClassLevel_Hook);
	my_hook(0x005AC020, CCharacterSpellsPanel__HandleOkButton, CCharacterSpellsPanel__HandleOkButton_Hook);
	my_hook(0x00497E40, CCharPageChar__HandleLevelUpButton, CCharPageChar__HandleLevelUpButton_Hook);
	my_hook(0x005A9C30, CCharacterSkillsPanel__HandleOkButton, CCharacterSkillsPanel__HandleOkButton_Hook);
	my_hook(0x005ABFC0, CCharacterSpellsPanel__HandleCancelButton, CCharacterSpellsPanel__HandleCancelButton_Hook);
	my_hook(0x005AB2A0, CCharacterSpellsPanel__HandleModalEscKey, CCharacterSpellsPanel__HandleModalEscKey_Hook);

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