### Board Test Program
##### Author: D. Coderre
##### Date: 3.16.2016

#### Brief 

Use a simple function to cause a crash

#### Hardware Setup
 * Ubuntu 14.04 server
 * CAEN A3818 with driver 1.5.2, Firmware 0.5, 2 optical links
 * CAEN V2718 hooked to optical link 0
 * CAEN V1724 hooked to optical link 1 
   * Optionally, second V1724 daisy-chained on optical link 1 as well
 * Output 3 of V2718 fanned to TRIN of V1724(s)
 * V1724 firmware V1724_XENON_F9180407_FA278904 

#### Software
 * CAEN VMElib 2.4.1
 * This software
 * On Ubuntu probably just build-essential

#### Building
 * Install CAEN VME lib
 * Install A3818 kernel module
 * make

#### Reproducing the error:

##### Step 1: Running with one V1724 (no error)
  
  1. Hook up V2718 to A3818 port 0 via optical link
  2. Hook up one V1724 to A3818 port 1 via optical link
  3. Make sure only one V1724 is defined in crash.ini
  4. Set the trigger frequency using TRIGGER in crash.ini to 10
  5. Run ./crashit and some data should transfer (gets deleted)
  6. It should be possible to adjust TRIGGER to any value (tested up to 15000) with no issue

##### Step 2: Running with two V1724 (error arises)

  1. Hook up V2718 to A3818 port 0 via optical link
  2. Hook up two V1724 in daisy chain to A3818 port 1 via optical link
  3. Make sure two V1724 are defined in crash.ini (uncomment the line indicated)
  4. Set the trigger frequency using TRIGGER in crash.ini to 10
  5. Run ./crashit and some data should come
  6. At low rates it probably won't crash. But run with increasing trigger rates and eventually the boards get a read error (-2). 
  7. After getting the read error try to run the program again. It will fail on CAENVME_init for any board that got a read error in the previous run.
  8. Reboot the crate to clear the error (maybe, sometimes need to do twice)
        	
