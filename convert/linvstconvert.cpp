  #include <stdio.h>
  #include <dirent.h>
  #include <iostream>
  #include <fstream>
    
   int main()
   {
   DIR *dirlist;
   struct dirent *dentry;
   std::string convertname;
   
   bool test = std::ifstream("linvst.so").good();
   
   if(!test)
   {
   printf("can't find linvst.so file\n");
   return 0;
   }
 
   dirlist = opendir("."); 
   
   if(dirlist != NULL)
   {
      
   while((dentry = readdir(dirlist)) != NULL)
    {

    convertname = " ";

    convertname = dentry->d_name;
   
    if(convertname.find(".dll") != std::string::npos)
    {
    convertname.replace(convertname.begin() + convertname.find(".dll"), convertname.end(), ".so");
    }
    else if(convertname.find(".Dll") != std::string::npos)
    {
    convertname.replace(convertname.begin() + convertname.find(".Dll"), convertname.end(), ".so");
    }
    else if(convertname.find(".DLL") != std::string::npos)
    {
    convertname.replace(convertname.begin() + convertname.find(".DLL"), convertname.end(), ".so");
    }
    else
    continue;

    std::ifstream source("linvst.so", std::ios::binary);
      
    std::ofstream dest(convertname.c_str(), std::ios::binary);

    dest << source.rdbuf();

    source.close();
    dest.close();
        
    }
    
    }

    closedir(dirlist);
    
   }

