/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      System level: initialization, cleanup, etc.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 * 		+ Changes from archeide 
 */

#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/dir.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "fs.h"
#include "posixHandleTGDS.h"

//stdbool.h handles these
/*
#define TRUE	1
#define FALSE	0
*/
#include <stdbool.h>
#include "conf.h"
#include "utilsTGDS.h"

CONFIG *config[MAX_CONFIGS] = { NULL, NULL, NULL, NULL };
CONFIG *config_override = NULL;
CONFIG *config_language = NULL;
CONFIG *system_config = NULL;
CONFIG_HOOK *config_hook = NULL;
int config_installed = false;




//emuCore CFG into file setting
void save_config(CONFIG *cfg)
{
   CONFIG_ENTRY *pos;

   if (cfg) {
      if (cfg->filename) {
		 if (cfg->dirty) {
			FS_lock();
		    /* write changed data to disk */
		    FILE *f = fopen(cfg->filename, "w");
			//FIL fhandler;
			
			//if(f_open(&fhandler,cfg->filename,FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
			if(f)
			{				
				pos = cfg->head;
	
		       while (pos) {
			  	 if (pos->name) {
			    	 
					 fputs(pos->name, f);
			     	 //f_puts(pos->name,&fhandler);
					 
					 if (pos->name[0] != '['){
						fputs(" = ", f);
						//f_puts(" = ",&fhandler);
					 }
			     }
			    if (pos->data){
			       fputs(pos->data, f);
				   //f_puts(pos->data,&fhandler);
				}
			     fputs("\n", f);
			     //f_puts("\n",&fhandler);
				 
				 pos = pos->next;
		       }
		       fclose(f);
				//f_close(&fhandler);
			}
		    FS_unlock();
		  }
      }
   }
}


void save_config_file()
{
	save_config(config[0]);
}

/* destroy_config:
 *  Destroys a config structure, writing it out to disk if the contents
 *  have changed.
 */
void destroy_config(CONFIG *cfg)
{
   CONFIG_ENTRY *pos, *prev;

   if (cfg) {
      if (cfg->filename) {
		save_config(cfg);
	    free(cfg->filename);
      }

      /* destroy the variable list */
      pos = cfg->head;

      while (pos) {
	 prev = pos;
	 pos = pos->next;

	 if (prev->name)
	    free(prev->name);

	 if (prev->data)
	    free(prev->data);

	 free(prev);
      }

      free(cfg);
   }
}




/* config_cleanup:
 *  Called at shutdown time to free memory being used by the config routines,
 *  and write any changed data out to disk.
 */
void config_cleanup()
{
   CONFIG_HOOK *hook, *nexthook;
   int i;

   for (i=0; i<MAX_CONFIGS; i++) {
      if (config[i]) {
	 destroy_config(config[i]);
	 config[i] = NULL;
      }
   }

   if (config_override) {
      destroy_config(config_override);
      config_override = NULL;
   }

   if (config_language) {
      destroy_config(config_language);
      config_language = NULL;
   }

   if (system_config) {
      destroy_config(system_config);
      system_config = NULL;
   }

   if (config_hook) {
      hook = config_hook;

      while (hook) {
	 if (hook->section)
	    free(hook->section);

	 nexthook = hook->next; 
	 free(hook);
	 hook = nexthook;
      }

      config_hook = NULL;
   }

//   _remove_exit_func(config_cleanup);
   config_installed = false;
}



/* init_config:
 *  Sets up the configuration routines ready for use, also loading the
 *  default config file if the loaddata flag is set and no other config
 *  file is in memory.
 */
void init_config(int loaddata)
{
   if (!config_installed) {
//      _add_exit_func(config_cleanup);
      config_installed = true;
   }

   if (!system_config) {
      system_config = (CONFIG *)malloc(sizeof(CONFIG));
      if (system_config) {
	 system_config->head = NULL;
	 system_config->filename = NULL;
	 system_config->dirty = false;
      }
   }
}



