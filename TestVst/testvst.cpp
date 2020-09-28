#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <shellapi.h>

#define APPLICATION_CLASS_NAME "dssi_vst"

#define OLD_PLUGIN_ENTRY_POINT "main"
#define NEW_PLUGIN_ENTRY_POINT "VSTPluginMain"

#ifdef VESTIGE
typedef int16_t VstInt16;	
typedef int32_t VstInt32;		
typedef int64_t VstInt64;		
typedef intptr_t VstIntPtr;
#define VESTIGECALLBACK __cdecl
#include "vestige.h"
#else
#include "pluginterfaces/vst2.x/aeffectx.h"
#endif

#ifdef VESTIGE
typedef AEffect *(VESTIGECALLBACK *VstEntry)(audioMasterCallback audioMaster);
#else
typedef AEffect *(VSTCALLBACK *VstEntry)(audioMasterCallback audioMaster);
#endif

using namespace std;

VstIntPtr VESTIGECALLBACK hostCallback(AEffect *plugin, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)    
{		 
VstIntPtr rv = 0;
    
     switch (opcode)
     {  
     case audioMasterVersion:
     rv = 2400;
     break;
        
     default:
     break;           
     }    
    
     return rv; 
}  

std::string getMaker(AEffect *m_plugin)
{
      char buffer[512];
      memset(buffer, 0, sizeof(buffer));
      m_plugin->dispatcher(m_plugin, effGetVendorString, 0, 0, buffer, 0);
      if (buffer[0])
      return buffer;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdline, int cmdshow)
{ 
LPWSTR *args;
int numargs;
   
    cout << "LinVst Vst Test: " << cmdline << endl;	

    args = CommandLineToArgvW(GetCommandLineW(), &numargs);
   
    if(args == NULL)
    {
    printf("CommandLine arguments failed\n");
    exit(0);
    }
    
    if(args[1] == NULL)
    {
	printf("Usage: vstest.exe path_to_vst_dll\n");	
	exit(0);
	}	
  
    HINSTANCE libHandle = 0;
    libHandle = LoadLibraryW(args[1]);
    if (!libHandle)
    {
	printf("LibraryLoadError\n");	
	exit(0);
	}	  
	
    LocalFree(args);
	
    VstEntry getinstance = 0;

    getinstance = (VstEntry)GetProcAddress(libHandle, NEW_PLUGIN_ENTRY_POINT);

    if (!getinstance) {
    getinstance = (VstEntry)GetProcAddress(libHandle, OLD_PLUGIN_ENTRY_POINT);
    if (!getinstance) {
    cout << "dssi-vst-server: ERROR: VST entrypoints \"" << NEW_PLUGIN_ENTRY_POINT << "\" or \""
                << OLD_PLUGIN_ENTRY_POINT << "\" not found in DLL \"" << "\"" << endl;     
 
    if(libHandle)
    FreeLibrary(libHandle);    
    exit(0);    
    }
    }	
	
    AEffect *m_plugin = getinstance(hostCallback);
    if (!m_plugin)	
    {
	printf("AEffectError\n");	
    if(libHandle)
    FreeLibrary(libHandle);    
    exit(0);    
    }	
      
    if (m_plugin->magic != kEffectMagic)
    {
    cout << "dssi-vst-server: ERROR: Not a VST plugin in DLL \"" << "\"" << endl;
	if(libHandle)
	FreeLibrary(libHandle);	    
	exit(0);    
    }

    if (!(m_plugin->flags & effFlagsCanReplacing))
    {
    cout << "dssi-vst-server: ERROR: Plugin does not support processReplacing (required)" << endl;
	if(libHandle)
	FreeLibrary(libHandle);	    
	exit(0);    
    }

    if(m_plugin->flags & effFlagsHasEditor)
    {
	printf("HasEditor\n");	
    }	
    else 
    { 
        printf("NoEditor\n");
	if(libHandle)
	FreeLibrary(libHandle);	    
	exit(0);        
    }  
	
	m_plugin->dispatcher( m_plugin, effOpen, 0, 0, NULL, 0);  
	
	printf("NumInputs %d\n", m_plugin->numInputs);
    printf("NumOutputs %d\n", m_plugin->numOutputs);
		
	cout << "Maker " << getMaker(m_plugin)	<< endl;	
	
	WNDCLASSEX          wclass;
		
    memset(&wclass, 0, sizeof(WNDCLASSEX));
    wclass.cbSize = sizeof(WNDCLASSEX);
    wclass.style = 0;
	// CS_HREDRAW | CS_VREDRAW;
    wclass.lpfnWndProc = DefWindowProc;
    wclass.cbClsExtra = 0;
    wclass.cbWndExtra = 0;
    wclass.hInstance = GetModuleHandle(0);
    wclass.hIcon = LoadIcon(GetModuleHandle(0), APPLICATION_CLASS_NAME);
    wclass.hCursor = LoadCursor(0, IDI_APPLICATION);
    // wclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wclass.lpszMenuName = "MENU_DSSI_VST";
    wclass.lpszClassName = APPLICATION_CLASS_NAME;
    wclass.hIconSm = 0;

    if (!RegisterClassEx(&wclass))
    {
	m_plugin->dispatcher( m_plugin, effClose, 0, 0, NULL, 0);  	
    if(libHandle)
    FreeLibrary(libHandle);  	
	exit(0);
	}

    HWND hWnd = CreateWindow(APPLICATION_CLASS_NAME, "LinVst", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
	    
    if (!hWnd)
    {
	UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
	m_plugin->dispatcher( m_plugin, effClose, 0, 0, NULL, 0);  	
    if(libHandle)
    FreeLibrary(libHandle);  	
	exit(0);		
	}
	
    ERect *rect = 0;
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);
    m_plugin->dispatcher(m_plugin, effEditOpen, 0, 0, hWnd, 0);
    m_plugin->dispatcher(m_plugin, effEditGetRect, 0, 0, &rect, 0);
    if (!rect)
    {
    m_plugin->dispatcher(m_plugin, effEditClose, 0, 0, 0, 0);	
	DestroyWindow(hWnd);	
	UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
	m_plugin->dispatcher( m_plugin, effClose, 0, 0, NULL, 0);  	
    if(libHandle)
    FreeLibrary(libHandle);  	
	exit(0);
    }
	
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rect->right - rect->left + 6, rect->bottom - rect->top + 25, SWP_NOMOVE);	
	    
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
        
    UINT_PTR timerval;
	    
    timerval = 678;
    timerval = SetTimer(hWnd, timerval, 80, 0);	
        
    int count = 0; 
	
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
        
    if((msg.message == WM_TIMER) && (msg.wParam == 678)) 
    {
    count += 1;
    
    if(count == 100)
    break;
    }              
    }	
    m_plugin->dispatcher(m_plugin, effEditClose, 0, 0, 0, 0);	
     
	DestroyWindow(hWnd);	
	UnregisterClassA(APPLICATION_CLASS_NAME, GetModuleHandle(0));
	        
    m_plugin->dispatcher( m_plugin, effClose, 0, 0, NULL, 0);  
	
    if(libHandle)
    FreeLibrary(libHandle);    	  	
}

