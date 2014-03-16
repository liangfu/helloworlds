#include <Python.h>

int main(int argc, char *argv[])
{
	Py_NoSiteFlag=1;
	Py_SetProgramName(argv[0]);
	Py_SetPythonHome(".");
	Py_InitializeEx(0);
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path = ['.','python27.zip','python27.zip/Lib']");

  PyRun_SimpleString("from time import time,ctime\n"
										 "import sys\n"
										 "import os\n"
										 "print os.listdir('.')\n"
                     "print 'Today is',ctime(time())\n");
  Py_Finalize();
  return 0;
}