/* get_line: 
 *  Helper for splitting files up into individual lines.
 */
int get_line(sint8 *data, int length, sint8 *name, sint8 *val)
{
   sint8 buf[256], buf2[256];
   int pos, i, j;

   for (pos=0; (pos<length) && (pos<255); pos++) {
      if ((data[pos] == '\r') || (data[pos] == '\n')) {
	 buf[pos] = 0;
	 if ((pos < length-1) && 
	     (((data[pos] == '\r') && (data[pos+1] == '\n')) ||
	      ((data[pos] == '\n') && (data[pos+1] == '\r')))) {
	    pos++;
	 }
	 pos++;
	 break;
      }

      buf[pos] = data[pos];
   }

   buf[MIN(pos,255)] = 0;

   /* skip leading spaces */
   i = 0;
   while ((buf[i]) && (isspace((int)buf[i])))
      i++;

   if (buf[i] == '[')
   {
   /* read section string */
   j = 0;
   while ((buf[i]) && (buf[i] != ']'))
      buf2[j++] = buf[i++];
    buf2[j] = 0;
    strcpy(name, buf2);
    name[strlen(name)+1] = 0;
    name[strlen(name)] = ']';
    val[0] = 0;
    return pos;
   }

   /* read name string */
   j = 0;
   while ((buf[i]) && (!isspace((int)buf[i])) && (buf[i] != '=') && (buf[i] != '#'))
      buf2[j++] = buf[i++];

   if (j) {
      /* got a variable */
      buf2[j] = 0;
      strcpy(name, buf2);

      while ((buf[i]) && ((isspace((int)buf[i])) || (buf[i] == '=')))
	 i++;

      strcpy(val, buf+i);

      /* strip trailing spaces */
      i = strlen(val) - 1;
      while ((i >= 0) && (isspace((int)val[i])))
	 val[i--] = 0;
   }
   else {
      /* blank line or comment */
      name[0] = 0;
      strcpy(val, buf);
   }

   return pos;
}



/* set_config:
 *  Does the work of setting up a config structure.
 */
#if 0
static void set_config(CONFIG **config, sint8 *data, int length, sint8 *filename)
{
   sint8 name[256];
   sint8 val[256];
   CONFIG_SECTION **prev, *p;
   CONFIG_ENTRY	**prev_e, *p_e;
   int pos;
   int	in_section = 0;

   init_config(false);

   if (*config) {
      destroy_config(*config);
      *config = NULL;
   }

   *config = malloc(sizeof(CONFIG));
   if (!(*config))
      return;

   (*config)->head = NULL;
   (*config)->dirty = false;

   if (filename) {
      (*config)->filename = malloc(strlen(filename)+1);
      if ((*config)->filename)
	 strcpy((*config)->filename, filename); 
   }
   else
      (*config)->filename = NULL;

   prev = &(*config)->head;
   pos = 0;

   while (pos < length) {
      pos += get_line(data+pos, length-pos, name, val);

      if (in_section && name[0] == '[')
	  {
	      in_section = 0;
	  }

      if (in_section)
      {
	  p_e = malloc(sizeof(CONFIG_ENTRY));
          if (!p_e)
    	    return;

          if (name[0]) {
	     p_e->name = malloc(strlen(name)+1);
	     if (p_e->name)
		strcpy(p_e->name, name);
	  }
	  else
	     p_e->name = NULL;

	  p_e->data = malloc(strlen(val)+1);
	  if (p_e->data)
	     strcpy(p_e->data, val);

	  p_e->next = NULL;
	  *prev_e = p_e;
	  prev_e = &p_e->next;

	  continue;
      }	

      p = malloc(sizeof(CONFIG_SECTION));
      if (!p)
	 return;

      p->key = NULL;

      if (name[0]) {
	 p->name = malloc(strlen(name)+1);
	 if (p->name)
	    strcpy(p->name, name);
      }
      else
	 p->name = NULL;

      p->data = malloc(strlen(val)+1);
      if (p->data)
	 strcpy(p->data, val);

      p->next = NULL;
      *prev = p;
      prev = &p->next;

      // FIXME
      if (name[0] == '[')
      {
	  prev_e = &p->head;
	  in_section = 1;
      }
   }
}
#else
void set_config(CONFIG **config, sint8 *data, int length, sint8 *filename)
{
   sint8 name[256];
   sint8 val[256];
   CONFIG_ENTRY **prev, *p;
   int pos;

   init_config(false);
   
   if (*config) {
      destroy_config(*config);
      *config = NULL;
   }

   *config = (CONFIG *)malloc(sizeof(CONFIG));
   if (!(*config))
      return;

   (*config)->head = NULL;
   (*config)->dirty = false;

   if (filename) {
      (*config)->filename = (sint8 *)malloc(strlen(filename)+1);
      if ((*config)->filename)
	 strcpy((*config)->filename, filename); 
   }
   else
      (*config)->filename = NULL;

   prev = &(*config)->head;
   pos = 0;

   while (pos < length) {
      pos += get_line(data+pos, length-pos, name, val);

      p = (CONFIG_ENTRY *)malloc(sizeof(CONFIG_ENTRY));
      if (!p)
	 return;

      if (name[0]) {
	 p->name = (sint8*)malloc(strlen(name)+1);
	 if (p->name)
	    strcpy(p->name, name);
      }
      else
	 p->name = NULL;

      p->data = (sint8*)malloc(strlen(val)+1);
      if (p->data)
	 strcpy(p->data, val);

      p->next = NULL;
      *prev = p;
      prev = &p->next;
   }
}

