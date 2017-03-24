#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <stdlib.h>

#include "window.h"

int WINAPI _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR pCmdLine, int nCmdShow){
	WindowApp *w = new WindowApp();

	if( w && w->Initialize(hInstance, nCmdShow)){
		while(true){
			if(!w->ProcessWindowMessage())
				break;
		}

		delete w;
	}

	return 0;
}