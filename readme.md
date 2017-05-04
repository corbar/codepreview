# Code Previewer Handler

This project creates a previewer handler for C++ source code (`*.cpp` and `*.h`).

## Compile

Open the solution (`Preview Handlers.sln`) in Visual Studio 2015. The solution contains three projects:
- `CodePreview`: implements the COM interfaces used by the applications to preview files in Windows
- `Scintilla`: the actual component that display the content of the files (you can replace this project with a newer version from [www.scintilla.org](http://www.scintilla.org/))
- `Simpad`: can be ignored at the moment.

Compile the solution (for maximum performance, you should compile using the release configuration).
This will produce two `dll`s: `CodePreview.dll` and `SciLexer.dll`. Copy both of them to a folder
where you want to store them permanently (e.g., `C:\Program Files\CodePreview`). Open the command line 
with administrator privileges and run the following command (make sure to use **your** path of the `dll`):
```
regsvr32 "C:\Program Files\CodePreview\CodePreview.dll"
```

This will register the preview handler in your system for `*.txt`, `*.h` and `*.cpp` files. The previewer
is used by Windows Explorer (with the preview pane active) and Outlook.

You can also use the prebuilt dlls available in the folder `bin`.

To unregister it, you should run
```
regsvr32 /u "C:\Program Files\CodePreview\CodePreview.dll"
```

To change the highlighting style for the previewd files, you should modify the file
`CodePreview/Styles.cpp`, changing `lexCpp` and/or `keywords_CPP` global variables.


## Note

Scintilla has lexers for 100+ languages (see `/scintilla/lexers/`). The previewer can be expanded
for any of those languages (you just need to create the theme and register it for the new file types).
Also, the previewer can be expanded with new languages, by creating new lexers.