#endif


/* load_config_file:
 *  Does the work of loading a config file.
 */
void load_config_file(CONFIG **config, sint8 *filename, sint8 *savefile)	//requires getfatfsPath() to be called from outside!
{
	int length = 0;
	if (*config)
	{
		destroy_config(*config);
		*config = NULL;
	}

	length = FS_getFileSize(filename);
	
	if (length > 0)
	{
		FS_lock();
		FILE *f = fopen(filename, "r");
		if (f)
		{
		//FIL fhandler;
		//if(f_open(&fhandler,filename,FA_READ) == FR_OK)
		//{
			sint8 *tmp = (sint8*)malloc(length);
			if (tmp)
			{
				fread(tmp, 1, length, f);
				//unsigned int read_so_far;
				//f_read(&fhandler, tmp, length, &read_so_far);
				printf("Config loaded.\n");
				set_config(config, tmp, length, savefile);
				free(tmp);
			}
			else{
				printf("Config not found.Generating...\n");
				set_config(config, NULL, 0, savefile);
			}
			fclose(f);
			//f_close(&fhandler);
		}
		else{
			set_config(config, NULL, 0, savefile);
		}
		FS_unlock();
	}
	else{
		set_config(config, NULL, 0, savefile);
	}
}



/* set_config_file:
 *  Sets the file to be used for all future configuration operations.
 */
void set_config_file(sint8 *filename)
{
   load_config_file(&config[0], filename, filename);
}



/* set_config_data:
 *  Sets the block of data to be used for all future configuration 
 *  operations.
 */
void set_config_data(sint8 *data, int length)
{
   set_config(&config[0], data, length, NULL);
}



/* override_config_file:
 *  Sets the file that will override all future configuration operations.
 */
void override_config_file(sint8 *filename)
{
   load_config_file(&config_override, filename, NULL);
}



/* override_config_data:
 *  Sets the block of data that will override all future configuration 
 *  operations.
 */
void override_config_data(sint8 *data, int length)
{
   set_config(&config_override, data, length, NULL);
}



/* push_config_state:
 *  Pushes the current config state onto the stack.
 */
