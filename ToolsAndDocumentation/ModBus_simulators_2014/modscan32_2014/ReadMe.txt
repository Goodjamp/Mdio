
ModScan should be installed in a write-enabled folder.  The license information
as well as the last configured settings are stored in a .cfg file within
the install folder.  If this .cfg file cannot be written, for any reason, 
ModScan will not be able to retain the license info and will prompt for it
again the next time it starts up.


Beginning with Windows 7, any application installed under the "Program Files"
section of the Windows Directory now requires Administrator Rights in order to
write to the folder.

The best solution is to create a folder directly off the Directory root such as:

C:\ModScan

Copy all files from the ModScan32.zip, (or ModScan64.zip), into the newly
created folder and double-click on the .exe to start.  Enter your license
information and wait for the "Thank You For Registering" dialog box.  Close the
application and verify that ms32frm.cfg, (or ms64frm.cfg), was created.  If so,
the application will be successfully licensed and the user information retained.

