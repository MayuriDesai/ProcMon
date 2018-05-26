//#include<iostream>
using namespace std;

#include<sys/types.h>
#include<sys/stat.h>
#include<iostream>
#include<string.h>
#include<windows.h>
#include<tlhelp32.h>
#include<stdio.h>
#include<io.h>

typedef struct LogFile
{
	char ProcessName[100];
	unsigned int pid;
	unsigned int ppid;
	unsigned int thread_cnt;

}LOGFILE;


class ThreadInfo
{
  private:
	  DWORD PID;
	  HANDLE hThreadSnap;
	  THREADENTRY32 te32;

  public:
	   ThreadInfo(DWORD);
	   BOOL ThreadDisplay();
};

ThreadInfo::ThreadInfo(DWORD no) /* initializing threadinfo class member with parameter comes to this constuctor*/
	                             /*that parameter is id of current process*/
{
  PID= no;/*initializing pid which is member of threadinfo class as current process id */

  hThreadSnap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,PID);/*This function takes a snapshot of the processes, modules*/
	                                                             /*and threads used by the processes. 1ST parameter includes thread list */
	                                                             /*in the snapshot.   2nd parameter is pid of */
																  /*current process .this API return handle (if API succeful return valid value) */
  if(hThreadSnap == INVALID_HANDLE_VALUE)
  {
     cout<<"Unable to create snapshot of current thread pool"<<endl;
	 return;
  }
  te32.dwSize = sizeof(THREADENTRY32);/*dwsize is member of  THREADENTRY32.Length, in bytes, of the structure. */
                                        /* initializing that member by setting sizeof THREADENTRY32 ,for preventing Thread32First failure. */

}

BOOL ThreadInfo::ThreadDisplay()/*displays details of threads*/
{
   if(! Thread32First(hThreadSnap,&te32))/*This function retrieves information about the first thread entered in a system snapshot.*/
	                                        /* 1st parameter is handle of current thread. 2nd parameter is empty object of THREADENTRY32 */
											/*which comes with filling data in it..return value indicates data is filled in obj or not*/
   {
      cout<<"Error:In getting first thread"<<endl;
      CloseHandle(hThreadSnap);
      return FALSE;
   }

   cout<<endl<<"THREAD OF THIS PROCESS : "<<endl;

   do
   {
     if(te32.th32OwnerProcessID == PID)/*Member of obj of THREADENTRY32 is Identifier of the process that created the thread checked with given pid*/
	 {
	   cout<<endl<<"\t THREAD ID: "<<te32.th32ThreadID<<endl;/*if process id of current thread matches display thread id*/
	 
	 }
   
   }while(Thread32Next(hThreadSnap,&te32));/*by using this API check next thread is present in handle of threadsnap i.e. in thread list*/

   CloseHandle(hThreadSnap);

   return TRUE;
}

///////////////

class DLLInfo
{
  private:
	  DWORD PID;
	  HANDLE hProcessSnap;
	  MODULEENTRY32 me32;

  public:
	   DLLInfo(DWORD);
	   BOOL DependentDLLDisplay();
};

DLLInfo::DLLInfo(DWORD no)
{
  PID= no;
  hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,PID);
  if(hProcessSnap == INVALID_HANDLE_VALUE)
  {
     cout<<"Unable to create snapshot of current thread pool"<<endl;
	 return;
  }
  me32.dwSize = sizeof(MODULEENTRY32);

}

BOOL  DLLInfo::DependentDLLDisplay()
{
	char arr[200];

   if(! Module32First(hProcessSnap,&me32))
   {
      cout<<"FAILED In getting  DLL information"<<endl;
      CloseHandle(hProcessSnap);
      return FALSE;
   }

   cout<<endl<<"DEPENDENT DLL OF THIS PROCESS : "<<endl;

   do
   {

	   wcstombs_s(NULL,arr,200,me32.szModule,200);
    
	   cout<<arr<<endl;

   }while(Module32Next(hProcessSnap,&me32));

   CloseHandle(hProcessSnap);
   return TRUE;
}

///////////////

class ProcessInfo
{
  private:
	  DWORD PID;
	  DLLInfo * pdobj;
	  ThreadInfo *ptobj;
	  HANDLE hProcessSnap;
	  PROCESSENTRY32 pe32; /*THIS  structure describes an entry of processes from a list of process*/

