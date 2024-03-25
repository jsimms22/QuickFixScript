# QuickFixScript

Program to handle a multitude of specific, repetitive tasks for someone who needed help and did not have time to rebuild a website from scratch. You know who you are.

Note to that person: Use Visual Studio to build the program source (if you need to), targeting release and x64

In short, the program will handle these tasks for all .pdfs in a given directory:
1) Verify the file is acceptable to use
2) Reset loop variables
3) Retrieve the ID of the file from the database, if we find an ID do the following:
   - Execute query to UPDATE Volume_Number for the given ID
   - Execute query to UPDATE NumIssue for the given ID
   - Execute query to UPDATE citationString for the given ID
   - Execute query to UPDATE Published_PDF_File for the given ID
   - If we updated the published pdf filepath then set pub_path_updated = true;
 4) If pub_path_updated = true then,
   - Update the filename in file explorer to match the new Published_PDF_File field in the database
   - If we updated the filename then set local_path_updated = true   
 5) if local_path_updated = true the,
   - Update specified fields in the respective .rdf for the .pdf