void push_config_state()
{
   int i;

   if (config[MAX_CONFIGS-1])
      destroy_config(config[MAX_CONFIGS-1]);

   for (i=MAX_CONFIGS-1; i>0; i--)
      config[i] = config[i-1];

   config[0] = NULL;
}



/* pop_config_state:
 *  Pops the current config state off the stack.
 */
void pop_config_state()
{
   int i;

   if (config[0])
      destroy_config(config[0]);

   for (i=0; i<MAX_CONFIGS-1; i++)
      config[i] = config[i+1];

   config[MAX_CONFIGS-1] = NULL;
}



/* prettify_section_name:
 *  Helper for ensuring that a section name is enclosed by [ ] braces.
 */
void prettify_section_name(sint8 *in, sint8 *out)
{
   if (in) {
      if (in[0] != '[')
	 strcpy(out, "[");
      else
	 out[0] = 0;

      strcat(out, in);

      if (out[strlen(out)-1] != ']')
	 strcat(out, "]");
   }
   else
      out[0] = 0;
}



/* hook_config_section:
 *  Hooks a config section to a set of getter/setter functions. This will 
 *  override the normal table of values, and give the provider of the hooks 
 *  complete control over that section.
 */
void hook_config_section(sint8 *section, int (*intgetter)(sint8 *, int), sint8 *(*stringgetter)(sint8 *, sint8 *), void (*stringsetter)(sint8 *,sint8 *))
{
   CONFIG_HOOK *hook, **prev;
   sint8 section_name[256];

   init_config(false);

   prettify_section_name(section, section_name);

   hook = config_hook;
   prev = &config_hook;

   while (hook) {
      if (strcasecmp(section_name, hook->section) == 0) {
	 if ((intgetter) || (stringgetter) || (stringsetter)) {
	    /* modify existing hook */
	    hook->intgetter = intgetter;
	    hook->stringgetter = stringgetter;
	    hook->stringsetter = stringsetter;
	 }
	 else {
	    /* remove a hook */
	    *prev = hook->next;
	    free(hook->section);
	    free(hook);
	 }

	 return;
      }

      prev = &hook->next;
      hook = hook->next;
   }

   /* add a new hook */
   hook = (CONFIG_HOOK*)malloc(sizeof(CONFIG_HOOK));
   if (!hook)
      return;

   hook->section = (sint8*)malloc(strlen(section_name)+1);
   if (!(hook->section)) {
      free(hook);
      return;
   }
   strcpy(hook->section, section_name);

   hook->intgetter = intgetter;
   hook->stringgetter = stringgetter;
   hook->stringsetter = stringsetter;

   hook->next = config_hook;
   config_hook = hook;
}



/* is_config_hooked:
 *  Checks whether a specific section is hooked in any way.
 */
int config_is_hooked(sint8 *section)
{
   CONFIG_HOOK *hook = config_hook;
   sint8 section_name[256];

   prettify_section_name(section, section_name);

   while (hook) {
      if (strcasecmp(section_name, hook->section) == 0)
	 return true;

      hook = hook->next;
   }

   return false;
}



/* find_config_string:
 *  Helper for finding an entry in the configuration file.
 */
CONFIG_ENTRY *find_config_string(CONFIG *config, sint8 *section, sint8 *name, CONFIG_ENTRY **prev)
{
   CONFIG_ENTRY *p;
   int in_section = true;

   if (config) {
      p = config->head;

      if (prev)
	 *prev = NULL;

      while (p) {
	 if (p->name) {
	    if ((section) && (p->name[0] == '[') && (p->name[strlen(p->name)-1] == ']')) {
	       /* change section */
	       in_section = (strcasecmp(section, p->name) == 0);
	    }
	    if ((in_section) || (name[0] == '[')) {
	       /* is this the one? */
	       if (strcasecmp(p->name, name) == 0)
		  return p;
	    }
	 }

	 if (prev)
	    *prev = p;

	 p = p->next;
      }
   }

   return NULL;
}