  public:
	   ProcessInfo();
	   BOOL ProcessDisplay(char *);
	   BOOL ProcessLog();
	   BOOL ReadLog(DWORD,DWORD,DWORD,DWORD);
	   BOOL PrecessSearch(char *);
	   BOOL KillProcess(char *);
};


ProcessInfo::ProcessInfo( ) /*when user enters ps command this constuctor gets call*/
{
	ptobj=NULL; /* initializing pointer of class threadinfo */
	pdobj=NULL;/* initializing pointer of class dllinfo*/


    hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); /*This function takes a snapshot of the processes, modules*/
	                                                             /*and threads used by the processes. 1ST parameter includes process  list */
	                                                             /*in the snapshot.   2nd parameter is 0 which indicates we want snapshot of */
																  /*current process .this API return handle (if API succeful return valid value) */
  if(hProcessSnap == INVALID_HANDLE_VALUE)
  {
     cout<<"Unable to create snapshot of current running process"<<endl;
	 return;
  }

  pe32.dwSize = sizeof(PROCESSENTRY32); /*dwsize is member of  PROCESSENTRY32..Length, in bytes, of the structure. */
                                        /* initializing that member by setting sizeof processentry32 ,for preventing Process32First failure. */

}

BOOL  ProcessInfo::ProcessLog()
{
	char * month[]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};/*initialize month array for displaying month of */
																							 /*creating & displaying month of log file*/
	char  FileName[50],arr[512];
	int ret=0, fd=0, count=0;
	SYSTEMTIME lt;
	LOGFILE fobj;
	FILE *fp;

	GetLocalTime(&lt);/*for getting current local time from systemtime, parameter send empty so that info abt time will filled in that*/

	sprintf(FileName,"trial.txt",lt.wHour,lt.wMinute,lt.wDay,month[lt.wMonth - 1]);/*creating txt file with given hr,min,day,month info*/
	                                                                                 /* from obj of SYSTEMTIME */
	fp=fopen(FileName,"wb");/*returns pointer of file if file created*/

	if(fp == NULL)
	{
	  cout<<"Unable to create log file"<<endl;
	  return FALSE;
	}
	else/*displaying info abt log file*/
	{
	   cout<<"Log file sucessfully gets created as: "<<FileName<<endl;
	    cout<<"Time of log file creation is ->  "<<lt.wHour<<":"<<lt.wMinute<<":"<<lt.wDay<<"th"<<month[lt.wMonth - 1]<<endl;
	}


   if(! Process32First(hProcessSnap,&pe32))/*This function retrieves information about the first process entered in a system snapshot.*/
	                                        /* 1st parameter is handle of current process 2nd parameter is empty object of PROCESSENTRY32 */
											/*which comes with filling data in it..return value indicates data is filled in obj or not*/
   {
      cout<<"ERROR : In finding the first process."<<endl;
      CloseHandle(hProcessSnap);
      return FALSE;
   }

   //cout<<endl<<"DEPENDENT DLL OF THIS PROCESS : "<<endl;

   do
   {

	   wcstombs_s(NULL,arr,200,pe32.szExeFile,200);
       strcpy_s(fobj.ProcessName,arr);/* fobj is obj of structure LOGFILE & processname is data member of that stucture. */
									  /*processname from array is copyied to datamember of fobj */
      fobj.pid=pe32.th32ProcessID;/*process id assign to datamember pid of fobj */
	  fobj.ppid=pe32.th32ParentProcessID;/* parent process id assign to datamember ppid of fobj*/
	  fobj.thread_cnt=pe32.cntThreads;/*thread count of given process is assigned to fobj*/

	  fwrite(&fobj,sizeof(fobj),1,fp);/*write all info from fobj into file using fp*/
   }while(Process32Next(hProcessSnap,&pe32));/*check next process from handle of processsnap */

   CloseHandle(hProcessSnap);
   fclose(fp);
   return TRUE;
}

/* if ps-a is commend,display all process details, if ps -d is a command ,display first process details of that process for which DLL is used. */
/* if ps -t is a command ,display first process details of that process of which thread is used.*/

