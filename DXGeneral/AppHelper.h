
#if defined(DEBUG) | defined(_DEBUG) 

#define DEFAULT_WINMAIN(className) \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, \
	PSTR cmdLine, int showCmd) \
{ \
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
	className theApp(hInstance); \
	if (!theApp.Init()) \
		return 0; \
	return theApp.Run(); \
}

#else

#define DEFAULT_WINMAIN(className) \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, \
	PSTR cmdLine, int showCmd) \
{ \
	className theApp(hInstance); \
	if (!theApp.Init()) \
		return 0; \
	return theApp.Run(); \
}

#endif 
