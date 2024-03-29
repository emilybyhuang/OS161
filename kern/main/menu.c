 /* In-kernel menu and command dispatcher.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/limits.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <syscall.h>
#include <uio.h>
#include <vfs.h>
#include <sfs.h>
#include <test.h>
#include "opt-synchprobs.h"
#include "opt-sfs.h"
#include "opt-net.h"

#define _PATH_SHELL "/bin/sh"

#define MAXMENUARGS  16

int dbflagsMenuOn = 0;
u_int32_t dbflags = 0x0;

void
getinterval(time_t s1, u_int32_t ns1, time_t s2, u_int32_t ns2,
	    time_t *rs, u_int32_t *rns)
{
	if (ns2 < ns1) {
		ns2 += 1000000000;
		s2--;
	}

	*rns = ns2 - ns1;
	*rs = s2 - s1;
}

////////////////////////////////////////////////////////////
//
// Command menu functions 

/*
 * Function for a thread that runs an arbitrary userlevel program by
 * name.
 *
 * Note: this cannot pass arguments to the program. You may wish to 
 * change it so it can, because that will make testing much easier
 * in the future.
 *
 * It copies the program name because runprogram destroys the copy
 * it gets by passing it to vfs_open(). 
 */
static
void
cmd_progthread(void *ptr, unsigned long nargs)
{
	char **args = ptr;
	char progname[128];
	int result;

	assert(nargs >= 1);

	if (nargs > 2) {
		kprintf("Warning: argument passing from menu not supported\n");
	}

	/* Hope we fit. */
	assert(strlen(args[0]) < sizeof(progname));

	strcpy(progname, args[0]);

	result = runprogram(progname);
	if (result) {
		kprintf("Running program %s failed: %s\n", args[0],
			strerror(result));
		return;
	}

	/* NOTREACHED: runprogram only returns on error. */
}

/*
 * Common code for cmd_prog and cmd_shell.
 *
 * Note that this does not wait for the subprogram to finish, but
 * returns immediately to the menu. This is usually not what you want,
 * so you should have it call your system-calls-assignment waitpid
 * code after forking.
 *
 * Also note that because the subprogram's thread uses the "args"
 * array and strings, until you do this a race condition exists
 * between that code and the menu input code.
 */
static
int
common_prog(int nargs, char **args)
{
	int result;
	struct thread * thread;

#if OPT_SYNCHPROBS
	kprintf("Warning: this probably won't work with a "
		"synchronization-problems kernel.\n");
#endif

	result = thread_fork(args[0] /* thread name */,
			args /* thread arg */, nargs /* thread arg */,
			cmd_progthread, &thread);
	if (result) {
		kprintf("thread_fork failed: %s\n", strerror(result));
		return result;
	}

	return thread_join(thread);
}

/*
 * Command for running an arbitrary userlevel program.
 */
static
int
cmd_prog(int nargs, char **args)
{
	if (nargs < 2) {
		kprintf("Usage: p program [arguments]\n");
		return EINVAL;
	}

	/* drop the leading "p" */
	args++;
	nargs--;

	return common_prog(nargs, args);
}

/*
 * Command for starting the system shell.
 */
static
int
cmd_shell(int nargs, char **args)
{
	(void)args;
	if (nargs != 1) {
		kprintf("Usage: s\n");
		return EINVAL;
	}

	args[0] = (char *)_PATH_SHELL;

	return common_prog(nargs, args);
}


/*
 * Command for changing directory.
 */
static
int
cmd_chdir(int nargs, char **args)
{
	if (nargs != 2) {
		kprintf("Usage: cd directory\n");
		return EINVAL;
	}

	return vfs_chdir(args[1]);
}

/*
 * Command for printing the current directory.
 */