BOOL ProcessInfo::ProcessDisplay(char * option) /*display information of parameter which comes from main(user)*/
{
	char arr[200];

   if(! Process32First(hProcessSnap,&pe32)) /*This function retrieves information about the first process entered in a system snapshot.*/
	                                        /* 1st parameter is handle of current process 2nd parameter is empty object of PROCESSENTRY32 */
											/*which comes with filling data in it..return value indicates data is filled in obj or not*/
   {
      cout<<"Error:In finding the first process"<<endl;
      CloseHandle(hProcessSnap);/*closing handle of current process*/
      return FALSE;
   }

   cout<<endl<<"DEPENDENT DLL OF THIS PROCESS : "<<endl;

   do
   {
	   cout<<endl<<"------------------------------------";
	   wcstombs_s(NULL,arr,200,pe32.szExeFile,200);/* convert unicode to asskey because unicode stores in 2 bye,4th parameter is obj of */
													 /*PROCESSENTRY32 with its data member which contains name of exe file  */
	   cout<<endl<<"PROCESS NAME :"<<arr;/*arr coontains name of process*/
	   cout<<endl<<"PID :"<<pe32.th32ProcessID;/* Identifier of the process. */
	   cout<<endl<<"Parent Process :"<<pe32.th32ParentProcessID;/*Identifier of the parent process. */
	   cout<<endl<<"NO of threads :"<<pe32.cntThreads;/*count of threads used by process*/

	   if( (_stricmp(option,"-a")==0)||(_stricmp(option,"-d")==0)||(_stricmp(option,"-t")==0) )/*checking of parameter send from main*/
	   {
	       if(   (_stricmp(option,"-t")==0)||(_stricmp(option,"-a")==0)  )
		   {
		      ptobj=new ThreadInfo(pe32.th32ProcessID);/*creating dyanamic memory of class threadinfo by calling constuctor */
													  /*with parameter of id of current process,so that we can get info of threads of current process*/
		      ptobj->ThreadDisplay();/* displaying information of threads of current process. using pointer.*/
		      delete ptobj;
		   }

		    if(   (_stricmp(option,"-d")==0)||(_stricmp(option,"-a")==0)  )
		   {
		      pdobj=new DLLInfo(pe32.th32ProcessID);/*creating dyanamic memory of class DLLinfo by calling constuctor */
													  /*with parameter of id of current process,so that we can get info of DLL of current process*/
		      pdobj->DependentDLLDisplay();/* displaying information of DLL of current process. using pointer.*/
		      delete pdobj;
		   }
	   }
	  
	   cout<<endl<<"------------------------------------";
   }while(Process32Next(hProcessSnap,&pe32));/*checking of next process is present or not from given processsnap*/

   CloseHandle(hProcessSnap);
   return TRUE;
}


BOOL ProcessInfo ::ReadLog(DWORD hr,DWORD min,DWORD date,DWORD month)
{
	char FileName[50];
	char * montharr[]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};/*for storing info abt month taken from user*/
	int ret=0,count=0;
	LOGFILE fobj;
	FILE * fp;

	sprintf(FileName,"trial.txt",hr,min,date,montharr[month - 1]);
	fp=fopen(FileName,"rb");

	if(fp==NULL)
	{
	   cout<<"Unable to OPEN log file named as: "<<FileName<<endl;
	  return FALSE;
	}

	while((ret= fread(&fobj,1,sizeof(fobj),fp)) !=0 ) /*reading containts from file returning value 1 if succesful ,storing containts from file to fobj*/
	{
	    cout<<endl<<"------------------------------------";
	   cout<<endl<<"PROCESS NAME :"<<fobj.ProcessName<<endl;/* accessing from fobj name of process*/
	   cout<<endl<<"PID of current precess:"<<fobj.pid<<endl;/* accessing from fobj pid of process*/
	   cout<<endl<<"Parent Process :"<<fobj.ppid<<endl;/* accessing from fobj pid of parent process*/
	   cout<<endl<<"Thread count of process:"<<fobj.thread_cnt<<endl;/* accessing from fobj  thread count of process*/
	
	}

	return TRUE; 
}

BOOL ProcessInfo::PrecessSearch(char * name)
{
	char arr[200];
	BOOL Flag=FALSE;

	if(! Process32First(hProcessSnap,&pe32))/*This function retrieves information about the first process entered in a system snapshot.*/
	                                        /* 1st parameter is handle of current process 2nd parameter is empty object of PROCESSENTRY32 */
											/*which comes with filling data in it..return value indicates data is filled in obj or not*/
   {
      CloseHandle(hProcessSnap);/*if  Process32First fails*/
      return FALSE;
   }

	do
	{
          wcstombs_s(NULL,arr,200,pe32.szExeFile,200);

	  if(_stricmp(arr,name) == 0)/* cheking of name of file comes as parameter to search funtion with array name of file*/
	  {
	   cout<<endl<<"------------------------------------";
	   cout<<endl<<"PROCESS NAME :"<<arr<<endl;/*array  name*/
	   cout<<endl<<"PID :"<<pe32.th32ProcessID<<endl;
	   cout<<endl<<"Parent Process PID :"<<pe32.th32ParentProcessID<<endl;
	   cout<<endl<<"No of Thread :"<<pe32.cntThreads;

	    Flag=TRUE;
		break;
	   }
	
	}while(Process32Next(hProcessSnap,&pe32) );
	
	 CloseHandle(hProcessSnap);
	return Flag;/*return 1 if succesfully flag=1*/
}

