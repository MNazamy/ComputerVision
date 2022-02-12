#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

class Morphology{

    public:

    int numImgRows, numImgCols, imgMin, imgMax; // Img vars
    int numStructRows, numStructCols, structMin, structMax; // struct vars
    int rowOrigin, colOrigin, rowFrameSize, colFrameSize;
    int extraRows, extraCols, rowSize, colSize;
    int **zeroFramedAry, **morphAry, **tempAry, **structAry;

	Morphology( ifstream* imgFile, ifstream* structFile, ofstream* dilateOutFile, ofstream* erodeOutFile, ofstream* openingOutFile, ofstream* closingOutFile, ofstream* prettyPrintFile){

        // reading img file data
        *imgFile >> numImgRows;  
        *imgFile >> numImgCols; 
        *imgFile >> imgMin;
        *imgFile >> imgMax;

        // reading struct file data
        *structFile >> numStructRows;   
        *structFile >> numStructCols;
        *structFile >> structMin;
        *structFile >> structMax;
        *structFile >> rowOrigin;
        *structFile >> colOrigin;

        // assigning numerical values
        rowFrameSize = numStructRows / 2;
        colFrameSize = numStructCols / 2;
        extraRows = rowFrameSize * 2;
        extraCols = colFrameSize * 2;
        rowSize = numImgRows + extraRows;
        colSize = numImgCols + extraCols;
        


        // building our 2D arrays using pointers and loops

            // builds the number of rows for each ary
        zeroFramedAry = new int* [rowSize];
        morphAry = new int* [rowSize];
        tempAry= new int* [rowSize];
        structAry = new int* [numStructRows];

            // builds the coloumns for each ary
        for(int i=0; i<rowSize; i++){
            zeroFramedAry[i] = new int[colSize];
            morphAry[i] = new int[colSize];
            tempAry[i] = new int[colSize];
        }

        for(int i =0; i<numStructRows; i++){
            structAry[i] = new int[numStructCols];
        }

        zero2DAry(zeroFramedAry,rowSize, colSize);
        loadImg(imgFile);
        *prettyPrintFile << "Unchanged Image On Framed Array\n";
        prettyPrint(zeroFramedAry, prettyPrintFile, rowSize, colSize);


        zero2DAry(structAry, numStructRows, numStructCols);
        loadStruct(structFile);
        *prettyPrintFile << "\n\nStruct Array\n";
        prettyPrint(structAry, prettyPrintFile, numStructRows, numStructCols);


        zero2DAry(morphAry, rowSize, colSize);
        computeDilation (zeroFramedAry, morphAry);
        aryToFile (morphAry, dilateOutFile); 

        *prettyPrintFile << "\n\nImage After Dilation\n";
        prettyPrint (morphAry, prettyPrintFile, rowSize, colSize); 


        zero2DAry(morphAry, rowSize, colSize);
        computeErosion (zeroFramedAry, morphAry ); 
        aryToFile (morphAry, erodeOutFile);

        *prettyPrintFile << "\n\nImage After Erosion\n";
        prettyPrint (morphAry, prettyPrintFile, rowSize, colSize); 

        zero2DAry(morphAry, rowSize, colSize);
        computeOpening (); 
        aryToFile (morphAry, openingOutFile);
        *prettyPrintFile << "\n\nImage After Compute Opening\n";
        prettyPrint (morphAry, prettyPrintFile, rowSize, colSize); 

        zero2DAry(morphAry, rowSize, colSize);
        computeClosing (); 
        aryToFile (morphAry, closingOutFile);
        *prettyPrintFile << "\n\nImage After Compute Closing\n";
        prettyPrint (morphAry, prettyPrintFile, rowSize, colSize);



    }
    