static
int
cmd_pwd(int nargs, char **args)
{
	char buf[PATH_MAX+1];
	struct uio ku;
	int result;

	(void)nargs;
	(void)args;

	mk_kuio(&ku, buf, sizeof(buf)-1, 0, UIO_READ);
	result = vfs_getcwd(&ku);
	if (result) {
		kprintf("vfs_getcwd failed (%s)\n", strerror(result));
		return result;
	}

	/* null terminate */
	buf[sizeof(buf)-1-ku.uio_resid] = 0;

	/* print it */
	kprintf("%s\n", buf);

	return 0;
}

/*
 * Command for running sync.
 */
static
int
cmd_sync(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	vfs_sync();

	return 0;
}

/*
 * Command for doing an intentional panic.
 */
static
int
cmd_panic(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	panic("User requested panic\n");
	return 0;
}

/*
 * Command for shutting down.
 */
static
int
cmd_quit(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	vfs_sync();
	sys_reboot(RB_POWEROFF);
	thread_exit();
	return 0;
}

/*
 * Command for mounting a filesystem.
 */

/* Table of mountable filesystem types. */
static const struct {
	const char *name;
	int (*func)(const char *device);
} mounttable[] = {
#if OPT_SFS
	{ "sfs", sfs_mount },
#endif
	{ NULL, NULL }
};

static
int
cmd_mount(int nargs, char **args)
{
	char *fstype;
	char *device;
	int i;

	if (nargs != 3) {
		kprintf("Usage: mount fstype device:\n");
		return EINVAL;
	}

	fstype = args[1];
	device = args[2];

	/* Allow (but do not require) colon after device name */
	if (device[strlen(device)-1]==':') {
		device[strlen(device)-1] = 0;
	}

	for (i=0; mounttable[i].name; i++) {
		if (!strcmp(mounttable[i].name, fstype)) {
			return mounttable[i].func(device);
		}
	}
	kprintf("Unknown filesystem type %s\n", fstype);
	return EINVAL;
}

static
int
cmd_unmount(int nargs, char **args)
{
	char *device;

	if (nargs != 2) {
		kprintf("Usage: unmount device:\n");
		return EINVAL;
	}

	device = args[1];

	/* Allow (but do not require) colon after device name */
	if (device[strlen(device)-1]==':') {
		device[strlen(device)-1] = 0;
	}

	return vfs_unmount(device);
}

/*
 * Command to set the "boot fs". 
 *
 * The boot filesystem is the one that pathnames like /bin/sh with
 * leading slashes refer to.
 *
 * The default bootfs is "emu0".
 */
static
int
cmd_bootfs(int nargs, char **args)
{
	char *device;

	if (nargs != 2) {
		kprintf("Usage: bootfs device\n");
		return EINVAL;
	}

	device = args[1];

	/* Allow (but do not require) colon after device name */
	if (device[strlen(device)-1]==':') {
		device[strlen(device)-1] = 0;
	}

	return vfs_setbootfs(device);
}

static
int
cmd_kheapstats(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	kheap_printstats();
	
	return 0;
}

////////////////////////////////////////
//
// Menus.

static
void
showmenu(const char *name, const char *x[])
{
	int ct, half, i;

	kprintf("\n");
	kprintf("%s\n", name);
	
	for (i=ct=0; x[i]; i++) {
		ct++;
	}
	half = (ct+1)/2;

	for (i=0; i<half; i++) {
		kprintf("    %-36s", x[i]);
		if (i+half < ct) {
			kprintf("%s", x[i+half]);
		}
		kprintf("\n");
	}

	kprintf("\n");
}

static const char *opsmenu[] = {
	"[s]       Shell                     ",
	"[p]       Other program             ",
	"[dbflags] Debug flags               ",
	"[mount]   Mount a filesystem        ",
	"[unmount] Unmount a filesystem      ",
	"[bootfs]  Set \"boot\" filesystem     ",
	"[pf]      Print a file              ",
	"[cd]      Change directory          ",
	"[pwd]     Print current directory   ",
	"[sync]    Sync filesystems          ",
	"[panic]   Intentional panic         ",
	"[q]       Quit and shut down        ",
	NULL
};