BOOL ProcessInfo::KillProcess(char * name)
{
	char arr[200];
	int pid=-1;
	BOOL bret;
	HANDLE hProcess;

	if(! Process32First(hProcessSnap,&pe32))
   {
      CloseHandle(hProcessSnap);
      return FALSE;
   }

	do
	{
          wcstombs_s(NULL,arr,200,pe32.szExeFile,200);

	  if(_stricmp(arr,name) == 0)
	  {
	    pid=pe32.th32ProcessID;/*assigning id of process ,if process present otherwise return -1*/
		break;
	   }
	
	}while(Process32Next(hProcessSnap,&pe32) );
	
	 CloseHandle(hProcessSnap);
	
	 if(pid == -1)/*user not giving file name*/
	 {
	    cout<<"Error: there is no such process"<<endl;
		return FALSE;
	 }

	 hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,pid);/* 1st parameter are rights given to process,if that given paramter named right is checked*/
													/*agaist processes security. 2ndparamter if this value is TRUE, processes created by this*/
													/*process will inherit the handle. 3rd paramter id of process which has to open*/
													/*if that id is id of system proces this api fails*/
													/*If the function succeeds, the return value is an open handle to the specified process.*/
													/*If the function fails, the return value is NULL.*/
	  if(  hProcess ==  NULL)
	 {
	    cout<<"Error: there is no access to terminate"<<endl;
		return FALSE;
	 }

	  bret=TerminateProcess( hProcess,0);/*terminating process by giving handle of that process..The exit code to be used by the process and */
	                                     /*threads terminated as a result of this call. Use the GetExitCodeProcess function to retrieve a */
	                                    /*process's exit value.  If the function succeeds, the return value is nonzero.*/

	  if( bret ==  NULL)
	 {
	    cout<<"Error: unable to  terminate process"<<endl;
		return FALSE;
	 }

}

BOOL HardwareInfo()
{
	SYSTEM_INFO siSysInfo;/*creting obj of SYSTEM_INFO stucture,which contains all information of current system*/
	GetSystemInfo(&siSysInfo);/*API through which getting all hardware info,sending empty obj in which all info will filled */

	cout<<"OEM ID:"<<siSysInfo.dwOemId<<endl;/*no longer in general use  member that is retained for compatibility.*/
						/* Applications should use the wProcessorArchitecture branch of the union. (ARM,x64 ARM or Intel,x86)*/

	cout<<"Number of processors:"<<siSysInfo.dwNumberOfProcessors<<endl;/* The number of logical processors in the current system*/
	cout<<"Page Size:"<<siSysInfo.dwPageSize<<endl;/* page size of current system*/
	cout<<"Processor type:"<<siSysInfo.dwProcessorType<<endl;/* Use the wProcessorArchitecture, wProcessorLevel, and */
															/*wProcessorRevision members to determine the type of processor. */
	cout<<"Minimum application address  :"<<siSysInfo.lpMinimumApplicationAddress<<endl;/* A pointer to the lowest memory address*/
																		/*accessible to applications and dynamic-link libraries (DLLs*/
	cout<<"Maximum application address  :"<<siSysInfo.lpMaximumApplicationAddress<<endl;/* A pointer to the highest memory address */
																/*accessible to applications and DLLs.*/
	cout<<"Active processor mask  :"<<siSysInfo.dwActiveProcessorMask<<endl;/* A mask representing the set of processors configured */
																			/*into the system. Bit 0 is processor 0; bit 31 is processor 31.*/

	return TRUE;
}

void DisplayHelp()
{
	
	cout<<"ps  :Display all information of process"<<endl;
	cout<<"ps-t  :Display all information about threads"<<endl;
	cout<<"ps-d  :Display all information about DLL"<<endl;
	cout<<"cls  :clear the content of consol"<<endl;
	cout<<"log  :creates log of current running process onc drive"<<endl;
	cout<<"readlog  :Display information of specified log file"<<endl;
	cout<<"sysinfo  :Display current hardware configuration"<<endl;
	cout<<"search  :Search and  Display  information of specific running process"<<endl;
	cout<<"exit  : Terminates ProcMon"<<endl;

}


