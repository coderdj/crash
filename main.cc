#include "V1724.hh"
#include <iostream>
#include <unistd.h>

int main(){
  
  cout<<"Starting program"<<endl;

  string logfile = "crash.log";
  string inifile = "crash.ini";

  // Initialize electronics
  int ret=0;
  V1724 DigitizerInterface(logfile, inifile, ret);
  if(ret != 0){
    cout<<"Failed initialization. Bye"<<endl;
    return -1;
  }

  // Load register settings
  if(DigitizerInterface.LoadSettings()!=0){
    cout<<"Failed to load digitizer settings."<<endl;
    return -1;
  }

  // Start run
  if(DigitizerInterface.StartBoards()!=0){
    cout<<"Failed to start boards"<<endl;
    return -1;
  }

  // Loop
  char a='r';
  int counter=0;
  int tbytes=0;
  while(a!='q'){    
    int bytes=0;
    if((bytes=DigitizerInterface.ReadToNowhere())<0)
      cout<<"Read failed. Continuine until you stop it."<<endl;
    else if(bytes>0){
      tbytes+=bytes;
      cout<<"Transferred "<<tbytes/1000000<<" MB"<<endl;
    }
    //cin.get(a);
    //usleep(1000);
    counter++;
    //sleep(1);
    if(counter>500000)
      break;
  }
  DigitizerInterface.Close();
  return 0;
}