static const char *debugFlagsMenu[] = {
         "[df 1 on/off]        DB_LOCORE       ",
         "[df 2 on/off]        DB_SYSCALL      ",
         "[df 3 on/off]        DB_INTERRUPT    ",
         "[df 4 on/off]        DB_DEVICE       ",
         "[df 5 on/off]        DB_THREADS      ",
         "[df 6 on/off]        DB_VM           ",
         "[df 7 on/off]        DB_EXEC         ",
         "[df 8 on/off]        DB_VFS          ",
         "[df 9 on/off]        DB_SFS          ",
         "[df 10 on/off]       DB_NET          ",
         "[df 11 on/off]       DB_NETFS        ",
         "[df 12 on/off]       DB_KMALLOC      ",
         NULL
};

static
int
cmd_opsmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 operations menu", opsmenu);
	return 0;
}

/*
*/
static
int
cmd_dbflags(int nargs, char **args)
{
	kprintf("In cmd_dbflags\n");
	//time_t beforesecs, aftersecs, secs;
	//u_int32_t beforensecs, afternsecs, nsecs;
//	int position;
// 	char dfBuf[64];
//	char *savePtr = dfBuf;
        (void)nargs;
        (void)args;
	showmenu("OS/161 Debug flags", debugFlagsMenu);
	kprintf("Current value of dbflags is 0x%-4x\n",dbflags);
	//kprintf("OS/161 kernel [? for menu]:");
	
	//should separate these
	return 0;
}

static int cmd_df(int nargs, char **args){
	/*kprintf("nargs: %d\n",nargs);
	kprintf("args: %s\n",*args);
	args++;
	kprintf("args: %s\n",*args);
	args++;
	kprintf("args: %s\n",*args);
	*/
	
	//invalid num of args for df command
	if(nargs != 3)return -1;

	//The "words" enterred will be in args
	//kprintf("In cmd_df\n");
	char dfStr[] = "df";
	char onStr[] = "on";
	char offStr[] = "off";
	int position = -1;
	 if(strcmp(dfStr,*args) == 0){
		args++;
         	position = atoi(*args);
		if(position <= 12 && position >= 1){
			args++;
                        if(strcmp(*args,onStr)==0){
                                dbflags |= 1 << (position-1);
				return 0;
			}else if(strcmp(*args,offStr)==0){
				//You can't turn off something that's not turned on...that case is resolved here tho 0 & 0 is still 0
                                dbflags &= ~(1 << (position-1));
				return 0;
                        }else{
				//if the last word enterred is not on nor off
				kprintf("Usage: df nr on/off\n");
				return 0;
			}
                                         /*gettime(&aftersecs, &afternsecs);
                                         getinterval(beforesecs, beforensecs,
                                         aftersecs, afternsecs,
                                         &secs, &nsecs);

                                         kprintf("Operation took %lu.%09lu seconds\n",
                                         (unsigned long) secs,
                                         (unsigned long) nsecs);
                                        */
					return 0;
                                 	
		}else{
			kprintf("Usage: df nr on/off\n");
			return 0;
		}
	}else{
		kprintf("Usage: df nr on/off\n");
		 return 0;
	}
}
/*
static int cmd_df(int nargs, char **args){
	//kprintf("In cmd_df\n");
	if(dbflagsMenuOn == 1){	
	//kprintf("In while(1)\n");
		kprintf("Read input\n");
		kgets(dfBuf, sizeof(dfBuf));		
		gettime(&beforesecs, &beforensecs);
		char * thisWord;
		thisWord = strtok_r(dfBuf, " ",&savePtr);
		char dfStr[] = "df";
		char on[] = "on";
		char off[] = "off";
	
		if(strcmp(dfStr,thisWord) == 0){
			thisWord = strtok_r(savePtr, " ",&savePtr);		
			position = atoi(thisWord);
			if(position <= 12 && position >= 1){	
				thisWord = strtok_r(savePtr, " ",&savePtr);
				if(strcmp(thisWord,on) == 0 || strcmp(thisWord,off)==0){
					kprintf("Position : %d\n", position);
					//save this position
					if(strcmp(thisWord,on)==0){
						dbflags |= 1 << (position-1);
						
					}else{
						dbflags &= ~(1 << (position-1));
					}
					gettime(&aftersecs, &afternsecs);
                         		getinterval(beforesecs, beforensecs,
                                     	aftersecs, afternsecs,
                                     	&secs, &nsecs);

                         		kprintf("Operation took %lu.%09lu seconds\n",
                                 	(unsigned long) secs,
                                 	(unsigned long) nsecs);
					//kprintf("dbflags=0x%-4x\n",dbflags); 
					//kprintf("dbflags=0x%x\n", (unsigned char)dbflags);
					return 0;
				} 
			}
	
		}
		//return regardless of the validity
		kprintf("Invalid df command\n");
		return 0;
	}else{
		return -1;
	}
}
*/

