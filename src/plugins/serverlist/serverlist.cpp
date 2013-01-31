#include "NWNMSClient.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <pluginapi.h>
#include "detours.h"
//#include <m_common.h>
#include <nwnapi\nwnapi.h>
#include <nwnapi\CNWMessage.h>
#include <nwnapi\CTlkTable.h>
#include <nwnapi\CExoResMan.h>
#include <nwnapi\CNWRules.h>
#include <nwnapi\CAppManager.h>
#include <nwnapi\custom\nwn_globals.h>
#include <nwnapi\custom\nwn_globals.cpp>

#define DATA_DOING_NOTHING 0
#define DATA_OLD 1
#define DATA_FETCHING 2
#define DATA_READY 3

// The compiler optimizer will remove these when set to 0, so don't worry about all inefficient lines that call them.
// LOG_THREADS will log thread behavior - very spammy but useful.
// USE_REDUNDANT_LOCKS are data-access locks of data that is used mutually by both threads, but the normal
// threads in place should prevent them from being accessed at once.  If turning these on and then off 
// makes a difference in the way things behave (it shouldn't), then that will alert you to a bug.  This can be 
// helpful to finding "the ghost in the machine" that accompanies multithreading issues -- addicted.  
#define LOG_THREADS 0
#define USE_REDUNDANT_LOCKS 0

FILE *logFile;
char logFileName[] = "logs/nwncx_serverlist.txt";
NWNMSClient *client;

int joingroup_called = 0;

HANDLE threadlock_JoinGroup;
HANDLE threadlock_FetchServers;
HANDLE threadlock_dataStatus;
HANDLE threadlock_clientClass;

HANDLE thandle;
int data_status = DATA_DOING_NOTHING; 
int roomGlobal = -1;

void InitPlugin();
void CreateMutexes();
void InitThread();


//////////////////////////////////////////////////////////////////////////
PLUGINLINK *pluginLink = 0;
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"NWNCX Serverlist",
	PLUGIN_MAKE_VERSION(1,0,0,0),
	"",
	"virusman & addicted2rpg",
	"virusman@virusman.ru & duckbreath@yahoo.com",
	"� 2013 virusman & addicted2rpg",
	"http://www.virusman.ru/",
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





void FetchServers(int nRoom) {

	if(LOG_THREADS) {
		fprintf(logFile, "Child thread acquiring JoinGroup....");
		fflush(logFile);
	}

	WaitForSingleObject(threadlock_JoinGroup, INFINITE);

	if(LOG_THREADS) {
		fprintf(logFile, "ACQUIRED (C)\n");
		fprintf(logFile, "Child thread acquiring FetchServers....");
		fflush(logFile);
	}

	WaitForSingleObject(threadlock_FetchServers, INFINITE);

	if(LOG_THREADS) {
		fprintf(logFile, "ACQUIRED (C)\n");
		fflush(logFile);
	}

	// This, thankfully, never happens.  If somehow it does, we really need to know about it.
	if(USE_REDUNDANT_LOCKS)	WaitForSingleObject(threadlock_dataStatus, INFINITE);
	if(data_status == DATA_READY) { 
		fprintf(logFile, "* Serious mutex error in FetchServers().  Report in a post at nwnx.org\n");
		fflush(logFile);
		ExitThread(0);
		return;
	}

	data_status = DATA_FETCHING;
	if(USE_REDUNDANT_LOCKS) ReleaseMutex(threadlock_dataStatus);

	fprintf(logFile, "Class initialization & pulling rooms, but not adding them to the NWN client yet.\n");
	fflush(logFile);
	client = new NWNMSClient(logFile, g_pAppManager);

	if(USE_REDUNDANT_LOCKS) WaitForSingleObject(threadlock_clientClass, INFINITE);
	client->GetServersInRoom(nRoom);  
	if(USE_REDUNDANT_LOCKS) ReleaseMutex(threadlock_clientClass);


	if(USE_REDUNDANT_LOCKS) WaitForSingleObject(threadlock_dataStatus, INFINITE);
	data_status = DATA_READY;
	if(USE_REDUNDANT_LOCKS) ReleaseMutex(threadlock_dataStatus);

	fprintf(logFile, "Rooms pulled, data is ready.\n");
	fflush(logFile);

	if(LOG_THREADS) {
		fprintf(logFile, "Child thread releasing FetchServers....");
		fflush(logFile);
	}

	ReleaseMutex(threadlock_FetchServers);
	if(LOG_THREADS) {
		fprintf(logFile, "RELEASED (C)\n");
		fflush(logFile);
	}

	if(LOG_THREADS) {
		fprintf(logFile, "Child thread releasing JoinGroup....");
		fflush(logFile);
	}
	ReleaseMutex(threadlock_JoinGroup);
	if(LOG_THREADS) {
		fprintf(logFile, "RELEASED (C)\n");
		fflush(logFile);
	}


	// Child thread dies here.
}

void Poll() {

	if(USE_REDUNDANT_LOCKS) WaitForSingleObject(threadlock_dataStatus, INFINITE);
	if(data_status == DATA_READY) {
		fprintf(logFile, "Adding servers to Neverwinter Nights client\n");
		fflush(logFile);
		if(USE_REDUNDANT_LOCKS) WaitForSingleObject(threadlock_clientClass, INFINITE);
		client->AddServers();
		delete client;
		if(USE_REDUNDANT_LOCKS) ReleaseMutex(threadlock_clientClass);
		data_status = DATA_OLD;
		fprintf(logFile, "Done!\n\n\n");
		fflush(logFile);
	}
	if(USE_REDUNDANT_LOCKS) ReleaseMutex(threadlock_dataStatus);

}