int main(int argc,char *argv[])
{
	BOOL bret;
	char *ptr =NULL;
	ProcessInfo * ppobj=NULL;
	char command[4][80],str[80];
    int count,min,date,month,hr;

	while(1)
	{
	   fflush(stdin);
	   strcpy_s(str,"");
	   cout<<endl<<"ProcMon: >";
	   
	   fgets(str,80,stdin);/*command given by user is taken here.*/
	   count=sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);/*that command may contain 2/3 words.for dividing that words*/

	   if(count==1)
	   {
	   
	       if(_stricmp(command[0],"ps") == 0)
		   {
		     ppobj= new ProcessInfo();/*creating obj of processinfo class */
			 bret=ppobj->ProcessDisplay("-a");/*displaying info of given paramter name i.e. info of all process,*/
			                                  /*returns value if successful or 0 if fails*/
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: Unable to display process"<<endl;
			 }

			 delete ppobj;
		   }
	   
		   else if(_stricmp(command[0],"log") == 0)/*creating log file */
		   {
		     ppobj= new ProcessInfo();
			 bret=ppobj->ProcessLog();/*displayong info of log file.return true value if function succeded*/
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: Unable to create log file"<<endl;
			 }

			 delete ppobj;
		   }
	   
		   else if(_stricmp(command[0],"sysinfo") == 0)/*for getting sysinfo*/
		   {
		     bret=HardwareInfo();/*calling naked function,return true if execute succesfully*/
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: Unable to get hardware info"<<endl;
			 }

			 cout<<"Hardware information of current system is: "<<endl;
		   }

		    else if(_stricmp(command[0],"readlog") == 0)
		   {
			   ProcessInfo * ppobj;
			   ppobj= new ProcessInfo();/*initializing ppobj object by constuctor of ProcessInfo */
			   
			   cout<<"Enter log file details: "<<endl;
			   cout<<"Hour: ";
			   cin>>hr;
			   cout<<"Minutes: ";
			   cin>>min;
			   cout<<"Date: ";
			   cin>>date;
			   cout<<"Month: ";
			   cin>>month;


		     bret=ppobj->ReadLog(hr,min,date,month);/*calling member function readlog*/
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: Unable to read specified log file"<<endl;
			 }

			 
		   }

		    else if(_stricmp(command[0],"clear") == 0)/*if user gives clear command*/
		   {
		      system("cls"); /*tell system to handle command by giving it through comment */
			  continue;
		   }

		   else if(_stricmp(command[0],"help") == 0)
		   {
		      DisplayHelp();/*displaying all commmamds with its meaning which we are using/ proving in project*/
			  continue;
		   }

			else if(_stricmp(command[0],"exit") == 0)
		   {
		      cout<<endl<<"Terminating the ProcMon"<<endl;
			  break;
		   }
			else
			{
			  cout<<endl<<"ERROR: Command not found"<<endl;
			  continue;
			}
		   
	   }

	   else if(count == 2)
	   {

	       if(_stricmp(command[0],"ps") == 0)
		   {
		     ppobj= new ProcessInfo();
			 bret=ppobj->ProcessDisplay(command[1]);/*displaying information of all files given as parameter,stored as command[1] */
                                                    /*if this function executes succesfully return 1*/
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: Unable to display process information"<<endl;
			 }

			 delete ppobj;
		   }

		   else if(_stricmp(command[0],"search") == 0) /* search info of running process*/
		   {
		     ppobj= new ProcessInfo();
			 bret=ppobj->PrecessSearch(command[1]);/*calling function with its parameter ,given by user as a file name*/
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: There is no such process"<<endl;
			 }

			 delete ppobj;
			 continue;
		   }
	     
		   else if(_stricmp(command[0],"kill") == 0)
		   {
		     ppobj= new ProcessInfo();
			 bret=ppobj->KillProcess(command[1]);
		     if(bret == FALSE)
			 {			   
			   cout<<"ERROR: There is no such process"<<endl;
			 }
			 else
			  cout<<command[1]<<" ...Terminated succesfully..!"<<endl;


			 delete ppobj;
			 continue;
		   }
	  
	   }
	   else
	   {
	        cout<<"ERROR : COMMAND not found !!!!"<<endl;
			continue;
	   }
	      
	   
	 }
 	
	 return 0;
}
