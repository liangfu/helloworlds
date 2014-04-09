/* \copyright
   The MIT License:

   Copyright (c) 2008 Ivan Gagis <igagis@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

   \brief
   liangfu modified:
   1. concatenate into a single header file
   2. improved comments in code
   3. memory leak fix
   
   Typical usage:

   \code
   #include "qthread.h"
   
   class Test : public Thread
   {
     char m_str[1024];
   public:
     Test(){sprintf(m_str,"=");}
     void run(){
   	   while (!this->m_quitFlag){
   	     fprintf(stderr,"%s",m_str);
   	   }
     }
   };
   
   int main(int argc, char * argv[])
   {
     Test * t=new Test();
     t->start();
     Test::sleep(1);
     t->quit();
     delete t;
     return 0;
   }
   \endcode
*/

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

class Exc {
public:
  Exc(const char * str){fprintf(stderr,"%s\n",str);exit(-1);}
};

class Message;
class Thread;
class Queue;

template <typename T>
class Ptr{
  T * m_ptr;
public:
  Ptr(T * ptr=0):m_ptr(ptr){}
  ~Ptr(){}//if (m_ptr){delete m_ptr;m_ptr=0;}
  int IsValid(){return this->m_ptr!=0;}
  T * Extract(){return this->m_ptr;}
  T * operator->(){return this->m_ptr;}
};

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32)

#ifndef __WIN32__
#define __WIN32__
#endif
#ifndef WIN32
#define WIN32
#endif

#include <windows.h>

#elif defined(__GNUC__) //assume pthread

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <ctime>

#else
#error "unknown platform"
#endif

//forward declarations
class Queue;
class Thread;

/**
   @brief Mutex object class
   Mutex stands for "Mutual execution".
*/
class Mutex
{
  friend class CondVar;

  //system dependent handle
#ifdef __WIN32__
  CRITICAL_SECTION m;
#elif defined(__GNUC__)
  pthread_mutex_t m;
#else
#error "unknown system"
#endif

  //forbid copying
  Mutex(const Mutex& ){};
  Mutex(Mutex& ){};
  Mutex& operator=(const Mutex& ){return *this;};

public:

  Mutex(){
#ifdef __WIN32__
	InitializeCriticalSection(&this->m);
#elif defined(__GNUC__) //pthread
	pthread_mutex_init(&this->m, NULL);
#else
#error "unknown system"
#endif
  };

  ~Mutex(){
#ifdef __WIN32__
	DeleteCriticalSection(&this->m);
#elif defined(__GNUC__) //pthread
	pthread_mutex_destroy(&this->m);
#else
#error "unknown system"
#endif
  };

  /**
   * @brief Acquire mutex lock.
   * If one thread acquired the mutex lock then all other threads
   * attempting to acquire the lock on the same mutex will wait until the
   * mutex lock will be released.
   */
  void Lock(){
#ifdef __WIN32__
	EnterCriticalSection(&this->m);
#elif defined(__GNUC__) //pthread
	pthread_mutex_lock(&this->m);
#else
#error "unknown system"
#endif
  }
	
  /**
   * @brief Release mutex lock.
   */
  void Unlock(){
#ifdef __WIN32__
	LeaveCriticalSection(&this->m);
#elif defined(__GNUC__) //pthread
	pthread_mutex_unlock(&this->m);
#else
#error "unknown system"
#endif
  };

  /**
   * @brief Helper class which automatically Locks the given mutex.
   * This helper class automatically locks the given mutex in the constructor and
   * unlocks the mutex in destructor. This class is useful if the code between
   * mutex lock/unlock may return or throw an exception,
   * then the mutex will not remain locked in such case.
   */
  class LockerUnlocker
  {
	Mutex *mut;

	//forbid copying
	LockerUnlocker(const LockerUnlocker& ){}
	LockerUnlocker(LockerUnlocker& ){}
	LockerUnlocker& operator=(const LockerUnlocker& ){return *this;}
  public:
	LockerUnlocker(Mutex &m):
	  mut(&m)
	{
	  this->mut->Lock();
	};
	~LockerUnlocker(){
	  this->mut->Unlock();
	};
  };
};//~class Mutex


