#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "Globals.hh"
#include "Authenticator.hh"
#include "Configurator.hh"
#include "Greeter.hh"

#define TAB 9
#define ENTER 10
#define ESCAPE 27

//------------------------------------------------------------------------
void 
Greeter::draw_topbar() 
{
   time_t now = time(&now);
   struct tm *now_tm = localtime(&now);
   char time_str[64];

   // start fresh
   werase(topbar);

   wmove(topbar, 0, 2);
   waddstr(topbar, "F1:Power-off");

   wmove(topbar, 0, 17);
   waddstr(topbar, "F2:Reboot");

   wmove(topbar, 0, COLS-10);
   waddstr(topbar, "F3:Clear");

   strftime(time_str, 64, (configurator->get_timeformat()).c_str(), now_tm);
   int len_time = strlen(time_str);
   mvwprintw(topbar, 0, (COLS-len_time)/2, time_str); 

   wrefresh(topbar);
   refresh();
}

void
Greeter::create_main_window()
{
   int rows = 9;
   int cols = 50;

   main_window = subwin(stdscr, rows, cols, (LINES-rows)/2, (COLS-cols)/2);
   wbkgd(main_window, COLOR_PAIR(1));
   box(main_window, 0, 0);
   mvwprintw(main_window, 0, 2, " %s Login ", hostname.c_str());
   refresh();

   int nfields = 3;
   fields = (FIELD**)malloc(sizeof(FIELD*) * (nfields+1));

   std::string stitle;
   fields[0] = new_field(1, 25, 0, 12, 0, 0);
   if(!strlen(session.c_str()))  stitle = "<- Session ->";
   else                          stitle = "<- " + session + " ->";
   set_field_buffer(fields[0], 0, stitle.c_str());
   field_opts_on(fields[0], O_ACTIVE);
   field_opts_off(fields[0], O_EDIT);
   fields[1] = new_field(1, 25, 2, 12, 0, 0);
   set_field_buffer(fields[1], 0, "");
   field_opts_on(fields[1], O_ACTIVE);
   field_opts_on(fields[1], O_EDIT);
   fields[2] = new_field(1, 25, 4, 12, 0, 0);
   set_field_buffer(fields[2], 0, "");
   field_opts_on(fields[2], O_ACTIVE);
   field_opts_on(fields[2], O_EDIT);
   field_opts_off(fields[2], O_PUBLIC);
   for(int i=0; i<nfields; i++){
      set_field_fore(fields[i], COLOR_PAIR(1));
      set_field_back(fields[i], COLOR_PAIR(1));
   }
   
   fields[3] = NULL;

   form = new_form(fields);
   win_form = derwin(main_window, rows-1, cols-1, 1, 1);
   set_form_win(form, win_form);
   set_form_sub(form, derwin(win_form, form->rows+1, form->cols+1, 1, 1));


   pos_form_cursor(form);

   assert(post_form(form) == E_OK);

   mvwprintw(main_window, 4, 2, "Username: "); 
   mvwprintw(main_window, 6, 2, "Password: "); 

   //refresh();
   wrefresh(main_window);
   wrefresh(win_form);
}

/*
void
Greeter::create_session_menu()
{
   int n_choices = 2;
   session_items = (ITEM**)malloc(sizeof(ITEM*) * (n_choices+1));
   session_items[0] = new_item("SimpleWC", "");
   session_items[1] = new_item("DWL", "");
   session_items[2] = NULL; 

   session_menu = new_menu((ITEM**)session_items);
   session_menu_win = newwin(6, 20, 1, 20);
   keypad(session_menu_win, TRUE);

   set_menu_win(session_menu, session_menu_win);
   set_menu_sub(session_menu, derwin(session_menu_win, 4, 15, 1, 1));

   set_menu_mark(session_menu, "* ");
   
   box(session_menu_win, 0, 0);

   post_menu(session_menu);
   wrefresh(session_menu_win);
}

void
Greeter::delete_menu(WINDOW* window, MENU* menu, ITEM** items, int nitems)
{
   unpost_menu(menu);   
   free_menu(menu);
   for(int i=0; i<nitems; i++)
      free_item(items[i]);
   delwin(window);
   //draw_topbar();    // refresh topbar
   string stitle = session + " --->";
   set_field_buffer(fields[0], 0, stitle.c_str());
   refresh();
}

void 
Greeter::scroll_menu(WINDOW* window, MENU* menu)
{
   int key;
   while ((key=getch()) != ESCAPE) {
      switch(key){
         case KEY_DOWN:
            menu_driver(menu, REQ_DOWN_ITEM);
            break;
         case KEY_UP:
            menu_driver(menu, REQ_UP_ITEM);
            break;
         case ENTER:
            session = item_name(current_item(menu));
            wprintw(messagebar, "[*] Menu Selected: %s", session.c_str());
            return;
            break;
      }
      wrefresh(window);
   }
}
*/

void
Greeter::scroll_sessions(enum ScrollDirection dir)
{
   if(nsessions==0) return;

   icursession = dir==PREV ? icursession-1 : icursession+1;
   // wrap around
   if(icursession<0) icursession = nsessions-1;
   if(icursession>(nsessions-1)) icursession = 0;

   //wprintw(messagebar, "[*] nsessions: %d, icursession: %d", nsessions, icursession);
   session = session_names[icursession];
   
   //wprintw(messagebar, "session name = %s", session.c_str());
   string stitle = "<- " + session + " ->";
   set_field_buffer(fields[0], 0, stitle.c_str());
}

void
Greeter::clear_fields()
{
   set_current_field(form, fields[1]);
   form_driver(form, REQ_CLR_FIELD);
   set_current_field(form, fields[2]);
   form_driver(form, REQ_CLR_FIELD);
   set_current_field(form, fields[1]);
}