    void zero2DAry(int** inAry, int nrows, int ncols) {
        // Set the entire Ary (nRows by nCols) to zero
        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                inAry[i][j] = 0;
            }
        }
    }

    void loadImg(ifstream* imgFile) {
        // load imgFile to zeroFramedAry inside of frame, begins at (rowOrigin, colOrigin)
        for(int i =0; i<numImgRows; i++){
            for(int j=0; j<numImgCols; j++){
                *imgFile >> zeroFramedAry[i+rowOrigin][j+colOrigin];
            }
        }
    }

    void loadStruct(ifstream* structFile) {
        // load structFile to structAry
        for(int i =0; i<numStructRows; i++){
            for(int j=0; j<numStructCols; j++){
                *structFile >> structAry[i][j];
            }
        }
    }

    void computeDilation(int** inAry, int** outAry) {
        // process every pixel in inAry, put result to outAry 
        for(int i =rowFrameSize; i<rowSize; i++){
            for(int j=colFrameSize; j<colSize; j++){
                if(inAry[i][j] > 0) onePixelDilation(i,j,inAry, outAry);
            }
        }
    }

    void onePixelDilation(int i, int j, int** inAry, int** outAry) {
           int iOffSet = i-rowOrigin;   // offset vars
           int jOffSet = j-colOrigin;
           bool matchFlag = true;
           int rIndex =0;       // row index 
           int cIndex=0;        // col index
           while(matchFlag && rIndex < numStructRows){  // while r < numRows
               while(matchFlag && cIndex < numStructCols){  // while c < numCols
                   if( (structAry[rIndex][cIndex] > 0) && (inAry[iOffSet + rIndex][jOffSet + cIndex] <= 0)) matchFlag = false;  // check matchFlag
                   cIndex++;
               }
               rIndex++;
           }
           if(matchFlag) outAry[i][j] = 1;
           else outAry[i][j] = 0;    
    }

    void computeErosion(int** inAry, int** outAry) {
        // process every pixel in inAry, put result to outAry 
        for(int i =rowFrameSize; i<rowSize; i++){
            for(int j=colFrameSize; j<colSize; j++){
                if(inAry[i][j] > 0) onePixelErosion(i,j,inAry, outAry);
            }
        }
    }

    void onePixelErosion(int i, int j, int** inAry, int** outAry) {

           int iOffSet = i-rowOrigin;   // offset vars
           int jOffSet = j-colOrigin;
           bool matchFlag = true;
           int rIndex =0;       // row index 
           int cIndex=0;        // col index
           while(matchFlag && rIndex < numStructRows){  // while r < numRows
               while(matchFlag && cIndex < numStructCols){  // while c < numCols
                   if( (structAry[rIndex][cIndex] > 0) && (inAry[iOffSet + rIndex][jOffSet + cIndex] <= 0)) matchFlag = false;  // check matchFlag
                   cIndex++;
               }
               rIndex++;
           }
           if(matchFlag) outAry[i][j] = 1;
           else outAry[i][j] = 0;

    }

    void computeClosing() {
        computeDilation(zeroFramedAry, tempAry);
        computeErosion(tempAry, morphAry);
    }

    void computeOpening() {

        computeErosion(zeroFramedAry, tempAry);
        computeDilation(tempAry, morphAry);

    }

    void aryToFile(int** inAry, ofstream* outFile) {

        int r = numImgRows;
        int c = numImgCols;
        *(outFile) <<  r << " " << c << " " << imgMin << " " << imgMax << endl;
        for(int i =rowFrameSize; i<rowSize; i++){
            for(int j=colFrameSize; j<colSize; j++){
                *(outFile) << inAry[i][j] << " ";
            }
            *(outFile) << "\n";
        }
    }

    void prettyPrint(int** inAry, ofstream* outFile, int r, int c) {

        for(int i=0; i<r; i++){
            for(int j=0; j<c; j++){
                if(inAry[i][j] == 0) {
                    *(outFile) << ". ";
                }
                else {
                    *(outFile) << inAry[i][j] << " ";
                }
            }
            *(outFile) << "\n";
        }
    }
};

int main(int argc, char* argsv[]){

    if (argc != 8){	// If not the correct amount of arguments
        cout << "Error: \n Expected 7 arguments" << endl << "Received " << argc << " arguments" << endl;
        return -1;
    }

    ifstream imgFile, structFile;
    ofstream dilateOutFile, erodeOutFile, openingOutFile, closingOutFile, prettyPrintFile;

    imgFile.open(argsv[1],ios::in);	// Open the files
    structFile.open(argsv[2], ios::in);
    dilateOutFile.open(argsv[3], ios::out);
    erodeOutFile.open(argsv[4], ios::out);
    openingOutFile.open(argsv[5], ios::out);
    closingOutFile.open(argsv[6], ios::out);
    prettyPrintFile.open(argsv[7], ios::out);
	
    Morphology* m = new Morphology(&imgFile, &structFile, &dilateOutFile, &erodeOutFile, &openingOutFile, &closingOutFile, &prettyPrintFile);

    imgFile.close();
    structFile.close();
    dilateOutFile.close();
    erodeOutFile.close();
    openingOutFile.close();
    closingOutFile.close();
    prettyPrintFile.close();

}