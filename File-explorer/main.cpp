#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include <fcntl.h>
#include<dirent.h>
#include<termios.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>
#include<string>
#include <pwd.h>
#include<grp.h>
#include <sys/ioctl.h>
#include<iostream>
#include<vector>
#include<stack>
#include<sstream>
#include<string>
#include<cstring>
#include <sys/wait.h>
using namespace std;

#define DEF_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK S_IWGRP|S_IWOTH

#define clr printf("\033[H\033[J");
#define cursorforward(x) printf("\033[%dC", (x))
#define cursorbackward(x) printf("\033[%dD", (x))
#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108

void copy_files(vector<string> srcFiles, string destination);
void disableNonCononicalMode();
string getAbsolutePath(string inp);
void displayDirectory();
int isRegularFile(string str);
int isDirectory(string str);

vector<string> fileList;
stack<string> bStore;
stack<string> fStore;

int rowCount;
int colCount;

DIR *dir;
struct dirent *dRead;

//to store global path
string path ="";
string currPath = "";
string destPath = "";

//Paths 1). 2)./ 3)~/

	
// cannot exceed normalRows count in any case
	int normalRows;
// keeping track of movements	
	int trackMoves;

// keep track of first file printed on terminal and last file printed on terminal
	int firstFilePos=0;
	int lastFilePos=0;

// no of files
	int noOfFiles = 0;


struct stat buf;

struct termios initialTermios,nonCanonTerm;

void getTokens(string line,vector<string> &tokens){

//cout<<"*****"<<endl;
    stringstream convert(line); 
      
    string intermediate; 
      
    // Tokenizing w.r.t. space ' ' 
    while(getline(convert, intermediate, ' ')) 
    { 
        tokens.push_back(intermediate); 
    } 
	

}

int isRegularFile(string listP)
{
	char tmp[listP.length()];
	strcpy(tmp,listP.c_str());
    struct stat list_stat;
    stat(tmp, &list_stat);
    return S_ISREG(list_stat.st_mode);
}

int isDirectory(string listP) 
{
	char tmp[listP.length()];
	strcpy(tmp,listP.c_str());
    struct stat list_stat;
    if (stat(tmp, &list_stat) != 0)
       return 0;
    return S_ISDIR(list_stat.st_mode);
}


void copy_directory(string src, string dest){ // dest -- recieved full path

	DIR* copyDirectory = opendir(src.c_str());
		if(!copyDirectory){
			printf("Error opening folder\n");
// copy files to destination directory
// use .c_str to convert from string to ctype string
			disableNonCononicalMode();
			exit(1);
		}
		// also check if directory is empty
		string copyDestPath = destPath;
		string copySrcPath = currPath;
		while((dRead = readdir(copyDirectory)) != NULL){ //maintaining a pointer
			currPath = src;
			destPath = dest;
			vector<string> vsrc = {dRead->d_name};
			copy_files(vsrc,destPath);
			currPath = copySrcPath;
			destPath = copyDestPath;
					
	}

}

// copy files to destination directory
// use .c_str to convert from string to ctype string

//if it's a directory copy directory.. for that make a recursive call on function. 

void copy_files(vector<string> srcFiles,string dest ){

	// reset at last
	string copyDestPath = destPath;
	string copySrcPath = currPath;

	//--------------------------------
	struct stat src_stat,dest_stat;

	string currDest = dest;  // format path // assuming full path
	char src_data[2048];
	//open destination directory
	// copy given file to this directory
	dir = opendir(currDest.c_str());
	int src_fd, dest_fd;

	if(dir){
		for(auto x: srcFiles){ // file1 Testing
			// string src_path = x.c_str();
			if( x == "." || x == ".."){continue;}
			if(isDirectory(currPath+"/"+x)){

				//dest me directory1 k name se directory create....
				//cout<<"Destiii :"<<dest<<endl;
				int check = mkdir((currDest+"/"+x).c_str(), 0756);
				// int check = mkdir((x).c_str(), 0756);
				if(check == -1){
				//	cout<<" Path was : "<<(currDest+"/"+x)<<endl;
				//	cout<<"Error 1"<<endl;
					disableNonCononicalMode();
					exit(1);
				}else{
					// copy_directory(currPath+"/"+x,currDest);
					copy_directory(currPath+"/"+x,(currDest+"/"+x));

				}

			
				}
			else{
					src_fd = open((currPath + "/"+x).c_str(), O_RDONLY, 0);

					int n = read(src_fd,src_data,1024);
					dest_fd = open((currDest+"/"+x).c_str(), O_WRONLY|O_CREAT, DEF_MODE);
					// read(src_fd, src_data, n);
					
					if(n > 0){

						write(dest_fd,src_data,n);
						

						
					}
			}
		

		}
	}
	
	// --------------------------------------------
	currPath = copySrcPath;
	destPath = copyDestPath;
	// --------------------------------------------
	
}

