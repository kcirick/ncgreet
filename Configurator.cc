#include <fstream>
#include <dirent.h>
#include <string>
#include <vector>

#include "Globals.hh"
#include "Configurator.hh"

//------------------------------------------------------------------------
string
Configurator::read_last_session()
{
   ifstream lsfile(last_session_file.data());
   if(!lsfile.is_open()) return string();

   string line;
   string last_session;
   while(getline(lsfile, line)){
      last_session = line;
   }
   lsfile.close();
   return last_session;
}

void
Configurator::write_last_session(string lastsession)
{
   ofstream lsfile(last_session_file.data());
   if(!lsfile.is_open()) return;
   
   lsfile << lastsession;

   lsfile.close();
}

int
Configurator::get_sessions() 
{
   nsessions = 0;
   struct dirent *dp;
   DIR *dfd;

   if((dfd = opendir(session_dir.c_str()))==NULL) {
      return 0;
   }

   string filename_qfd;

   while((dp = readdir(dfd)) != NULL) {
      if((dp->d_name)[0] == '.') continue;

      filename_qfd = string(session_dir + "/" + dp->d_name);
      
      ifstream sessionfile(filename_qfd.data());
      string line;
      if(!sessionfile.is_open()) continue;

      while(getline(sessionfile, line)) {
         if(line.empty() || line[0]=='\n' || line[0]=='#' || line[0]=='[') continue;

         string id = trimString(getToken(line, '='));
         string value = trimString(line);

         if(id == "Name") session_names.push_back(value);
         if(id == "Exec") session_cmds.push_back(value);
      }
      nsessions++;
      sessionfile.close();
   }
   return nsessions;
}

//------------------------------------------------------------------------
void
Configurator::set_defaults()
{
   poweroff_cmd = "systemctl poweroff -i";
   reboot_cmd   = "systemctl reboot -i";

   session_dir = "/usr/local/share/wayland-sessions";
   last_session_file = "/var/lib/greeter/ncgreet_lastsession";
}

void
Configurator::read_config(string filename)
{
   set_defaults();

   ifstream configfile(filename.data());
   string line;
   if(configfile.is_open()){ 
      while(getline(configfile, line)){
         if(line.empty() || line[0]=='#') continue;

         string id = trimString(getToken(line, '='));
         string value = trimString(line);

         if(id == "poweroff_cmd")   poweroff_cmd = value;
         if(id == "reboot_cmd")     reboot_cmd = value;

         if(id == "session_dir")       session_dir = value;
         if(id == "last_session_file") last_session_file = value;
      }
   }

   // get sessions
   get_sessions();
}