int (__fastcall *CGameSpyClient__JoinGroupRoom)(void *pGameSpy, int edx, int nRoom);
int __fastcall CGameSpyClient__JoinGroupRoom_Hook(void *pGameSpy, int edx, int nRoom)
{		


	if(LOG_THREADS) {
		fprintf(logFile, "Main thread acquiring JoinGroup....");
		fflush(logFile);
	}
	WaitForSingleObject(threadlock_JoinGroup, INFINITE);
	if(LOG_THREADS) {
		fprintf(logFile, "ACQUIRED (M)\n");
		fflush(logFile);
	}


	roomGlobal = nRoom;


	if(!joingroup_called) {
		joingroup_called = 1;
	}

	InitThread();
	
	ResumeThread(thandle);
	if(LOG_THREADS) {
		fprintf(logFile, "Main thread releasing JoinGroup....");
		fflush(logFile);
	}
	ReleaseMutex(threadlock_JoinGroup);
	if(LOG_THREADS) {
		fprintf(logFile, "RELEASED (M)\n");
		fflush(logFile);
	}


	//return 	CGameSpyClient__JoinGroupRoom(pGameSpy, edx, nRoom);
	return true;
}

void (__fastcall *CConnectionLib__UpdateGameSpyClient)();
void __fastcall CConnectionLib__UpdateGameSpyClient_Hook(CConnectionLib *pConnectionLib)
{

	CConnectionLib__UpdateGameSpyClient();

	if(joingroup_called) {
		if(LOG_THREADS) {
			fprintf(logFile, "Main thread [POLL direction] acquiring FetchServers....");
			fflush(logFile);
		}
		
		WaitForSingleObject(threadlock_FetchServers, INFINITE);

		if(LOG_THREADS) {
			fprintf(logFile, "ACQUIRED (M)\n");
			fflush(logFile);
		}


		Poll();		
		
		if(LOG_THREADS) {
			fprintf(logFile, "Main thread releasing FetchServers....");
			fflush(logFile);
		}
		
		ReleaseMutex(threadlock_FetchServers);
		
		if(LOG_THREADS) {
			fprintf(logFile, "RELEASED (M)\n");
			fflush(logFile);
		}		

	}

	// CConnectionLib__UpdateGameSpyClient();


	


}

void HookFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	*(DWORD*)&CGameSpyClient__JoinGroupRoom = 0x00805080;
	*(DWORD*)&CConnectionLib__UpdateGameSpyClient = 0x008027A0;
	int success = DetourAttach(&(PVOID&)CGameSpyClient__JoinGroupRoom, CGameSpyClient__JoinGroupRoom_Hook)==0  &&
				  DetourAttach(&(PVOID&)CConnectionLib__UpdateGameSpyClient, CConnectionLib__UpdateGameSpyClient_Hook)==0;

	fprintf(logFile, "Hooked: %d\n", success);
	DetourTransactionCommit();
}

void InitPlugin()
{
	logFile = fopen(logFileName, "w");
	fprintf(logFile, "NWCX Serverlist plugin 1.0 (PRE-RELEASE)\n");
	fprintf(logFile, "(c) 2013 by virusman & addicted2rpg\n");
	fflush(logFile);
	if(pluginLink){
	}
	HookFunctions();
	CreateMutexes();
}

void CreateMutexes() {
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR SD;

	InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&SD, TRUE, NULL, FALSE);
    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &SD;
    sa.bInheritHandle = TRUE;


	threadlock_JoinGroup = CreateMutexA(&sa, FALSE, NULL);
	threadlock_FetchServers = CreateMutexA(&sa, FALSE, NULL);
	threadlock_dataStatus = CreateMutexA(&sa, FALSE, NULL );
	threadlock_clientClass = CreateMutexA(&sa, FALSE, NULL );

	if(threadlock_JoinGroup == NULL || threadlock_FetchServers == NULL || 
		threadlock_dataStatus == NULL || threadlock_clientClass == NULL) {
		fprintf(logFile, "* Fatal - Mutexes were null.\n");
		fflush(logFile);
		ExitProcess(0);
		return;
	}


}

void InitThread() {
	LPTHREAD_START_ROUTINE  start_address = (LPTHREAD_START_ROUTINE) FetchServers;
	/*
	SIZE_T stack_size = 0;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR SD;


	InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&SD, TRUE, NULL, FALSE);
    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &SD;
    sa.bInheritHandle = TRUE;
	*/


	thandle = CreateThread(NULL, 0, start_address, (void *)roomGlobal, CREATE_SUSPENDED, NULL);
	if(thandle == NULL) {
		fprintf(logFile, "* Fatal - Thread creation failed.\n");
		fflush(logFile);
		ExitProcess(0);
		return;
	}


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
		CloseHandle(threadlock_JoinGroup);
		CloseHandle(threadlock_FetchServers);
		CloseHandle(thandle);
		//delete plugin;
	}
	return TRUE;
}