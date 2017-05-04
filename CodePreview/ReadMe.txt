CodePreview (previews source code files)
===============================
This project contains a handler for displaying a preview inside the
Windows Explorer preview pane, Outlook or other preview handler hosts.

Sample Language Implementations
===============================
C++

To run the sample:
=================
     0. Run "regsvr32.exe codepreview.dll" to register the handler.
     1. Navigate Windows Explorer to the directory that contains the sample files.
     2. Select the example .cpp file.
     3. Show the Preview Pane if it is not already showing.

64-bit Note
===========
THIS PROJECT AS-IS MUST BE BUILT 64-BIT FOR 64-BIT TARGETS
You can also make the 32-bit output work on a 64-bit version of Windows by changing the AppId
on the class registration to the WOW64 surrogate host (which will work for x86 and x64 versions of Windows).
That AppId value is: {534A1E02-D58F-44f0-B58B-36CBED287C7C}


Removing the Sample
==================
Run "regsvr32.dll /u codepreview.dll"
