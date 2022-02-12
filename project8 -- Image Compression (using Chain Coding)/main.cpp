#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cmath>
using namespace std;

/*
Project 8: (C++) You are to implement the Chain code algorithm and reconstruct the object from the resulting chain code.
In this project, you may assume that the input binary image contains only one object without holes.
There are three components in this project:
1) Image compression: apply chain code algorithm to a binary input file to produce chain code of the object boundary.
2) Boundary reconstruction: Reconstruct the boundary of the object from the chain code.
3) Fill the interior of the object: implement the two interior filling algorithms given in the lecture note.
a) Method1: the 3-pass interior filling algorithm may not work on all shapes of objects, such as the object in img2.
** You are encouraged to modify the algorithm to make it works.
b) Method2: The 2nd algorithm in the lecture note, you are to come up with your idea as how to determine a zero
pixel to the right of a nonzero pixel is an interior or an exterior pixel of the object, do your best!
*** You will be given 2 data files: img1 and img2. img1 contains a simple straight boundary object; img2 contains an
irregular shape object. What to do as follows:
1) Implement your program based on the specs given below.
2) Run and debug your program using method1 with img1 until your program produces the reconstructed file is
identical to img1.
3) Run and debug your program using method2 with img1 until your program produces the reconstructed file is
identical (or almost identical) to img1.
4) Run and debug your program using method1 with img2 until your program produces the reconstructed file is
"almost" identical to img2.
5) Run and debug your program using method2 with img2 until your program produces the reconstructed file is
"almost" identical to img2.
============================================
Include in your hard copies:
- cover page
- source code
- Pretty Print img1
- Print chainCodeFile for img1
- Print boundaryFile for img1
- Print deCompressedFile for img1 using method1
- Print deCompressedFile for img1 using method2
- Pretty Print img2
- Print chainCodeFile for img2
- Print BoundaryFile for img2
- Print deCompressedFile for img2 using method1
- Print deCompressedFile for img2 using method2
**************************************
Language: C++
**************************************
Project points:12 pts
Due Date: Soft copy (*.zip) and hard copies (*.pdf):


*** Name your soft copy and hard copy files using the naming convention as given in the project submission requirement.
*** All on-line submission MUST include Soft copy (*.zip) and hard copy (*.pdf) in the same email attachments with
correct email subject as stated in the email requirement; otherwise, your submission will be rejected.

2

**************************************
I. Inputs: There are two inputs
1) a binary image (use argv [1]): contain only one object without hole.
2) whichMethod (use argv [2]): // 1 or 2. 1 means use method1; 2 means use method2
**************************************
II. Outputs: There are four outFiles
**************************************
a) chainCodeFile (not from argv []): To store the chain code of the object in the following format:

numRows numCols minVal maxVal // image header use one text line
Label startRow startCol // use one text line
code1 code2 code3 .... // All in one text line, with one blank space between codes.
// In real life, each chain code (0 to 7) only use 3 bits and without blank spaces between codes!
b) boundaryFile (not from argv[]): // An image file for the restored boundary of the object from chain code.
c) deCompressedFile (not from argv []): //An image file for the deconstructed image using both algorithms with
captions.
d) outFile // for prettyPrint input image.
*/

class point{
    public:
    int row, col;

    point(){
        row = -1;
        col = -1;
    }

    point(int r, int c){
        row = r;
        col = c;
    }

};

class ChainCode{

    public:

    // Class Variables:

    ifstream inFile;    // variables for file input/output
    fstream chainCodeFile, boundaryFile, deCompressedFile,outFile;

    int numRows, numCols, minVal, maxVal; // variables for initial image

    int** imgAry;   // 2D arrays to store images. will be padded with 1pixel to each side
    int** reconstructAry;

    bool open = false;

    int startRow, startCol, label, lastQ, chainDir; // Integers necessary to create chainCode
    point startP, currP;    // Point holds row, col.
    point neighborCoord[8]; 
    int zeroTable[8] = {6,0,0,2,2,4,4,6}; 

    // Constructor
    ChainCode(string fname, int method){

            // open input file
        inFile.open(fname);

            // read from inFile
        inFile >> numRows >> numCols >> minVal >> maxVal;
        
            // dynamically create 2D arrays
        reconstructAry = new int*[numRows+2];   
        imgAry = new int*[numRows+2];
        for(int i =0; i<numRows+2; i++){
            reconstructAry[i] = new int[numCols+2];
            imgAry[i] = new int[numCols+2];
        }
            // set array values to zero
        for(int i =0; i<numRows+2; i++){
            for(int j=0; j<numCols+2; j++){
                reconstructAry[i][j]=0;
                imgAry[i][j]=0;
            }
        }

            // load the image and print it
        loadImage();
        outFile.open((fname.substr(0,fname.length()-4) + "_outFile.txt"),ios::out);
        reformatPrettyPrint(imgAry, &outFile);
        outFile.close();

            // part 1: compress file into chain code
        chainCodeFile.open((fname.substr(0,fname.length()-4) + "_chainCode.txt") , ios::out);
        getChainCode();
        chainCodeFile.close();

            // part 2: construct boundary
        boundaryFile.open((fname.substr(0,fname.length()-4) + "_boundary.txt"),ios::out);
        chainCodeFile.open((fname.substr(0,fname.length()-4) + "_chainCode.txt"), ios::in);
        constructBoundary();
        ary2File(reconstructAry, &boundaryFile);
        boundaryFile.close();
        chainCodeFile.close();

            // part 3: decompress file via method1 or method2
        deCompressedFile.open((fname.substr(0,fname.length()-4) + "_deCompressed.txt"),ios::out);
        if (method == 1) method1();
        else if (method == 2) method2();
        else{ cout << "ERROR: can only put a '1' or a '2' for argument 2!"; exit(EXIT_FAILURE); }
        ary2File(reconstructAry, &deCompressedFile);
        deCompressedFile.close();


    }


