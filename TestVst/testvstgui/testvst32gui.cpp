   #include <stdio.h>
   #include <stdlib.h>
   #include <dirent.h>
   #include <iostream>
   #include <fstream>
   #include <gtk/gtk.h>

   gchar *folderpath;

   int folderhit = 0;

   int doconvert(char *folder)
   {   	   
   std::string cfolder = folder;
   
   std::string command = "./testvst32-batch ";
   
   std::string extra = "/";
   
   command = command + cfolder + extra;
   
   system(command.c_str());
   
   return 0;    
   }

void quitcallback ()
{
	
  if(folderpath)
  g_free(folderpath);

  gtk_main_quit ();

}

void foldercallback (GtkFileChooser *folderselect)
{

folderpath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (folderselect));

folderhit = 1;

}

void buttoncallback(GtkFileChooser *button)
{	
	
 if(folderhit == 1)
 { 
 if(doconvert(folderpath) == 1)
 {
 gtk_button_set_label(GTK_BUTTON (button), "Not Found");
 folderhit = 0;
 return;
 }
 
 gtk_button_set_label(GTK_BUTTON (button), "Done");

 folderhit = 0;
}

}

int main (int argc, char *argv[])
{
  GtkWidget *window, *folderselect, *spacertext2, *vbox, *button;
  GtkFileFilter *extfilter;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "TestVst");
  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  spacertext2 = gtk_label_new ("Choose vst dll folder");
  
  folderselect = gtk_file_chooser_button_new ("Choose vst dll Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

  button = gtk_button_new ();
  gtk_button_set_label(GTK_BUTTON (button), "Start");

  vbox = gtk_vbox_new (FALSE, 8);

  gtk_box_pack_start(GTK_BOX (vbox), spacertext2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), folderselect, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), button, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (quitcallback), NULL);
  g_signal_connect (G_OBJECT (folderselect), "selection_changed", G_CALLBACK (foldercallback), NULL);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (buttoncallback), NULL);

  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(folderselect), TRUE);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (folderselect), g_get_home_dir());

  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show_all (window);

  gtk_main ();
  return 0;
}

