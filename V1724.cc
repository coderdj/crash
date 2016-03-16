#include <CAENVMElib.h>
#include <CAENVMEtypes.h>
#include <iostream>
#include <sstream>
#include "V1724.hh"
#include <time.h>
#include <iterator>
#include <utility>   


V1724::V1724()
{
  fCrateController = -1;  
  fTriggerRate=-1;
}
V1724::~V1724()
{}
V1724::V1724(string logfile, string optionsfile, int &ret){
  fCrateController = -1;
  fTriggerRate=-1;
  ret = Initialize(logfile, optionsfile);
}

int V1724::Initialize(string logfile, string optionsfile){

  // Open log file
  fLog.open(logfile.c_str());
  if(!fLog){
    cerr<<"Failed to open log file!"<<endl;
    return -1;
  }

  // Open options file and parse
  ifstream infile;
  infile.open(optionsfile.c_str());
  if(!infile){
    cerr<<"Failed to open options file!"<<endl;
    return -1;
  }
  
  string file_line;
  int ccLink=-1;
  
  while(getline(infile, file_line)){
    if ( file_line[0] == '#' ) continue; //comment
    
    //parse       
    istringstream iss(file_line);
    vector<string> words;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(words));
    if(words.size()<2) continue;

    if(words[0] == "BOARD" && words.size()>=4){
      if(words[1]=="V1724"){
	int errorcode=0;
	if((errorcode=InitializeBoard(StringToInt(words[2]), StringToInt(words[3])))
	    !=0){
	  cerr<<"Failed to initialize digitizer. Error: "<<errorcode<<endl;
	  return -1;
	}
      }
      else if(words[1]=="V2718")
	ccLink=StringToInt(words[2]);
    }
    if(words[0] == "REGISTER" && words.size()>=3)
      fRegisters.push_back(make_pair(StringToHex(words[1]),StringToHex(words[2])));
    if(words[0] == "TRIGGER" && words.size()>=2)
      fTriggerRate = StringToInt(words[1]);
  }
  if(ccLink!=-1 && fTriggerRate!=-1){
    if(InitializeCrateController(ccLink, fTriggerRate)!=0){
      cout<<"Failed to initialize crate controller"<<endl;
      //return -1;
    }
  }

  //  if(fCrateController == -1){
  //cerr<<"You didn't specify a V2718 in the ini file, exiting."<<endl;
  //return -1;
  //}
  if(fDigitizers.size()==0){
    cerr<<"No digitizers found in ini file, exiting."<<endl;
    return -1;
  }
  fLog<<GetTimeString()<<": Initialized with "<<fDigitizers.size()<<" boards and "<<
      "crate controller at handle "<<fCrateController<<endl;
  infile.close();
  return 0;
    
}

int V1724::WriteRegister(u_int32_t reg, u_int32_t val, int handle){
  // This function writes a CAEN register and returns 0 (success) or -1 (failure)  
  // Output is logged to fLog
  int BOARD_VME=0;
  cout<<"Write register "<<hex<<reg<<" with value "<<val<<dec<<" to handle "<<handle<<endl;

  int ret = CAENVME_WriteCycle(handle,BOARD_VME+reg,
                               &val,cvA32_U_DATA,cvD32);
  if(ret!=cvSuccess)
    fLog<<"Failed to write register "<<hex<<reg<<" with value "<<val<<
      ", returned value: "<<dec<<ret<<endl;
  else
    fLog<<"Write register "<<hex<<reg<<" with value "<<val<<dec<<" returns "<<ret<<endl;
  return ret;
}

int V1724::ReadRegister(u_int32_t reg, u_int32_t &val, int handle){
  // Read a register and put its value in val
  int BOARD_VME=0;
  int ret = CAENVME_ReadCycle(handle,BOARD_VME+reg,
                              &val,cvA32_U_DATA,cvD32);
  if(ret!=cvSuccess)
    fLog<<"Failed to read register "<<hex<<reg<<" with return value: "<<
      dec<<ret<<endl;
  else
    fLog<<"Read register " << hex << reg << " with value " << val << dec <<endl;
  return ret;
}

int V1724::InitializeBoard(int link, int board){
  // CAENVME_init                                                                    
  CVBoardTypes BType = cvV2718;
  int tempHandle = -1;
  int cerror;
  if((cerror=CAENVME_Init(BType,link,board,
                          &tempHandle))!=cvSuccess){
    fLog<<"Error in CAEN initialization link "
        <<link<<" crate "<<board<<" handle "<<tempHandle<<": "<<cerror<<endl;
    return -1;
  }
  else
    fLog<<"CAENVME_init success for board handle " << tempHandle <<endl;
  
  fDigitizers.push_back(tempHandle);
  return 0;
}

int V1724::LoadSettings(){
  for(unsigned int x=0; x<fDigitizers.size();x++){
    // Load register settings
    for( unsigned int y=0; y<fRegisters.size(); y++ ){
      if(WriteRegister(fRegisters[y].first, fRegisters[y].second, fDigitizers[x])!=0)
	return -1;
    }
  }
  return 0;
}