void renameFile(string file1, string file2){
	string str1 = getAbsolutePath(file1);
	string str2 = getAbsolutePath(file2);
	rename(str1.c_str(),str2.c_str());
}

string getAbsolutePath(string inp){
	 string ret="";
	 //absolute
	 
	 if((inp[0]>='a' and inp[0]<='z') or(inp[0]>='A' and inp[0]<='Z' )){
	 	ret=currPath+string("/")+inp;
	 }
	 else if(inp[0]=='~') {
		 ret=path+inp.substr(1,inp.length());
	 
	 }
	 else if(inp[0]=='/'){
		 ret=path+inp;
	 
	 }
	 
	 else if(inp[0]=='.'){
	 
	 
	 ret=currPath+inp.substr(1,inp.length());

 
 }
 

 return ret;
}

void clearFowardStack(){

	while(!fStore.empty()){
		fStore.pop();
	}
}

void clearBackwardStack(){
	while(!bStore.empty()){
		bStore.pop();
	}
}

void mygoto(string i)
{
	string myp=getAbsolutePath(i);
	bStore.push(currPath);
	clearFowardStack();
	currPath=myp;

}

void runCommand(string line){

	//getline(cin,line,'\n');
	printf("%s\n", line.c_str());
	vector<string> tokens;
	getTokens(line,tokens);
	int len = tokens.end() - tokens.begin();

	if(tokens[0] == "copy"){
		vector<string> srcFiles;
		copy(tokens.begin()+1,tokens.end()-1,back_inserter(srcFiles));
		//copy_files(srcFiles,(currPath+string("/")+tokens[len -1]));
		// copy file1 Testing Reference 
	}
	else if(tokens[0] == "create_file"){
		// go to createFile();

	}
	else if(tokens[0] == "create_dir"){

	}else if(tokens[0] == "rename"){
		renameFile(tokens[1],tokens[2]);

	}else if(tokens[0] == "goto"){
		mygoto(tokens[1]);

	}else if(tokens[0] == "delete_file"){

	}else{

	}

}


void clearCommand(){
	int cmdPos = rowCount-2;
	printf("%c[%d;%dH",27,cmdPos,1);
	printf("%c[2K", 27);
	gotoxy(rowCount-2,1);
	printf("%s\n", ": ");
}

void gotoCommandMode(){

	//add dynamically
	// gotoxy(30,1);
	// gotoxy(30,1);
	string temp = currPath;
	gotoxy(rowCount-1,1);
	printf("%s", ": ");

	string line = "";
	char ch;


do{
    //clearCommand();
    ch = cin.get();
	

    if(ch==10) //enter
    {
        clearCommand();
        runCommand(line);
        line = "";
        
    }/*else if(ch==127) // backspace
    {

        clearCommand();
        if(line.length()==1)
        {
            line="";
        }
        else
        {
            line=line.substr(0,line.length()-1);
        }
        
        
        //cout<<line;

    }*/
	else if(ch!=27){
        line+=ch;
        cout<<ch;

    }
    

 }while(ch!=27);


 	int cmdPos=rowCount+1;
	printf("%c[%d;%dH",27,cmdPos,1);
	printf("%c[2K", 27);
	currPath=temp;
	fileList.clear();
	displayDirectory();

	

	// switch(tokens[0]){
		
	// 			case "copy":{
	// 						vector<string> srcFiles;
	// 						copy(tokens.begin()+1,tokens.end()-1,back_inserter(srcFiles));
	// 						copy_files(srcFiles,tokens[len -1]);

	// 						//copy_files(tokens);
							
	// 						break;
	// 					}

	// 		}
	}








void disableNonCononicalMode(){
	  tcsetattr(fileno(stdin), TCSAFLUSH, &initialTermios);
}