/* get_config_string:
 *  Reads a string from the configuration file.
 */
sint8 *get_config_string(sint8 *section, sint8 *name, sint8 *def)
{
   sint8 section_name[256];
   CONFIG_HOOK *hook;
   CONFIG_ENTRY *p;

   init_config(true);

   prettify_section_name(section, section_name);

   /* check for hooked sections */
   hook = config_hook;

   while (hook) {
      if (strcasecmp(section_name, hook->section) == 0) {
	 if (hook->stringgetter)
	    return hook->stringgetter(name, def);
	 else
	    return def;
      }
      hook = hook->next;
   }

   /* find the string */
   p = find_config_string(config_override, section_name, name, NULL);

   if (!p) {
      if ((name[0] == '#') || ((section_name[0] == '[') && (section_name[1] == '#')))
	 p = find_config_string(system_config, section_name, name, NULL);
      else
	 p = find_config_string(config[0], section_name, name, NULL);
   }

	if (p){
		return ((sint8*)p->data ? (sint8*)p->data : (sint8*)"");
	}
	else{
		return def;
	}
}



/* get_config_int:
 *  Reads an integer from the configuration file.
 */
int get_config_int(sint8 *section, sint8 *name, int def)
{
   CONFIG_HOOK *hook;
   sint8 section_name[256];
   sint8 *s;

   prettify_section_name(section, section_name);

   /* check for hooked sections */
   hook = config_hook;

   while (hook) {
      if (strcasecmp(section_name, hook->section) == 0) {
	 if (hook->intgetter) {
	    return hook->intgetter(name, def);
	 }
	 else if (hook->stringgetter) {
	    s = hook->stringgetter(name, NULL);
	    if ((s) && (*s))
	       return strtol(s, NULL, 0);
	    else
	       return def;
	 }
	 else
	    return def;
      }
      hook = hook->next;
   }

   /* read normal data */
   s = get_config_string(section_name, name, NULL);

   if ((s) && (*s))
      return strtol(s, NULL, 0);

   return def;
}



/* get_config_hex:
 *  Reads a hexadecimal integer from the configuration file.
 */
int get_config_hex(sint8 *section, sint8 *name, int def)
{
   sint8 *s = get_config_string(section, name, NULL);
   int i;

   if ((s) && (*s)) {
      i = strtol(s, NULL, 16);
      if ((i == 0x7FFFFFFF) && (strcasecmp(s, "7FFFFFFF") != 0))	//fat16 boundary limits
	 i = -1;
      return i;
   }

   return def;
}


/* get_config_oct:
 *  Reads a octal integer from the configuration file.
 */
int get_config_oct(sint8 *section, sint8 *name, int def)
{
   sint8 *s = get_config_string(section, name, NULL);
   int i;

   if ((s) && (*s)) {
      i = strtol(s, NULL, 8);
      return i;
   }

   return def;
}

/* get_config_float:
 *  Reads a float from the configuration file.
 */
float get_config_float(sint8 *section, sint8 *name, float def)
{
   sint8 *s = get_config_string(section, name, NULL);

   if ((s) && (*s))
      return atof(s);

   return def;
}


/* get_config_argv:
 *  Reads an argc/argv style token list from the configuration file.
 */
sint8 **get_config_argv(sint8 *section, sint8 *name, int *argc)
{
   

   static sint8 buf[256];
   static sint8 *argv[MAX_ARGV];
   int pos, ac;

   sint8 *s = get_config_string(section, name, NULL);

   if (!s) {
      *argc = 0;
      return NULL;
   }

   strcpy(buf, s);
   pos = 0;
   ac = 0;

   while ((ac<MAX_ARGV) && (buf[pos]) && (buf[pos] != '#')) {
      while ((buf[pos]) && (isspace((int)buf[pos])))
	 pos++;

      if ((buf[pos]) && (buf[pos] != '#')) {
	 argv[ac++] = buf+pos;

	 while ((buf[pos]) && (!isspace((int)buf[pos])))
	    pos++;

	 if (buf[pos])
	    buf[pos++] = 0;
      }
   }

   *argc = ac;
   return argv;
}