/**
 * @brief Semaphore class.
 */
class Semaphore
{
  //system dependent handle
#ifdef __WIN32__
  HANDLE s;
#elif defined(__GNUC__)
  sem_t s;
#else
#error "unknown system"
#endif

  //forbid copying
  Semaphore(const Semaphore& ){}
  Semaphore(Semaphore& ){}
  Semaphore& operator=(const Semaphore& ){return *this;}
public:

  Semaphore(uint initialValue = 0){
#ifdef __WIN32__
	this->s = CreateSemaphore(NULL, initialValue, 0xffffff, NULL);
	if(this->s == NULL){
	  //        LOG(<<"Semaphore::Semaphore(): failed"<<std::endl)
	  throw Exc("Semaphore::Semaphore(): creating semaphore failed");
	}
#elif defined(__GNUC__)
	if(sem_init(&this->s, 0, initialValue) < 0 ){
	  throw Exc("Semaphore::Semaphore(): creating semaphore failed");
	}
#else
#error "unknown system"
#endif
  }

  ~Semaphore(){
#ifdef __WIN32__
	CloseHandle(this->s);
#elif defined(__GNUC__)
	sem_destroy(&this->s);
#else
#error "unknown system"
#endif
  }

  bool Wait(uint timeoutMillis = 0){
#ifdef __WIN32__
	switch( WaitForSingleObject(this->s, DWORD(timeoutMillis == 0 ? INFINITE : timeoutMillis)) ){
	case WAIT_OBJECT_0:
	  //LOG(<<"Semaphore::Wait(): exit"<<std::endl)
	  return true;
	case WAIT_TIMEOUT:
	  return false;
	  break;
	default:
	  throw Exc("Semaphore::Wait(): wait failed");
	  break;
	}
#elif defined(__GNUC__)
	if(timeoutMillis == 0){
	  int retVal;
	  do{
		retVal = sem_wait(&this->s);
	  }while(retVal == -1 && errno == EINTR);
	  if(retVal < 0){
		throw Exc("Semaphore::Wait(): wait failed");
	  }
	}else{
	  timespec ts;
	  ts.tv_sec = timeoutMillis / 1000;
	  ts.tv_nsec = (timeoutMillis % 1000) * 1000 * 1000;
	  if(sem_timedwait(&this->s, &ts) != 0){
		if(errno == ETIMEDOUT)
		  return false;
		else
		  throw Exc("Semaphore::Wait(): error");
	  }
	  return true;
	}
#else
#error "unknown system"
#endif
  }

  //TODO: remove this method
  // inline void Post(){
  // 	fprintf(stderr,"Semaphore::Post(): is deprecated, use Semaphore::Signal()\n");
  // 	this->Signal();
  // }

  inline void Signal(){
#ifdef __WIN32__
	if( ReleaseSemaphore(this->s, 1, NULL) == 0 ){
	  throw Exc("Semaphore::Post(): releasing semaphore failed");
	}
#elif defined(__GNUC__)
	if(sem_post(&this->s) < 0){
	  throw Exc("Semaphore::Post(): releasing semaphore failed");
	}
#else
#error "unknown system"
#endif
  }
};

class CondVar
{
#if defined(__WIN32__)
  Mutex cvMutex;
  Semaphore semWait;
  Semaphore semDone;
  uint numWaiters;
  uint numSignals;
#elif defined(__GNUC__)
  //A pointer to store system dependent handle
  pthread_cond_t cond;
#else
#error "unknown system"
#endif

  //forbid copying
  CondVar(const CondVar& ){};
  CondVar(CondVar& ){};
  CondVar& operator=(const CondVar& ){return *this;};
public:

  CondVar(){
#if defined(__WIN32__)
	this->numWaiters = 0;
	this->numSignals = 0;
#elif defined(__GNUC__)
	pthread_cond_init(&this->cond, NULL);
#else
#error "unknown system"
#endif
  }

  ~CondVar(){
#if defined(__WIN32__)
#elif defined(__GNUC__)
	pthread_cond_destroy(&this->cond);
#else
#error "unknown system"
#endif
  }

