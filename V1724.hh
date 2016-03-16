#ifndef _V1724_HH_
#define _V1724_HH_

/* Simple class for interacting with CAEN boards
   All output printed to log file created with contructor
*/
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sys/types.h>

using namespace std;

class V1724{

public:
  V1724();
  ~V1724();
  V1724(string logfile, string optionsfile, int &ret);

  int Initialize(string logfile, string optionsfile);

  // Write a register value to board with handle. Ret 0(success) -1(fail)
  int WriteRegister(u_int32_t reg, u_int32_t val, int handle);
  // Read a register to val. Ret 0(success) -1(fail)
  int ReadRegister(u_int32_t reg, u_int32_t &val, int handle);
  // Initialize a board and put handle into memory
  int InitializeBoard(int link, int board);
  // Load a settings file containing board definitions and register settings
  int LoadSettings();
  // Initialize V2718 to trigger the boards
  int InitializeCrateController(int handle, int triggerRate);

  // Read data
  int ReadToNowhere();

  int StartBoards();
  int StopLoop();

  void SWTrigger();

  // CAENVME_End to all digitizers, close cc
  int Close();
private:
  // Helper function for reading ini file
  u_int32_t StringToHex(const string &str);
  u_int32_t StringToInt(const string &str);
  string GetTimeString();

  // Store the digitizer
  vector <int> fDigitizers;
  int fCrateController;

  // Store digitizer settings
  vector<pair <int, int> > fRegisters;
  int fTriggerRate;
  ofstream fLog;
};

#endif