/* insert_variable:
 *  Helper for inserting a new variable into a configuration file.
 */
CONFIG_ENTRY *insert_variable(CONFIG *the_config, CONFIG_ENTRY *p, sint8 *name, sint8 *data)
{
   CONFIG_ENTRY *n = (CONFIG_ENTRY *)malloc(sizeof(CONFIG_ENTRY));

   if (!n)
      return NULL;

   if (name) {
      n->name = (sint8*)malloc(strlen(name)+1);
      if (n->name)
	 strcpy(n->name, name);
   }
   else
      n->name = NULL;

   if (data) {
      n->data = (sint8*)malloc(strlen(data)+1);
      if (n->data)
	 strcpy(n->data, data);
   }
   else
      n->data = NULL;

   if (p) {
      n->next = p->next;
      p->next = n; 
   }
   else {
      n->next = NULL;
      the_config->head = n;
   }

   return n;
}



/* set_config_string:
 *  Writes a string to the configuration file.
 */
void set_config_string(sint8 *section, sint8 *name, sint8 *val)
{
   CONFIG *the_config;
   CONFIG_HOOK *hook;
   CONFIG_ENTRY *p, *prev;
   sint8 section_name[256];

   init_config(true);

   prettify_section_name(section, section_name);

   /* check for hooked sections */
   hook = config_hook;

   while (hook) {
      if (strcasecmp(section_name, hook->section) == 0) {
	 if (hook->stringsetter)
	    hook->stringsetter(name, val);
	 return;
      }
      hook = hook->next;
   }

   /* decide which config file to use */
   if ((name[0] == '#') || ((section_name[0] == '[') && (section_name[1] == '#')))
      the_config = system_config;
   else
      the_config = config[0];

   if (the_config) {
      p = find_config_string(the_config, section_name, name, &prev);

      if (p) {
	 if ((val) && (*val)) {
	    /* modify existing variable */
	    if (p->data)
	       free(p->data);

	    p->data = (sint8*)malloc(strlen(val)+1);
	    if (p->data)
	       strcpy(p->data, val);
	 }
	 else {
	    /* delete variable */
	    if (p->name)
	       free(p->name);

	    if (p->data)
	       free(p->data);

	    if (prev)
	       prev->next = p->next;
	    else
	       the_config->head = p->next;

	    free(p);
	 }
      }
      else {
	 if ((val) && (*val)) {
	    /* add a new variable */
	    if (section_name[0]) {
	       p = find_config_string(the_config, NULL, section_name, &prev);

	       if (!p) {
		  /* create a new section */
		  p = the_config->head;
		  while ((p) && (p->next))
		     p = p->next;

		  if ((p) && (p->data) && (*p->data))
		     p = insert_variable(the_config, p, NULL, NULL);

		  p = insert_variable(the_config, p, section_name, NULL);
	       }

	       /* append to the end of the section */
	       while ((p) && (p->next) && 
		      (((p->next->name) && (*p->next->name)) || 
		       ((p->next->data) && (*p->next->data))))
		  p = p->next;

	       p = insert_variable(the_config, p, name, val);
	    }
	    else {
	       /* global variable */
	       p = the_config->head;
	       insert_variable(the_config, NULL, name, val);
	       the_config->head->next = p;
	    }
	 } 
      }

      the_config->dirty = true;
   }
}



/* set_config_int:
 *  Writes an integer to the configuration file.
 */
void set_config_int(sint8 *section, sint8 *name, int val)
{
   sint8 buf[32];
   sprintf(buf, "%d", val);
   set_config_string(section, name, buf);
}



/* set_config_hex:
 *  Writes a hexadecimal integer to the configuration file.
 */