  void Wait(Mutex& mutex)
  {
#if defined(__WIN32__)
	this->cvMutex.Lock();
	++this->numWaiters;
	this->cvMutex.Unlock();

	mutex.Unlock();

	this->semWait.Wait();

	this->cvMutex.Lock();
	if(this->numSignals > 0){
	  this->semDone.Post();
	  --this->numSignals;
	}
	--this->numWaiters;
	this->cvMutex.Unlock();

	mutex.Lock();
#elif defined(__GNUC__)
	pthread_cond_wait(&this->cond, &mutex.m);
#else
#error "unknown system"
#endif
  }

  void Notify()
  {
#if defined(__WIN32__)
	this->cvMutex.Lock();

	if(this->numWaiters > this->numSignals){
	  ++this->numSignals;
	  this->semWait.Post();
	  this->cvMutex.Unlock();
	  this->semDone.Wait();
	}else{
	  this->cvMutex.Unlock();
	}
#elif defined(__GNUC__)
	pthread_cond_signal(&this->cond);
#else
#error "unknown system"
#endif
  }
};

class Message
{
  friend class Queue;

  Message *next;//pointer to the next message in a single-linked list

protected:
  Message() : next(0) {}

public:
  virtual ~Message(){}

  virtual void Handle()=0;
};

class Queue
{
  CondVar cond;

  Mutex mut;

  Message *first,*last;

  //forbid copying
  Queue(const Queue& ){}
  Queue(Queue& ){}
  Queue& operator=(const Queue& ){return *this;}

public:
  Queue() : first(0), last(0)
  {}

  ~Queue()
  {
	Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
	Message *msg = this->first;
	Message	*nextMsg;
	while(msg){
	  nextMsg = msg->next;
	  delete msg;
	  msg = nextMsg;
	}
  }

  void PushMessage(Ptr<Message> msg)
  {
  	{
  	  Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
  	  if(this->first){
  		assert(this->last && this->last->next == 0);
		this->last = this->last->next = msg.Extract();
		assert(this->last->next == 0);
	  }else{
  		assert(msg.IsValid());
		this->last = this->first = msg.Extract();
  	  }
  	}
  	this->cond.Notify();
  }

  Ptr<Message> PeekMsg()
  {
  	Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
  	if(this->first){
  	  Message* ret = this->first;
  	  this->first = this->first->next;
  	  return Ptr<Message>(ret);
  	}
  	return Ptr<Message>();
  };

  Ptr<Message> GetMsg()
  {
  	Mutex::LockerUnlocker mutexLockerUnlocker(this->mut);
  	if(this->first){
  	  Message* ret = this->first;
  	  this->first = this->first->next;
  	  return Ptr<Message>(ret);
  	}
  	this->cond.Wait(this->mut);
  	assert(this->first);
  	  Message* ret = this->first;
  	this->first = this->first->next;
  	return Ptr<Message>(ret);
  }
};

/**
   @brief a base class for threads.
   This class should be used as a base class for thread objects, one should override the
   Thread::Run() method.
*/
class Thread
{
  friend class QuitMessage;

  //Thread Run function
#ifdef __WIN32__
  static DWORD __stdcall RunThread(void *data)
#elif defined(__GNUC__) //pthread
  static void* RunThread(void *data)
#else
#error "unknown system"
#endif
  {
	Thread *thr = reinterpret_cast<Thread*>(data);
	try{
	  thr->run();
	}catch(...){
	  assert(false);
	}

#ifdef __GNUC__
	pthread_exit(0);
#endif
	return 0;
  };

  Mutex mutex;

  Ptr<Message> preallocatedQuitMessage;

  volatile bool m_isRunning;//true if thread is running

  //system dependent handle
#if defined(__WIN32__)
  HANDLE th;
#elif defined(__GNUC__)
  pthread_t th;
#else
#error "unknown system"
#endif

  /**
   * @brief Send a message to thread's queue.
   * @param msg - a message to send.
   */
  // void pushMessage(Ptr<Message> msg){
  // 	this->queue.PushMessage(msg);
  // }
  
  //forbid copying
  Thread(const Thread& ){}
  Thread(Thread& ){}
  Thread& operator=(const Thread& ){return *this;}

protected:
  // looks like it is not necessary to protect this flag by mutex,
  // volatile will be enough
  volatile bool m_quitFlag;