static const char *testmenu[] = {
	"[at]  Array test                    ",
	"[bt]  Bitmap test                   ",
	"[qt]  Queue test                    ",
	"[km1] Kernel malloc test            ",
	"[km2] kmalloc stress test           ",
	"[tt1] Thread test 1                 ",
	"[tt2] Thread test 2                 ",
	"[tt3] Thread test 3                 ",
#if OPT_NET
	"[net] Network test                  ",
#endif
	"[sy1] Semaphore test                ",
	"[sy2] Lock test             (1)     ",
	"[sy3] CV test               (1)     ",
	"[fs1] Filesystem test               ",
	"[fs2] FS read stress        (4)     ",
	"[fs3] FS write stress       (4)     ",
	"[fs4] FS write stress 2     (4)     ",
	"[fs5] FS create stress      (4)     ",
	NULL
};

static
int
cmd_testmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 tests menu", testmenu);
	kprintf("    (1) These tests will fail until you finish the "
		"synch assignment.\n");
	kprintf("    (4) These tests will fail until you finish the "
		"file system assignment.\n");
	kprintf("\n");

	return 0;
}

static const char *mainmenu[] = {
	"[?o] Operations menu                ",
	"[?t] Tests menu                     ",
#if OPT_SYNCHPROBS
	"[1a] Cat/mouse with semaphores      ",
	"[1b] Cat/mouse with locks and CVs   ",
	"[1c] Stoplight                      ",
#endif
	"[kh] Kernel heap stats              ",
	"[q] Quit and shut down              ",
	NULL
};

static
int
cmd_mainmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 kernel menu", mainmenu);
	return 0;
}

////////////////////////////////////////
//
// Command table.

static struct {
	const char *name;
	int (*func)(int nargs, char **args);
} cmdtable[] = {
	/* menus */
	{ "?",		cmd_mainmenu },
	{ "h",		cmd_mainmenu },
	{ "help",	cmd_mainmenu },
	{ "?o",		cmd_opsmenu },
	{ "?t",		cmd_testmenu },

	/* operations */
	{ "s",		cmd_shell },
	{ "p",		cmd_prog },
	{ "dbflags",	cmd_dbflags},
	{ "mount",	cmd_mount },
	{ "unmount",	cmd_unmount },
	{ "bootfs",	cmd_bootfs },
	{ "pf",		printfile },
	{ "cd",		cmd_chdir },
	{ "pwd",	cmd_pwd },
	{ "sync",	cmd_sync },
	{ "panic",	cmd_panic },
	{ "q",		cmd_quit },
	{ "exit",	cmd_quit },
	{ "halt",	cmd_quit },
	{ "df", 	cmd_df},
#if OPT_SYNCHPROBS
	/* in-kernel synchronization problems */
	{ "1a",		catmousesem },
	{ "1b",		catmouselock },
	{ "1c",		createcars },
#endif

	/* stats */
	{ "kh",         cmd_kheapstats },

	/* base system tests */
	{ "at",		arraytest },
	{ "bt",		bitmaptest },
	{ "qt",		queuetest },
	{ "km1",	malloctest },
	{ "km2",	mallocstress },
#if OPT_NET
	{ "net",	nettest },
#endif
	{ "tt1",	threadtest },
	{ "tt2",	threadtest2 },
	{ "tt3",	threadtest3 },
	{ "sy1",	semtest },

	/* synchronization assignment tests */
	{ "sy2",	locktest },
	{ "sy3",	cvtest },

	/* file system assignment tests */
	{ "fs1",	fstest },
	{ "fs2",	readstress },
	{ "fs3",	writestress },
	{ "fs4",	writestress2 },
	{ "fs5",	createstress },

	{ NULL, NULL }
};

