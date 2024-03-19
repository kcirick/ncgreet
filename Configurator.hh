#ifndef CONFIGURATOR_HH
#define CONFIGURATOR_HH

class Configurator {
   public:
      Configurator() { };
      ~Configurator() { };

      void read_config(string);


      string read_last_session();
      void write_last_session(string);

      int get_nsessions() { return nsessions; }
      vector <string> get_session_names() { return session_names; }
      vector <string> get_session_cmds()  { return session_cmds; }

      string get_poweroff_cmd() { return poweroff_cmd; };
      string get_reboot_cmd() { return reboot_cmd; };
      string get_timeformat() { return timeformat; };

   private:
      int nsessions;
      vector <string> session_names;
      vector <string> session_cmds;

      string poweroff_cmd;
      string reboot_cmd;
      string session_dir;
      string last_session_file;
      string timeformat;

      //--- functions
      void set_defaults();
      int get_sessions();
};

#endif