  // Queue queue;
  
  /**
	 @brief This should be overriden, this is what to be run in new thread.
	 Pure virtual method, it is called in new thread when thread runs.
  */
  virtual void run()=0;
  
public:
  inline Thread();//see implementation below as inline method

  virtual ~Thread()
  {
	this->m_quitFlag = true;
	assert(this->preallocatedQuitMessage.IsValid());
	//this->pushMessage(this->preallocatedQuitMessage);
	this->quit();
	this->join();
  }

  inline int isRunning(){return m_isRunning;}
  
  /**
	 @brief Start thread execution.
	 @param stackSize - size of the stack in bytes which should be allocated for this thread.
	 If stackSize is 0 then system default stack size is used.
  */
  //0 stacksize stands for default stack size (platform dependent)
  void start(uint stackSize = 0)
  {
	// protect by mutex to avoid several Start() methods to be called by
	// concurrent threads simultaneously
	Mutex::LockerUnlocker mutexLockerUnlocker(this->mutex);

	if(this->m_isRunning){
	  throw Exc("Thread::Start(): Thread is already running");
	}
	this->m_quitFlag=0;

#if defined(__WIN32__)
	this->th = CreateThread(NULL, static_cast<size_t>(stackSize), &RunThread,
							reinterpret_cast<void*>(this), 0, NULL);
	if(this->th == NULL){
	  throw Exc("Thread::Start(): starting thread failed");
	}
#elif defined(__GNUC__)
	{
	  pthread_attr_t attr;

	  pthread_attr_init(&attr);
	  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	  pthread_attr_setstacksize(&attr, static_cast<size_t>(stackSize));

	  if(pthread_create(&this->th, &attr, &RunThread, this) != 0){
		pthread_attr_destroy(&attr);
		throw Exc("Thread::Start(): starting thread failed");
	  }
	  pthread_attr_destroy(&attr);
	}
#else
#error "unknown system"
#endif
	this->m_isRunning = true;
  };

  /**
   * @brief Wait for thread finish its execution.
   */
  void join()
  {
	// protect by mutex to avoid several Join() methods to be called by
	// concurrent threads simultaneously
	Mutex::LockerUnlocker mutexLockerUnlocker(this->mutex);

	if (!this->m_isRunning){ return; }

#ifdef __WIN32__
	WaitForSingleObject(this->th, INFINITE);
	CloseHandle(this->th);
	this->th = NULL;
#elif defined(__GNUC__)
	pthread_join(this->th, 0);
#else
#error "unknown system"
#endif
	this->m_isRunning = false;
  };

  /**
   * @brief Suspend the thread for a given number of milliseconds.
   * @param msec - number of milliseconds the thread should be suspended.
   */
  static void sleep(uint msec = 0)
  {
#ifdef __WIN32__
	// Sleep() crashes on mingw (I do not know why),
	// this is why I use SleepEx() here.
	SleepEx(DWORD(msec), FALSE);
#elif defined(__GNUC__)
	if(msec == 0){
	  pthread_yield();
	}else{
	  usleep(msec * 1000);
	}
#else
#error "unknown system"
#endif
  }

  /**
   * @brief Send 'Quit' message to thread's queue.
   */
  inline void quit();//see implementation below

};

class QuitMessage : public Message
{
  Thread *thr;
public:
  QuitMessage(Thread * thread) : thr(thread)
  {
	if(!this->thr){
	  throw Exc("QuitMessage::QuitMessage(): thread pointer passed is 0");
	}
  }
  
  void Handle(){this->thr->m_quitFlag = 1;}
};

inline void Thread::quit()
{
  //this->pushMessage(new QuitMessage(this));
  this->m_quitFlag=1;
  this->m_isRunning=0;
}

inline Thread::Thread() :
  preallocatedQuitMessage(new QuitMessage(this)),m_isRunning(false), m_quitFlag(false)
{
#if defined(__WIN32__)
  this->th = NULL;
#elif defined(__GNUC__)
  //do nothing
#else
#error "unknown system"
#endif
};

#endif // __QTHREAD_H__

