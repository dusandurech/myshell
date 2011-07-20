
#ifndef HISTORY_H

#define HISTORY_H

extern int history_init();
extern int history_load();
extern int history_save();
extern void history_backup_current_line(const char *str_command);
extern void history_add(const char *str_command);
extern int history_up();
extern int history_down();
extern char* history_get_select();
extern int history_quit();

#endif