void displayStatFile(string fileStr){

	// string str[5];
	// str[0] -- file_name, str[1] -- file_size, str[2] -- permissions, str[3] -- user_name, str[4] -- user_group, str[5] -- last_modified
	// str[0] = fileStr;
	// cout<<
	string currFile = fileStr;
	char fileArr[currFile.length()];
	strcpy(fileArr,currFile.c_str());

	fileStr = currPath + string("/") + fileStr;
	char tmp[fileStr.length()];
	strcpy(tmp,fileStr.c_str());




	stat(tmp,&buf);   //buf contains struct for stat
	char moditfiedTime[100];


	//change below code
	struct passwd *paswd = getpwuid(buf.st_uid);
	struct group *grp = getgrgid(buf.st_gid);




	printf((S_ISDIR(buf.st_mode)) ? "d" : "-");
	printf((buf.st_mode & S_IRUSR) ? "r" : "-");
	printf((buf.st_mode & S_IWUSR) ? "w" : "-");
	printf((buf.st_mode & S_IXUSR) ? "x" : "-");
	printf((buf.st_mode & S_IRGRP) ? "r" : "-");
	printf((buf.st_mode & S_IWGRP) ? "w" : "-");
	printf((buf.st_mode & S_IXGRP) ? "x" : "-");
	printf((buf.st_mode & S_IROTH) ? "r" : "-");
	printf((buf.st_mode & S_IWOTH) ? "w" : "-");
	printf((buf.st_mode & S_IXOTH) ? "x" : "-");


	printf("\t%.1fK",((double)buf.st_size)/1024);

	if(paswd != 0)
		printf("\t%s", paswd->pw_name);
	if(grp != 0)
		printf("\t%s", grp->gr_name);

	// printf("st_mode = %o\n",buf.st_mode);
	
	strcpy(moditfiedTime,ctime(&buf.st_mtime));
	moditfiedTime[strlen(moditfiedTime)-1] = '\0';

	// printf("%-10s", moditfiedTime );
	printf("\t%s", moditfiedTime);

	printf("\t%s\n", fileArr);

	// cout<<fileStr<<endl;

}



void displayWhileScrolling(int k){
	// display from firstFilePos to lastFilePos

	clr;


	if( k == -1){
		firstFilePos--;
		lastFilePos--;
	}else{
		firstFilePos++;
		lastFilePos++;
	}
	// printf("test ");


	// dir = opendir(".");
	string str = currPath;
	char tmp[str.length()];
	strcpy(tmp,str.c_str());

	// dir = opendir(currPath.c_str());
	dir = opendir(tmp);

	if(!dir){
		printf("Error opening folder\n");
		exit(1);
	}

	int count = 1;

	while((dRead = readdir(dir)) != NULL){ //maintaining a pointer

			// printf("test 2\n");
		if((count >= firstFilePos) && (count <= lastFilePos)){

			count++;

			
			displayStatFile(dRead->d_name);
			// printf("\t%s", dRead->d_name);

		}else{
			count++;
		}
		
		
		// enableNonCanonicalMode();
	}

	//if(lastFilePos > )

	if(k == -1){
		gotoxy(1,1);
	}else{
		// gotoxy(1,normalRows);
		gotoxy(normalRows,1);
	}

	closedir(dir);


}

void enableNonCanonicalMode(){
	tcgetattr(fileno(stdin), &initialTermios);

	nonCanonTerm = initialTermios;


	// turing off ECHOing , diable canocal mode -- ~ICANON
	nonCanonTerm.c_lflag = nonCanonTerm.c_lflag & ~(ECHO | ICANON);
	// nonCanon.c_lflag & = ~(ECHO | ICANON);
	tcsetattr(fileno(stdin), TCSAFLUSH, &nonCanonTerm);
}

void displayDirectory(){

	printf("\033[H\033[J");

	struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

   	rowCount = w.ws_row;
    colCount = w.ws_col;
    // printf("%d%d\n",rowCount,colCount );

    normalRows = (rowCount + 1)/2;
    trackMoves = normalRows;
 
	char tmp[currPath.length()];
	strcpy(tmp,currPath.c_str());
	dir = opendir(tmp);

	if(!dir){
		printf("Error opening folder\n");
		exit(1);
	}

	// dRead = readdir(dir);


	int counter = 0; // counter wont let no of prints exceed normalRows
	noOfFiles = 0; // keep track of no of folders/files in

	while((dRead = readdir(dir)) != NULL){ //maintaining a pointer

		if(counter < normalRows){
			string str = string(dRead->d_name);
			char tmp[str.length()];
			strcpy(tmp,str.c_str());
			displayStatFile(tmp);
			fileList.push_back(str);
			
			// displayStatFile(dRead->d_name);
		//	printf("\t%s", dRead->d_name);

			noOfFiles++;
			counter++;	
			//push all files to vector
			// fileList.push_back(dRead->d_name);
		}else{
			noOfFiles++;
			//push all files to vector
			string str = dRead->d_name;
			fileList.push_back(str);
			// fileList.push_back(dRead->d_name);
		}
		
		// enableNonCanonicalMode();
	}

	firstFilePos = 1;
	lastFilePos = counter;
	// printf("%d\n",counter);

	if( counter < normalRows){
		trackMoves = counter;
		normalRows = counter;
	}

	// gotoxy(0,trackMoves);
	gotoxy(trackMoves,0);

	// enableNonCanonicalMode();
	closedir(dir);
	//char c;
	//while (read(fileno(stdin), &c, 1) == 1 && c != 'q');

}

