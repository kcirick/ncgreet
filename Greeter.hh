#ifndef GREETER_HH
#define GREETER_HH

#include <ncurses.h>
#include <form.h>
#include <menu.h>

class Greeter {
   public:
      Greeter() { };
      ~Greeter() { };

      void init_greeter(Configurator*, string);
      void process(Authenticator *);
      void clean_up();

      void draw_topbar();

   private:
      string hostname;
      string uname;
      string passwd;
      string session;

      WINDOW *topbar;
      WINDOW *main_window;
      WINDOW *messagebar;

      WINDOW *win_form;
      FORM *form;
      FIELD **fields;

      Configurator* configurator;

      int nsessions;
      int icursession;
      vector<string> session_names;
      vector<string> session_cmds;

      //unsigned int wh = 0, ww = 0;  // window height/width

      //--- functions
      void create_main_window();

      //void create_session_menu();
      //void delete_menu(WINDOW*, MENU*, ITEM**, int);
      //void scroll_menu(WINDOW*, MENU*);

      void scroll_sessions(enum ScrollDirection);
      void clear_fields();

      bool try_authenticate(Authenticator*, string, string, string);
};

#endif
