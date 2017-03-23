#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <stdlib.h>

#include "window.h"

int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR pCmdLine, int nCmdShow){
	WindowApp *g = new WindowApp();

	if( g && g->Initialize(hInstance, nCmdShow)){
		while(true){
			if(!g->ProcessWindowMessage())
				break;
		}

		delete g;
	}

	return 0;
}