void set_config_hex(sint8 *section, sint8 *name, int val)
{
   if (val >= 0) {
      sint8 buf[32];
      sprintf(buf, "%X", val);
      set_config_string(section, name, buf);
   }
   else
      set_config_string(section, name, "-1");
}


/* set_config_oct:
 *  Writes a hexadecimal integer to the configuration file.
 */
void set_config_oct(sint8 *section, sint8 *name, int size, int val)
{
   if (val >= 0) {
      sint8 buf[16];
      sint8 fmt[8];
      sprintf(fmt, "%%0%do", size);
      sprintf(buf, fmt, val);
      set_config_string(section, name, buf);
   }
   else
      set_config_string(section, name, "-1");
}



/* set_config_float:
 *  Writes a float to the configuration file.
 */
void set_config_float(sint8 *section, sint8 *name, float val)
{
   sint8 buf[32];
   sprintf(buf, "%f", val);
   set_config_string(section, name, buf);
}





/* _load_config_text:
 *  Reads in a block of translated system text, looking for either a
 *  user-specified file, a ??text.cfg file, or a language.dat#??TEXT_CFG 
 *  datafile object.
 */
void _load_config_text()
{
}



/* get_config_text:
 *  Looks up a translated version of the specified English string,
 *  returning a suitable message in the current language if one is
 *  available, or a copy of the parameter if no translation can be found.
 */
sint8 * get_config_text(sint8 *msg)
{
    return NULL;
}


int	is_section_exists(sint8 *section)
{
   CONFIG_ENTRY *p;
   CONFIG *cfg;
   sint8 section_name[256];

   init_config(true);

   prettify_section_name(section, section_name);

   cfg = config[0];

   if (cfg) {
      p = cfg->head;

      while (p) {
	 if (p->name) {
	    if ((section) && (p->name[0] == '[') && (p->name[strlen(p->name)-1] == ']')) {
	       if (strcasecmp(section_name, p->name) == 0)
			return 1;
	    }
	 }

	 p = p->next;
      }
   }

   return 0;
}

static sint8	g_section[100];

/* find_config_string:
 *  Helper for finding an entry in the configuration file.
 */
sint8 *find_config_section_with_hex(sint8 *name, int hex)
{
   CONFIG_ENTRY *p;
   CONFIG   *cfg;

   init_config(true);

   cfg = config[0];

   if (cfg) {
      p = cfg->head;

      while (p) {
	 if (p->name) {
	    if ((p->name[0] == '[') && (p->name[strlen(p->name)-1] == ']')) {
	       strncpy(g_section, p->name+1, strlen(p->name)-2);
	       g_section[strlen(p->name)-2] = 0;
	    }
	    else
	    /* is this the one? */
	       if (strcasecmp(p->name, name) == 0)
	       {
		   if ((p->data) && (*p->data)) {
		        int i;
			i = strtoul(p->data, NULL, 16);
			if (i == hex)
			    return g_section;
		   }
		}
	 }

	 p = p->next;
      }
   }

   return NULL;
}

sint8 *find_config_section_with_string(sint8 *name, sint8 *str)
{
   CONFIG_ENTRY *p;
   CONFIG   *cfg;

   init_config(true);

   cfg = config[0];

   if (cfg) {
      p = cfg->head;

      while (p) {
	 if (p->name) {
	    if ((p->name[0] == '[') && (p->name[strlen(p->name)-1] == ']')) {
	       /* change section */
	       strncpy(g_section, p->name+1, strlen(p->name)-2);
	       g_section[strlen(p->name)-2] = 0;
	    }
	    else
	    /* is this the one? */
	       if (strcasecmp(p->name, name) == 0)
	       {
		   if ((p->data) && (*p->data)) {
		        int i;
			i = strcasecmp(str, p->data);
			if (i == 0)
			    return g_section;
		   }
		}
	 }

	 p = p->next;
      }
   }

   return NULL;
}
