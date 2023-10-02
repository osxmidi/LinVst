   #include <stdio.h>
   #include <dirent.h>
   #include <iostream>
   #include <fstream>
   #include <gtk/gtk.h>
   #include <string.h>
   #include <fts.h>
   #include <bits/stdc++.h>

   gchar *folderpath;
   gchar *filepath;

   int filehit = 0;
   int folderhit = 0;
   int filecopy = 0;

   int intimer = 0;
   int dofullup = 0;
   int vst2filehit = 0;

   size_t find_last(std::string searchstr, std::string searcharg)
   {
   size_t foundret = 0;
   size_t found = searchstr.find(searcharg, 0);
   foundret = found;
 
   while(found != std::string::npos)
   {
   found += searcharg.size();
   found = searchstr.find(searcharg, found);
   if(found != std::string::npos)
   foundret = found;
   }
   return foundret;
   }

   int doconvert(char *linvst, char folder[])
   {
   DIR *dirlist;
   struct dirent *dentry;
   std::string convertname;
   std::string convertnamecase;
   std::string cfolder;
   char *folderpath[] = {folder, 0};

   filecopy = 1;
   
   bool test = std::ifstream(linvst).good();
   
   if(!test)
   {
   filecopy = 0;
   return 1;
   }

   FTS *fs = fts_open(folderpath, FTS_NOCHDIR | FTS_LOGICAL, 0);
   if (!fs) 
   {
   return 1;
   }

   FTSENT *node;

    while ((node = fts_read(fs))) 
    {
    if (node->fts_level > 0 && node->fts_name[0] == '.')
    fts_set(fs, node, FTS_SKIP);
    else if (node->fts_info & FTS_F) 
    convertname = node->fts_path;
    else continue;
    convertnamecase = convertname;
    transform(convertnamecase.begin(), convertnamecase.end(), convertnamecase.begin(), ::tolower);
	vst2filehit = 0;

    if(convertnamecase.find(".dll") != std::string::npos)
	{
    int fulllength = strlen(convertnamecase.c_str());
    if((convertnamecase[fulllength - 1] == 'l') && (convertnamecase[fulllength - 2] == 'l') && (convertnamecase[fulllength - 3] == 'd') && (convertnamecase[fulllength - 4] == '.'))
    {    
    size_t position = find_last(convertnamecase, ".dll");
    if(position != 0)
    convertname.replace(convertname.begin() + position, convertname.end(), ".so");
    vst2filehit = 1;
	}	
    }

	if(vst2filehit == 1)
	{
    if(dofullup == 1)
    {
    std::string sourcename = linvst;
    std::ifstream source(sourcename.c_str(), std::ios::binary);      
    std::ofstream dest(convertname.c_str(), std::ios::binary);
    dest << source.rdbuf();
    source.close();
    dest.close();	
    }
    else
    {
    test = std::ifstream(convertname).good();
   
    if(!test)
    {
    std::string sourcename = linvst;
    std::ifstream source(sourcename.c_str(), std::ios::binary);      
    std::ofstream dest(convertname.c_str(), std::ios::binary);
    dest << source.rdbuf();
    source.close();
    dest.close();	
	}	
    }
    }
    }

   if(fs)
   fts_close(fs);
 
   filecopy = 0;

   return 0;
    
   }

void quitcallback ()
{
  if(filecopy == 0)
{
  if(folderpath)
  g_free(folderpath);

  if(filepath)
  g_free(filepath);

  gtk_main_quit ();
}

}

void foldercallback (GtkFileChooser *folderselect)
{

folderpath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (folderselect));

folderhit = 1;

}

void filecallback (GtkFileChooser *fileselect)
{

filepath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileselect));

filehit = 1;

}

gboolean dolabelupdate(gpointer data)
{

 gtk_button_set_label(GTK_BUTTON (data), "Update Newly Added Plugins");

 intimer = 0;

 return FALSE;

}

gboolean dolabelupdate2(gpointer data)
{

 gtk_button_set_label(GTK_BUTTON (data), "Update All Plugins (Upgrade All Plugins)");

 intimer = 0;

 return FALSE;

}

