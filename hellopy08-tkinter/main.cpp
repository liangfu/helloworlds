#include <Python.h>
#include <pythonrun.h>

#if defined(WIN32)
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#define MAX_PATH 1024
char * GetThisPath(char * dest, size_t destSize)
{
    if (!dest) return NULL;
    if (MAX_PATH > destSize) return NULL;

    DWORD length = GetModuleFileNameA( NULL, dest, destSize );
    PathRemoveFileSpecA(dest);
    return dest;
}
#endif

int main(int argc, char *argv[])
{
#if defined(WIN32)
  char dst[1024];
  char thispath[1024];
  sprintf(thispath,"%s",GetThisPath(dst,1024));
#endif

  Py_NoSiteFlag=1;
  Py_SetProgramName(argv[0]);
  Py_SetPythonHome(".");
  Py_InitializeEx(0);
  PySys_SetArgv(argc, argv);
  PyRun_SimpleString("import sys");
#if defined(WIN32)
  char syspath[4096];
  sprintf(syspath,"sys.path = ['%s\\.','%s\\DLLs','%s\\python27.zip','%s\\python27.zip/Lib','%s\\python27.zip/Lib/lib-tk','%s\\python27.zip/Lib/site-packages']",
    thispath,thispath,thispath,thispath,thispath,thispath);
  PyRun_SimpleString(syspath);
#else
  PyRun_SimpleString("sys.path = ['.','DLLs','python27.zip','python27.zip/Lib','python27.zip/Lib/lib-tk','python27.zip/Lib/site-packages']");
#endif

  PyRun_SimpleString("from time import time,ctime\n"
    "import sys\n"
    "import os\n"
    //"print sys.argv\n"
    //"print os.listdir('.')\n"
    //"print 'Today is',ctime(time())\n"
    //"print os.getcwd()\n"
    //"print sys.path\n"
    //"import _tkinter;"
    //"import Tkinter;"
    //"from hello import Application\n"
    "import hello\n"
    //"root = Tkinter.Tk()\n"
    //"app = Application(master=root)\n"
    //"app.mainloop()\n"
    //"root.destroy()\n"
    );
  //system("pause");
  Py_Finalize();

  return 0;
}