bool 
Greeter::try_authenticate(Authenticator* authenticator, 
      string uname, string passwd, string session)
{
   werase(messagebar);

   //wprintw(messagebar, "[*] Username: %s / Password: %s / Session: %s", 
   //      uname.c_str(), passwd.c_str(), session.c_str());

   bool auth_state = false;
   auth_state = authenticator->authenticate(uname.c_str(), passwd.c_str());
   if(!auth_state) {
      wprintw(messagebar, string("[*] " + authenticator->getErrorMsg()).c_str()); 
      return auth_state; // if unsuccessful return false
   }
   
   // Find out the session command
   string session_cmd;
   if(nsessions==0){
      session_cmd = "/bin/bash";
   } else {
      for(int i=0; i<nsessions; i++) {
         if(session_names[i]==session){
            session_cmd = session_cmds[i];
            break;
         }
      }
   }

   bool session_state = false;
   session_state = authenticator->start_session(session_cmd.c_str());
   if(!session_state) {
      wprintw(messagebar, string("[*] " + authenticator->getErrorMsg()).c_str());
      return session_state;
   }

   wprintw(messagebar, "[*] Authenticated!");
   configurator->write_last_session(session);
   return true;
}

//------------------------------------------------------------------------
void 
Greeter::init_greeter(Configurator* configurator, string hostname) 
{
   // set hostname
   this->hostname = hostname;

   //init screen and sets up screen
   initscr();
   
   //set up colours
   start_color();
   //init_pair(1, COLOR_WHITE, COLOR_BLACK);
   //init_pair(2, COLOR_BLACK, COLOR_WHITE);
   init_pair(1, COLOR_BLACK, COLOR_BLUE);
   init_pair(2, COLOR_BLUE, COLOR_BLACK);
   
   // Get sessions from configurator
   nsessions = configurator->get_nsessions();
   if(nsessions>0) {
      session_names = configurator->get_session_names();
      session_cmds = configurator->get_session_cmds();
      session = configurator->read_last_session();
      if(session.empty()){
         icursession = 0;
         session = session_names[icursession];
      } else {
         for(int i=0; i<nsessions; i++){
            if(session==session_names[i]) icursession = i;
         }
      }
   }
   this->configurator = configurator;

   //curs_set(0);
   noecho();
   cbreak();
   keypad(stdscr, TRUE);

   bkgd(COLOR_PAIR(1));
   
   // draw topbar
   topbar = subwin(stdscr, 1, COLS, 0, 0);
   wbkgd(topbar, COLOR_PAIR(2));
   draw_topbar();

   // main window
   create_main_window();
   set_current_field(form, fields[1]);

   // message bar
   messagebar = subwin(stdscr, 1, COLS-2, LINES-2, 1);
   //wbkgd(messagebar, COLOR_PAIR(2));
}

void 
Greeter::process(Authenticator* authenticator) 
{
   int key;
   string cmd;
   while((key = getch())) {

      // Erase message bar and wait for the user input
      werase(messagebar);

      switch (key) {
         case KEY_F(1):
            cmd = configurator->get_poweroff_cmd();
            wprintw(messagebar, "[*] Power-off: %s", cmd.c_str());
            spawn(cmd);
            break;
         case KEY_F(2):
            cmd = configurator->get_reboot_cmd();
            wprintw(messagebar, "[*] Reboot: %s", cmd.c_str());
            spawn(cmd);
            break;
         case KEY_F(3):
            wprintw(messagebar, "[*] Clear");
            clear_fields();
            //redrawwin(topbar);
            redrawwin(main_window);
            //redrawwin(messagebar);
            break;
         case KEY_DOWN:
            if(form->current == fields[form->maxfield-1]) break;
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_END_LINE);
            break;
         case TAB:
            if(form->current == fields[form->maxfield-1])   break;
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_END_LINE);
            break;
         case KEY_UP:
            if(form->current == fields[0]) break;
            form_driver(form, REQ_PREV_FIELD);
            form_driver(form, REQ_END_LINE);
            break;
         case KEY_LEFT:
            if(form->current == fields[0]) {
               scroll_sessions(PREV);
            }
            break;
         case KEY_RIGHT:
            if(form->current == fields[0]) {
               scroll_sessions(NEXT);
            }
            break;
         case KEY_BACKSPACE:
            form_driver(form, REQ_END_LINE);
            if(strlen(trimString(field_buffer(form->current, 0)).c_str())>0)
               form_driver(form, REQ_DEL_PREV);
            break;
         case ENTER:
            if(form->current == fields[form->maxfield-1]){
               form_driver(form, REQ_END_LINE);
               uname    = trimString(field_buffer(fields[1], 0));
               passwd   = trimString(field_buffer(fields[2], 0));
               clear_fields();
               if(try_authenticate(authenticator, uname, passwd, session)) 
                  return; // Exit the loop
            } else 
               form_driver(form, REQ_NEXT_FIELD);
            break;
         default:
            form_driver(form, REQ_END_LINE);
            if(strlen(trimString(field_buffer(form->current, 0)).c_str())<20)
               form_driver(form, key);
            else
               wprintw(messagebar, "[E] max characters reached");
            break;
      }

      // refresh
      touchwin(stdscr);
      refresh();
      wrefresh(messagebar);
      wrefresh(main_window);
      wrefresh(win_form);
   }
}

void 
Greeter::clean_up() 
{
   // delete individual windows
   delwin(topbar);
   delwin(messagebar);

   // delete form
   unpost_form(form);   
   free_form(form);
   for(int i=0; fields[i]!=NULL; i++)
      free_field(fields[i]);

   delwin(win_form);
   delwin(main_window);

   //deallocates memory and ends ncurses
   endwin();
}