    // Methods Part 1 ::: Compression

    void getChainCode(){

        // find starting point
        findFirstP();

        // output header and chain info 
        chainCodeFile << numRows << " " << numCols << " " << minVal << " " << maxVal << "\n";
        chainCodeFile << label << " " << startP.row << " " << startP.col << " ";

        // prepare to enter loop
        currP.row = startP.row;
        currP.col = startP.col;
        lastQ = 4;
        // main loop
        do{
            lastQ = mod(++lastQ,8);
            chainDir = findNextP();
            chainCodeFile << chainDir << " ";
            currP = neighborCoord[chainDir];    // move to next point
            lastQ = zeroTable[mod(chainDir-1,8)];

        }while(currP.row != startP.row || currP.col != startP.col);    // will exit when we are at start point

    }

    void findFirstP(){

        for(int i=1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){

                if (imgAry[i][j] > 0){  // found first non-zero pixel , trace its border

                    // obtain label and start position
                    label = imgAry[i][j];
                    startP.row = i;
                    startP.col = j;
                    return;
                }
            }
        }

    }

    int findNextP(){

        loadNeighborsCoord();   // 
        int index = lastQ;  // start from last zero 
        int row, col;

        for(int i = 0; i<8; i++){


            row = neighborCoord[index].row;
            col = neighborCoord[index].col;

            if(imgAry[row][col] == label) return index; // return first match we see

            index = mod(++index,8);
        }

        cout << "Error, did not find a proper chain direction!\n";
        return -1;

    }

    void loadNeighborsCoord(){

        // just hard coded where the neighbors would be for currP

        neighborCoord[0].row = currP.row;   // right
        neighborCoord[0].col = currP.col+1;

        neighborCoord[1].row = currP.row-1; // top right
        neighborCoord[1].col = currP.col+1;

        neighborCoord[2].row = currP.row-1; // top middle
        neighborCoord[2].col = currP.col;

        neighborCoord[3].row = currP.row-1; // top left
        neighborCoord[3].col = currP.col-1;

        neighborCoord[4].row = currP.row;   // left
        neighborCoord[4].col = currP.col-1;

        neighborCoord[5].row = currP.row+1; // bottom left
        neighborCoord[5].col = currP.col-1;

        neighborCoord[6].row = currP.row+1; // bottom middle
        neighborCoord[6].col = currP.col;

        neighborCoord[7].row = currP.row+1; // bottom right
        neighborCoord[7].col = currP.col+1;

        
    }

    // Methods Part 2 ::: Boundary Construction

    void constructBoundary(){

            // read from chainCodeFile
        chainCodeFile >> numRows >> numCols >> minVal >> maxVal;        
        chainCodeFile >> label >> currP.row >> currP.col;

        int next;
        while(!chainCodeFile.eof()){
            reconstructAry[currP.row][currP.col] = label;
            chainCodeFile >> next;
            goToNextPoint(next);
        }
    }
    
    void goToNextPoint(int i){
        switch(i){
            case 0:
                currP.row = currP.row;
                currP.col = currP.col+1;
                break;
            case 1:
                currP.row = currP.row-1;
                currP.col = currP.col+1;
                break; 
            case 2:
                currP.row = currP.row-1;
                currP.col = currP.col;
                break;
            case 3:
                currP.row = currP.row-1;
                currP.col = currP.col-1;
                break;
            case 4:
                currP.row = currP.row;
                currP.col = currP.col-1;
                break;
            case 5:
                currP.row = currP.row+1;
                currP.col = currP.col-1;
                break; 
            case 6:
                currP.row = currP.row+1;
                currP.col = currP.col;
                break;
            case 7:
                currP.row = currP.row+1;
                currP.col = currP.col+1;
                break;  
        }
    }

    // Methods Part 3 ::: Decompression
    void toggle(){
        open = !open;
    }

    bool isOpeningBoundary(int i, int j){
        if(reconstructAry[i][j-1] > 0 || reconstructAry[i-1][j-1] > 0 || reconstructAry[i-1][j] > 0 || reconstructAry[i-1][j+1] > 0 ){
            return true;
        }         
        else return false;
    }

    bool isClosingBoundary(int i, int j){
        if(reconstructAry[i][j+1] > 0 || reconstructAry[i+1][j+1] > 0 || reconstructAry[i+1][j] > 0 || reconstructAry[i+1][j-1] > 0 ){
            return true;
        } 
        else return false;
    }

    void method1(){
        // 3-pass algorithm

            // 1st pass
        for(int i =1; i<=numRows; i++){ // scan img ary, row-by-row from T to B


            // mark all the boundary pixels with zero-neighbors on left and right
            for(int j =1; j<=numCols; j++){
                 if (reconstructAry[i][j]>0 && reconstructAry[i][j-1] == 0 && reconstructAry[i][j+1] == 0){
                    reconstructAry[i][j] = label+2;  // mark it as label + 2
                 }
            } // end marking loop


            // increment all pixels between 2 marked boundary pixels on the same row by 1
            if(open) toggle();  // start this row closed

            for(int j=1; j<=numCols;j++){
                if(reconstructAry[i][j] == label+2) toggle();   // if we found a mark:  open or close
                else if(open) ++reconstructAry[i][j];  // increment this pixel by 1
            } // end increment loop
        
        }// end first pass

            // 2nd pass
        for(int j =1; j<=numCols; j++){ // scan img ary, col-by-col from L to R

            // mark all the boundary pixels with zero-neighbors on top and bottom
            for(int i=1; i<=numRows; i++){
                 if (reconstructAry[i][j]>=label && reconstructAry[i-1][j] <=1 && reconstructAry[i+1][j] <=1  ) reconstructAry[i][j] = label+2;  // mark it as label + 2
            } // end marking loop


            // increment all pixels between 2 marked boundary pixels on the same row by 1
            if(open) toggle();  // start this col closed

            for(int i=1; i<=numRows;i++){
                if(reconstructAry[i][j] == label+2) toggle();   // if we found a boundary mark
                else if(open) ++reconstructAry[i][j];  // increment this pixel by 1
            } // end increment loop

        }// end second pass

            // 3rd pass : threshold
        for(int i =1; i<= numRows; i++){
            for(int j =1; j<= numCols; j++){
                if (reconstructAry[i][j]<2) reconstructAry[i][j] = 0;
                else reconstructAry[i][j] = label; 
            }
        } // end 3rd pass
    
    }

    void method2(){

        // 1-pass from lecture note

        for(int i =1; i<=numRows; i++){ // step 1: Scan img array, L-R and T-B

                // step 2: skip 0s until first boundary pixel
            int j =1;
            while(j<=numCols){
                if(reconstructAry[i][j] > 0 && isOpeningBoundary(i,j)) break;
                j++;
            } 
            if(j<=numCols) {
                reconstructAry[i][j] = label;
            }
 
                // step 3: p is a boundary pixel
            while(j<numCols && reconstructAry[i][j+1]>0) j++;    

                
                // step 4: skip boundary pixels until interior pixel or end of row
            while(j<numCols ){ 
                j++;
                if(reconstructAry[i][j] > 0 && isClosingBoundary(i,j)) break;
                reconstructAry[i][j] = label;
            }

        }// repeat for all rows

    }


    // Methods ::: utility

    int mod(int n, int mod){
        int temp = n;
        while(temp < 0){
            temp += mod;
        }
        return temp%mod;
    }

    void ary2File(int** ary, fstream* outFile){

        // output inside frame of ary to file, use reformatPrettyPrint.
        for(int i =1; i<=numRows; i++){
            if(i>1) *outFile << "\n";
            for(int j=1; j<=numCols; j++){
                if(ary[i][j] > 0){
                    if (ary[i][j] < 10){    // 2 padded spaces
                        *outFile << ary[i][j] << "   ";
                    }
                    else if(ary[i][j] < 100){   // 1 padded space
                        *outFile << ary[i][j] << "  ";
                    }
                    else{   // no spaces
                        *outFile << ary[i][j];
                    }
                }
                else *outFile << "0   ";

            }
        }
    }

    void loadImage(){
        // Load input onto inside frame of ZFAry.
        for(int i=1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){
                inFile >> imgAry[i][j];
            }
        }
    }

    void reformatPrettyPrint(int** ary, fstream* outFile){

        for(int i =1; i<=numRows; i++){
            if(i>1) *outFile << "\n";
            for(int j=1; j<=numCols; j++){
                if(ary[i][j] > 0){
                    if  (ary[i][j] < 10)       *outFile << ary[i][j] << "   ";  // 2 padded spaces
                    else if(ary[i][j] < 100)   *outFile << ary[i][j] << "  ";   // 1 space
                    else                       *outFile << ary[i][j];   // no space
                }
                else *outFile << ".   ";

            }
        }
    }

};


int main(int argc, char* argsv[]){

    if (argc != 3){	// If not the correct amount of arguments
        cout << "Error: \n Expected 2 arguments" << endl << "Received " << argc-1 << " arguments" << endl;
        return -1;
    }

    int method = stoi(argsv[2]);

    ChainCode* c = new ChainCode( argsv[1], method );
}