/*
 * Process a single command.
 */
static
int
cmd_dispatch(char *cmd)
{
//	kprintf("Command: %s\n", cmd);
//	kprintf("in cmd_dispatch");
	time_t beforesecs, aftersecs, secs;
	u_int32_t beforensecs, afternsecs, nsecs;
	char *args[MAXMENUARGS];
	int nargs=0;
	char *word;
	char *context;
	int i, result;

	for (word = strtok_r(cmd, " \t", &context);
	     word != NULL;
	     word = strtok_r(NULL, " \t", &context)) {
	//	kprintf("for loop word: %s\n, word);
		if (nargs >= MAXMENUARGS) {
			kprintf("Command line has too many words\n");
			return E2BIG;
		}
		args[nargs++] = word;
	}

	if (nargs==0) {
		return 0;
	}
	//kprintf("Word: %s\n",word);
	for (i=0; cmdtable[i].name; i++) {
		if (*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
			assert(cmdtable[i].func!=NULL);

			gettime(&beforesecs, &beforensecs);
			//kprintf("NARGS: %d, args: %s\n",nargs, args);
			result = cmdtable[i].func(nargs, args);

			gettime(&aftersecs, &afternsecs);
			getinterval(beforesecs, beforensecs,
				    aftersecs, afternsecs,
				    &secs, &nsecs);

			kprintf("Operation took %lu.%09lu seconds\n",
				(unsigned long) secs,
			(unsigned long) nsecs);
			
			return result;
		}
	}

	//Check for the dbflags menu as well

	kprintf("%s: Command not found\n", args[0]);
	return EINVAL;
}

/*
 * Evaluate a command line that may contain multiple semicolon-delimited
 * commands.
 *
 * If "isargs" is set, we're doing command-line processing; print the
 * comamnds as we execute them and panic if the command is invalid or fails.
 */
static
void
menu_execute(char *line, int isargs)
{
	//kprintf("In menu execute\n");
	//kprintf("Line: %s\n", line);
	char *command;
	char *context;
	int result;

	for (command = strtok_r(line, ";", &context);
	     command != NULL;
	     command = strtok_r(NULL, ";", &context)) {

		if (isargs) {
			kprintf("OS/161 kernel: %s\n", command);
		}

		//kprintf("Command: %s\n",command);
		result = cmd_dispatch(command);
		if (result) {
			kprintf("Menu command failed: %s\n", strerror(result));
			if (isargs) {
				panic("Failure processing kernel arguments\n");
			}
		}
	}
}

/*
 * Command menu main loop.
 *
 * First, handle arguments passed on the kernel's command line from
 * the bootloader. Then loop prompting for commands.
 *
 * The line passed in from the bootloader is treated as if it had been
 * typed at the prompt. Semicolons separate commands; spaces and tabs
 * separate words (command names and arguments).
 *
 * So, for instance, to mount an SFS on lhd0 and make it the boot
 * filesystem, and then boot directly into the shell, one would use
 * the kernel command line
 *
 *      "mount sfs lhd0; bootfs lhd0; s"
 */

void
menu(char *args)
{
	char buf[64];

	menu_execute(args, 1);

	while (1) {
		kprintf("OS/161 kernel [? for menu]: ");
		kgets(buf, sizeof(buf));
		menu_execute(buf, 0);
	}
}
