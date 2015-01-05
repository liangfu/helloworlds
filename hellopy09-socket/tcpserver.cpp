#include <Python.h>
#include <pythonrun.h>

int main(int argc, char *argv[])
{
  Py_NoSiteFlag=1;
  Py_SetProgramName(argv[0]);
  Py_SetPythonHome(".");
  Py_InitializeEx(0);
  PySys_SetArgv(argc, argv);
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path = ['.','DLLs','python27.zip','python27.zip/Lib','python27.zip/Lib/lib-tk']");

  PyRun_SimpleString("from time import time,ctime\n"
    "import sys\n"
    "import os\n"
    "print sys.argv\n"
    "print os.listdir('.')\n"
    "print 'Today is',ctime(time())\n"
    "print os.getcwd()\n"
    "print sys.path\n"
    "import tcpserver\n"
    );
  //system("pause");
  Py_Finalize();

  return 0;
}