void buttoncallback(GtkFileChooser *button, gpointer data)
{

 std::string name;

 if((filehit == 1) && (folderhit == 1) && (intimer == 0))
 {
 
 name = filepath;
 
 if(name.find("linvst.so") == std::string::npos)
 {
 gtk_button_set_label(GTK_BUTTON (button), "Not Found");

 filecopy = 0;
 filehit = 0;
 folderhit = 0;

 intimer = 1;

 g_timeout_add_seconds(3, dolabelupdate, (GtkWidget *)data);

 return;
 }

 if(doconvert(filepath, folderpath) == 1)
 {
 gtk_button_set_label(GTK_BUTTON (button), "Not Found");

 filecopy = 0;
 filehit = 0;
 folderhit = 0;

 intimer = 1;

 g_timeout_add_seconds(3, dolabelupdate, (GtkWidget *)data);

 return;
 }
 
 filecopy = 0;
 filehit = 1;
 folderhit = 0;

 gtk_button_set_label(GTK_BUTTON (button), "Done");

 intimer = 1;

 g_timeout_add_seconds(3, dolabelupdate, (GtkWidget *)data);

}

}

void buttoncallback2(GtkFileChooser *button, gpointer data)
{
 std::string name;

 if((filehit == 1) && (folderhit == 1) && (intimer == 0))
 {
 
 name = filepath;
 
 if(name.find("linvst.so") == std::string::npos)
 {
 gtk_button_set_label(GTK_BUTTON (button), "Not Found");

 filecopy = 0;
 filehit = 0;
 folderhit = 0;

 intimer = 1;

 g_timeout_add_seconds(3, dolabelupdate2, (GtkWidget *)data);

 return;
 }

 dofullup = 1;

 if(doconvert(filepath, folderpath) == 1)
 {
 gtk_button_set_label(GTK_BUTTON (button), "Not Found");

 filecopy = 0;
 filehit = 0;
 folderhit = 0;
 dofullup = 0;

 intimer = 1;

 g_timeout_add_seconds(3, dolabelupdate2, (GtkWidget *)data);

 return;
 }
 
 filecopy = 0;
 filehit = 1;
 folderhit = 0;
 dofullup = 0;

 gtk_button_set_label(GTK_BUTTON (button), "Done");

 intimer = 1;

 g_timeout_add_seconds(3, dolabelupdate2, (GtkWidget *)data);

}
}


int main (int argc, char *argv[])
{
  GtkWidget *window, *folderselect, *fileselect, *spacertext, *spacertext2, *spacertext3, *vbox, *button, *button2;
  GtkFileFilter *extfilter;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 300); 
  gtk_window_set_title (GTK_WINDOW (window), "LinVst");
  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  spacertext = gtk_label_new ("Choose linvst.so");
  spacertext2 = gtk_label_new ("Choose dll folder");
  spacertext3 = gtk_label_new ("Convert");
  
  folderselect = gtk_file_chooser_button_new ("Choose dll Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  fileselect = gtk_file_chooser_button_new ("Choose linvst.so", GTK_FILE_CHOOSER_ACTION_OPEN);

  button = gtk_button_new ();
  gtk_button_set_label(GTK_BUTTON (button), "Update Newly Added Plugins");

  button2 = gtk_button_new ();
  gtk_button_set_label(GTK_BUTTON (button2), "Update All Plugins (Upgrade All Plugins)");

  vbox = gtk_vbox_new (FALSE, 8);
  gtk_box_pack_start(GTK_BOX (vbox), spacertext, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), fileselect, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), spacertext2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), folderselect, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), spacertext3, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), button2, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (quitcallback), NULL);
  g_signal_connect (G_OBJECT (folderselect), "selection_changed", G_CALLBACK (foldercallback), NULL);
  g_signal_connect (G_OBJECT (fileselect), "selection_changed", G_CALLBACK (filecallback), NULL);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (buttoncallback), button);
  g_signal_connect (G_OBJECT (button2), "clicked", G_CALLBACK (buttoncallback2), button2);

  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(folderselect), TRUE);
  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(fileselect), TRUE);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (folderselect), g_get_home_dir());
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fileselect), g_get_current_dir ());

  extfilter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (extfilter, "linvst.so");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fileselect), extfilter);

  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show_all (window);

  gtk_main ();
  return 0;
}

