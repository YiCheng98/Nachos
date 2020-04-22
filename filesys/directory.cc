// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "filesys.h"
#include "directory.h"
#define NumDirEntries 		10
extern FileSystem  *fileSystem;
//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

void DirectoryEntry::GetName(char *into){
	strcpy(into, name);
	if (fileNameLen > FileNameMaxLen){
		fileSystem->extNameFile->ReadAt((char *)into + FileNameMaxLen, fileNameLen - FileNameMaxLen,0);
	}
}

void DirectoryEntry::SetName(char *into){
	fileNameLen = strlen(into);
	if (fileNameLen > FileNameMaxLen){
		for (int i=0;i<FileNameMaxLen;i++)
			name[i]=into[i];
		extNamePos = fileSystem->extNameFile->GetPosition();
		fileSystem->extNameFile->Write((char *)into + FileNameMaxLen, fileNameLen-FileNameMaxLen);
	}
	else{
		strcpy(name,into);
	}
}

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = FALSE;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file){
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++){
        if (table[i].inUse){
			int len=strlen(name);
			if (table[i].fileNameLen != len)
				continue;		
			else{
				char *tmpname = new char(len);
				table[i].GetName(tmpname);
				if (!strncmp(tmpname, name, FileNameMaxLen))
					return i;
			}
		}
	}
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);
    if (i != -1)
		return table[i].sector;
    return -1;
}
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
	return FALSE;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = TRUE;
			table[i].SetName(name);
            table[i].sector = newSector;
			table[i].type = 'F';
        return TRUE;
	}
    return FALSE;	// no space.  Fix when we have extensible files.
}

bool Directory::AddDir(char *name, int DirectorySector){ 
    if (FindIndex(name) != -1)
		return FALSE;
    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
			table[i].inUse = TRUE;
			table[i].SetName(name);
			Directory *directory = new Directory(NumDirEntries);
			OpenFile *directoryFile = new OpenFile(DirectorySector);
			directory->WriteBack(directoryFile);
			table[i].type = 'D';
			table[i].sector = DirectorySector;
			return TRUE;
		}
    return FALSE;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    return TRUE;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List(int preblank=0){
    char tmpName[256];
    char *prestr = new char(preblank+1);
	prestr[preblank]='\0';
    if (preblank){
		for (int i=0;i<preblank-2;i++)
			prestr[i]=' ';
		prestr[preblank-1] = '-';
		prestr[preblank-2] = '-';
    }
    prestr[preblank]='\0';
   for (int i = 0; i < tableSize; i++)
	if (table[i].inUse){
		if (table[i].type == 'F'){
			table[i].GetName(tmpName);
			printf("%sFILE: %s \n", prestr, tmpName);
		}
		else{
			table[i].GetName(tmpName);
			printf("%sDIRECTORY: %s\n", prestr, tmpName);
			OpenFile *currentDirFile = new OpenFile(Find(tmpName));
			Directory *currentDir = new Directory(NumDirEntries);
			currentDir->FetchFrom(currentDirFile);
			currentDir -> List(preblank + 10);
		}
	}
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
	    printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;
}
