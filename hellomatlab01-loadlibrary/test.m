function test
    
% oldenv=getenv('LD_LIBRARY_PATH');
% setenv('LD_LIBRARY_PATH',pwd);
thislib='test';
[nf,warn]=loadlibrary('libtest','libtest.h','alias',thislib);

calllib(thislib,'add',1,2)

unloadlibrary(thislib)
% setenv('LD_LIBRARY_PATH',oldenv);