void openAtPos(){
	// if currently you are at file open content in vi else display the whole directory plus push path to stack
	string fileName;
	// current file number in vector fileList
	int loc = firstFilePos + trackMoves -2;
	//cout<<"First "<<firstFilePos<<"current "<<trackMoves<<endl;

	fileName = fileList[loc];
	string origPath = currPath;

	//currPath = currPath + "/" + fileName;
	// cout<<currPath<<endl;

	//check if directory

	if(isDirectory((currPath+string("/")+fileName))){
		currPath = currPath + string("/") + fileName;
		// open directory , 1. push in bstore 2. call display to display on screen
		bStore.push(origPath);
		fileList.clear();
		displayDirectory();

	}else if(isRegularFile((origPath+string("/")+fileName))){
		pid_t pid = fork();
		if(pid == 0){
			execl("/usr/bin/vi","vi",(origPath+string("/")+fileName).c_str(),NULL);
			exit(1);
		}
		wait(NULL);
	}

//	currPath = origPath;
	
}

void handleScrolling(){
	int c;

	while(1){
		// cin.get(c,1);
		c = cin.get();
		// cout<<" Test "<<c<<endl;
		if(c == 27){
			// cout<<" HI "<<c<<endl;
			c = cin.get();
			c = cin.get();
			// cin.get(c,1);
			// cin.get(c,1);
			switch(c){
				case 'A':{
							if(trackMoves > 1 && trackMoves <= normalRows){
								printf("\033[%dA", (1));

								trackMoves--;
								//printf("%d A",trackMoves );

							}
							
							break;
						}
				case 'B':{
							if((trackMoves > 0) && (trackMoves < normalRows)){
								printf("\033[%dB", (1));
								trackMoves++;
								//printf("%d B",trackMoves );
							}
							
							break;

						}
				case 'C':{
							// Go right
					// cout<<"forward "<<currPath<<endl;
							if(!fStore.empty()){
								bStore.push(currPath); 
								currPath = fStore.top();
								fStore.pop();
								// if we wont to come back one step
								fileList.clear();
								displayDirectory();

							}

							break;
						}
				case 'D':{
							 // left arrow
							if(!bStore.empty()){
								fStore.push(currPath);
								currPath = bStore.top();
								bStore.pop();
								fileList.clear();
								displayDirectory();

							}

							break;
						}
				default:{
					  		//cout<<"SWITCH "<<c<<endl;
				}		
			}
			// printf("%c\n", c);


		}else if( c == 'q'){
			//cout<<" 1 "<<endl;
			disableNonCononicalMode();
			exit(1);

		}else if( c == 'k'){ //scroll up
			// get dimensions, display whole structur again
			//cout<<" 2 "<<endl;
			if(firstFilePos > 1 && (trackMoves == 1)){
				//display again
				//printf("%d k",trackMoves );
				displayWhileScrolling(-1);
			}


		}else if( c ==  'l'){ //scroll down
			// get dimensions, display whole structur again
			//cout<<" 3 "<<endl;
			if((lastFilePos < noOfFiles) && (trackMoves == normalRows)){
				//display again
				//printf("%d l",trackMoves );
				displayWhileScrolling(1);
			}

		}else if(c == 'H' || c == 'h'){ // Home
			// 104  72
			// cout<<" IN "<<endl;
			bStore.push(currPath);
			currPath = path;
			clearFowardStack();
			fileList.clear();
			displayDirectory();

		}else if(c == 10){ // Enter
			//cout<<" 4 "<<endl;
			//cout<<"opening"<<endl;
			// fStore.clear();
			clearFowardStack();
			// bStore.push(currPath); 
			// fileList.clear();
			openAtPos();// if we wont to come back one step
			
			

		}else if( c == 58){  // :
			//cout<<" 5 "<<endl;
			clearCommand();
			gotoCommandMode();
		}else if(c == 127){ //backspace (test)
			//cout<<" 6 "<<endl;
			
			string temp=currPath;
			int n=temp.size()-1;
			int f;
			for(int i=n;i>=0;i--)
			{
				if(temp[i]!='/')
					continue;
				else
				{
					f=i;
					break;
				}
			}
		
			string ob=temp.substr(0,f);
			clearFowardStack();
			bStore.push(currPath);
			currPath=ob;
			fileList.clear();
			displayDirectory();

	}
}
}

int main(int argc, char const *argv[])
{

	
    char mr[1024];
    getcwd(mr,1024);
    path=string(mr);
    currPath=string(path);
	//path = "/home/amisha/Desktop";
	bStore.push(path);

	currPath = path;

	printf("\033[H\033[J");
	enableNonCanonicalMode();
	displayDirectory();
	// gotoxy(0,trackMoves);
	gotoxy(trackMoves,0);

	handleScrolling();
	disableNonCononicalMode();
	

	return 0;
}