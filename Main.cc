#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <signal.h>
#include <time.h>

#include "Globals.hh"
#include "Configurator.hh"
#include "Authenticator.hh"
#include "Greeter.hh"

static bool debug = false;
static timer_t tid = 0;

//--- Helper functions ---------------------------------------------------
string 
trimString(string str) 
{
   str.erase(0, str.find_first_not_of(' '));
   str.erase(str.find_last_not_of(' ')+1);
   return str;
}

string 
getToken(string &str, char delim) 
{
   string token = str.substr(0, str.find_first_of(delim));
   str = str.substr(str.find_first_of(delim)+1);
   return token;
}

void spawn(string cmd) {
   char* sh = NULL;
   if(!(sh=getenv("SHELL"))) sh = (char*)"/bin/sh";

   if(fork()==0){
      dup2(STDERR_FILENO, STDOUT_FILENO);
      setsid();
      execl(sh, sh, "-c", cmd.c_str(), (char*)NULL);
   }
}

//--- Timer functions for the clock --------------------------------------
void 
timer_handler(union sigval timer_data)
{
   Greeter* greeter = (Greeter*)timer_data.sival_ptr;
   greeter -> draw_topbar();
   //return;
}

void
create_timer(Greeter* greeter) 
{
   struct sigevent sev;
   memset(&sev, 0, sizeof(struct sigevent));

   sev.sigev_notify = SIGEV_THREAD;
   sev.sigev_notify_function = &timer_handler;
   sev.sigev_value.sival_ptr = greeter;

   if(timer_create(CLOCK_REALTIME, &sev, &tid) != 0){
      perror ("time_create() did not return success\n");
      return;
   }

   struct itimerspec its;
   memset(&its, 0, sizeof(struct itimerspec));
   its.it_value.tv_sec  = 1;
   its.it_value.tv_nsec = 0;
   its.it_interval.tv_sec  = 30;
   its.it_interval.tv_nsec = 0;

   timer_settime(tid, 0, &its, NULL);
}

//------------------------------------------------------------------------
int
main(int argc, char ** argv)
{
   string config_file;
   // Parse arguments
   for(int i=1; i<argc; i++) {
      string iarg = argv[i];
      if(iarg=="--debug") debug = true;
      if(iarg=="--config" && ((i+1)<argc)) 
         config_file = argv[++i];
   }
   if(config_file.empty())
      config_file = "/etc/greetd/ncgreet_configrc";

   Configurator *configurator = new Configurator();
   configurator -> read_config(config_file);

   char my_hostname[32];
   gethostname(my_hostname, 32);

   Greeter *greeter = new Greeter();
   greeter -> init_greeter(configurator, string(my_hostname));

   // create timer
   create_timer(greeter);
   
   Authenticator *authenticator = new Authenticator();
   greeter -> process(authenticator);

   // clean up
   timer_delete(tid);
   greeter -> clean_up();

   return EXIT_SUCCESS;
}
