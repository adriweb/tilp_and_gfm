/*
 * TiLP Cocoa GUI for Mac OS X
 */
 
 #ifndef __COCOA_STRUCTS_H__
 #define __COCOA_STRUCTS_H__
 
 #import <Cocoa/Cocoa.h>
 
 struct cocoa_objects_ptr
 {
    // our classes
    id BoxesController;
    id MenuController;
    id PrefsController;
    id ToolbarController;
    id TilpController;
    
    // the main window
    id mainWindow;
    
    // the dlgbox (for pbars, see the following struct)
    id dlgboxEntry;
    
    // the remote control/terminal window
    id remoteControlWindow;
    
    // and its textField
    id remoteControlTextArea;
    
    // the operating mode of the remote control/terminal stuff
    int term_mode;
    
    // used by the preferences system
    NSMutableDictionary *tilpConfig;
    NSUserDefaults *prefs;
    
    // keep track of which widget is being used
    id alertPanel;
    
    // used for dlgbox_entry and userX_box
    char *dlgbox_data;
    int box_button;
 };


 // this struct replaces the progess_window p_win struct...
 struct cocoa_pbars_ptr
 {
    // replaces the gtk event loop
    int finished;
 
    // the different windows
    id pbar1Window;
    id pbar3Window;
    id pbar4Window;
    id pbar5Window;
  
    // the pbars in these windows
    id pbar1PBar; 
    id pbar3PBar1;
    id pbar3PBar2;
    id pbar4PBar;
    id pbar5PBar1;
    id pbar5PBar2;

    // the textField indicating the rate
    id pbar1Rate;
    id pbar3Rate;
    id pbar4Rate;
    id pbar5Rate;
    
    // the textField displaying a text message
    id pbar4Text;    
    id pbar5Text;
 
    // keep track of the pbars in use
    id pbar1;
    id pbar2;
    
    // keep track of the textfields used
    id pbar_rate;
    id pbar_text;
 
 };
 
 #endif /* !__COCOA_STRUCTS_H__ */