int V1724::InitializeCrateController(int link, int i_pulserHz){
  
  // Initialize
  CVBoardTypes BType = cvV2718;
  int tempHandle=-1, cerror=0;
  if((cerror=CAENVME_Init(BType,link,0,&tempHandle))!=cvSuccess){
    fLog<<"Error initializing crate controller on link "<<link<<endl;
    return -1;
  }
  fCrateController = tempHandle;

  
  if(i_pulserHz > 0){
    // We allow a range from 1Hz to 1MHz, but this is not continuous!  
    // If the number falls between two possible ranges it will be rounded to 
    // the maximum value of the lower one  
    CVTimeUnits tu = cvUnit104ms;
    u_int32_t width = 0x1;
    u_int32_t period = 0x0;
    if(i_pulserHz < 10){
      if(i_pulserHz > 5)
        period = 0xFF;
      else
        period = (u_int32_t)((1000/104) / i_pulserHz);
    }
    else if(i_pulserHz < 2450){
      tu = cvUnit410us;
      if(i_pulserHz >1219)
        period = 0xFF;
      else
        period = (u_int32_t)((1000000/410) / i_pulserHz);
    }
    else if(i_pulserHz < 312500){
      tu = cvUnit1600ns;
      period = (u_int32_t)((1000000/1.6) / i_pulserHz);
    }
    else if(i_pulserHz < 20000000){
      tu = cvUnit25ns;
      period = (u_int32_t)((1E9/25)/i_pulserHz);
    }
    else
      fLog<<"Invalid LED frequency set!"<<endl;;
    fLog<<"Setting LED frequecy "<<i_pulserHz<<" period "<<period<<" width "<<width<<endl;
    CAENVME_SetOutputConf(fCrateController, cvOutput3, cvDirect,
			  cvActiveHigh, cvMiscSignals);
    CAENVME_SetPulserConf(fCrateController, cvPulserB, period, width, tu, 0,
                          cvManualSW, cvManualSW);
    CAENVME_StartPulser(fCrateController,cvPulserB);
  }
  return 0;
  
}

int V1724::StartBoards(){
  for(unsigned int x=0;x<fDigitizers.size();x++){
    if(WriteRegister(0x8100, 0x4, fDigitizers[x])!=0){
      fLog<<"Failed to start the run for digitizer "<<fDigitizers[x]<<endl;
      return -1;
    }
    fLog<<"Started digitizer "<<fDigitizers[x]<<endl;
  }
  return 0;
}

int V1724::Close(){
  //Stop digitizers
  for(unsigned int x=0;x<fDigitizers.size();x++){
    if(WriteRegister(0x8100, 0x0, fDigitizers[x])!=0){
      fLog<<"Failed to stop the run for digitizer "<<fDigitizers[x]<<endl;
      return -1;
    }
  }
  int ret=0;
  for(unsigned int x=0;x<fDigitizers.size();x++){
    if(CAENVME_End(fDigitizers[x])!=cvSuccess)    {
      fLog<<"Failed to end digitizer "<<fDigitizers[x]<<endl;
      ret=-1;
    }
    if(CAENVME_End(fCrateController!=cvSuccess)){
      fLog<<"Failed to end crate controller " << fCrateController<<endl;
      ret=-1;
    }
  }
  return ret;
}
      
u_int32_t V1724::StringToHex(const string &str){
  stringstream ss(str);
  u_int32_t result;
  return ss >> std::hex >> result ? result : 0;
}

u_int32_t V1724::StringToInt(const string &str)
{
  stringstream ss(str);
  u_int32_t result;
  return ss >> result ? result : 0;
}

void V1724::SWTrigger(){
  int ret=0;
  for(unsigned int x=0; x<fDigitizers.size(); x++){
    int data = 0x1;
    ret += CAENVME_WriteCycle(fDigitizers[x],0x8108,
			      &data,cvA32_U_DATA,cvD32);
  }
  if(ret!=0)
    fLog<<"Error writing software trigger"<<endl;
}

int V1724::ReadToNowhere(){
  //Reads out all boards and deletes the data.
  // Returns -1 on failure, otherwise bytes read out

  // Initialize           
  unsigned int blt_bytes=0;
  int nb=0,ret=-5;

  for(unsigned int x=0;x<fDigitizers.size();x++){
    u_int32_t fBLTSize = 8388608;
    u_int32_t *buff = new u_int32_t[fBLTSize];
    do{
      nb=0;
      ret = CAENVME_FIFOBLTReadCycle(fDigitizers[x],0x32100000,
				     ((unsigned char*)buff)+blt_bytes,
				     fBLTSize,cvA32_U_BLT,cvD32,&nb);
      
      if((ret!=cvSuccess) && (ret!=cvBusError)){
	fLog<<"Board "<<fDigitizers[x]<<" reports read error "<<dec<<ret<<endl;
	delete[] buff;
	return -1;
      }
      blt_bytes+=nb;
      if(blt_bytes>fBLTSize)   {
	fLog<<"Board "<<fDigitizers[x]<<" reports insufficient BLT buffer size. ("
	    <<blt_bytes<<" > "<<fBLTSize<<")"<<endl;
	delete[] buff;
	return -1;
      }
    }while(ret!=cvBusError);
    delete[] buff;
  }

  return blt_bytes;

}

string V1724::GetTimeString()
{
  time_t rawtime;
  struct tm *timeinfo;
  char timestring[25];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(timestring,25,"%Y-%m-%dT%H:%M:%S - ",timeinfo);
  string retstring(timestring);
  return retstring;
}
