#include <iostream>
#include <fstream>
#include <String>
using namespace std;


class ImageCompression{

    // Class Variables

    ifstream inFile;
    fstream skeletonFile;
    ofstream prettyPrintFile, decompressedFile;
    int numRows, numCols, minVal, maxVal;   // read from img header
    int newMinVal = 9999;
    int newMaxVal = -1;
    int** zfAry;
    int** skeletonAry; 
    
    public: 

    // constructor -- runs everything
    
    ImageCompression(string in, string out){



        // Step 1: open files from command line arguments
        inFile.open(in);
        if(!inFile){
            cout << "\nERROR! Problem opening input file:   " << in << endl;
            return;
        }
        prettyPrintFile.open(out);

        // getting the proper names for output files, based on arg[1] filename
        string skeletonFileName = in.substr(0,in.length()-4) + "_skeleton.txt"; 
        string decompressedFileName = in.substr(0,in.length()-4) + "_decompressed.txt";
        skeletonFile.open(skeletonFileName, ios::out);
        decompressedFile.open(decompressedFileName);

        // Step 2: read in image header
        inFile >> numRows;
        inFile >> numCols;
        inFile >> minVal;
        inFile >> maxVal;

        // Step 3: Set up the arrays dynamically            
        zfAry = new int*[numRows+2];    
        skeletonAry = new int*[numRows+2];
            
        for(int i=0; i<numRows+2; i++ ){
            zfAry[i] = new int[numCols+2];
            skeletonAry[i] = new int[numCols+2];
        }

        // Step 3.5: Set arrays to zero
        setZero(zfAry);
        setZero(skeletonAry);

        // Step 4: Load image ont zfAry
        loadImage();

       // Compress the image
       compute8Distance(zfAry, &prettyPrintFile);
       imageCompression();
       skeletonFile.close();

       // Decompress the image
       skeletonFile.open(skeletonFileName, ios::in);
       setZero(zfAry);
       loadSkeleton(&skeletonFile);
       imageDecompression(&prettyPrintFile);

       // Outputting final result
       decompressedFile << numRows << " " << numCols << " " << minVal << " " << maxVal;
       threshHold(zfAry, &decompressedFile);

        // closing resources
       inFile.close();
       prettyPrintFile.close();
       skeletonFile.close();
       decompressedFile.close();
      
        // optional: Test for NO DIFFERENCES. 
        if ( ! compareTextFiles(in, decompressedFileName) ) cout << "THERE IS AN ERROR, THEY DO NOT MATCH!";

    }

    // Methods for Compression!

    void compute8Distance(int** ary, ofstream* outFile){

        *outFile << "\n********COMPRESSION********\n\n";

        firstPass8Distance(ary);
        *outFile << "\n Zero-Framed Array after first pass distance transform:\n";
        reformatPrettyPrint(ary, outFile);
        
        
        secondPass8Distance(ary);
        *outFile << "\nZero-Framed Array after second pass distance transform:\n";
        reformatPrettyPrint(ary,outFile);


    }

    void firstPass8Distance(int** ary){

            // 8 block distance == min of all 4 neighbors, and add 1
        int minNeighbor;
        for(int i=0; i<numRows+2; i++){
            for(int j=0; j<numCols+2; j++){

                if(zfAry[i][j] > 0){    // we only calculate boundaries if this pixel is > 0

                    // min (a,b,c,d) =  min ( min(a,b) , min(b,c) )
                    minNeighbor = min( min(zfAry[i][j-1], zfAry[i-1][j-1]) , min(zfAry[i-1][j], zfAry[i-1][j+1]) );
                    zfAry[i][j] =  minNeighbor + 1;
                }

            } // inner for

        }// outer for
    }

    void secondPass8Distance(int** ary){
        // Note** In second pass, you need to keep track the newMinVal and newMaxVal.
        // now zfAry[i][j] needs to be accounted for in minimum of neighbors
        int minNeighbor;
        for(int i=numRows; i>=1; i--){
            for(int j=numCols; j>=1; j--){
                
                if(zfAry[i][j] > 0){
                            // center right , bottom right        middle bottom,         bottom left
                    minNeighbor = min( min(zfAry[i][j+1], zfAry[i+1][j+1]) , min(zfAry[i+1][j], zfAry[i+1][j-1]) );

                    // in 2nd pass, we might leave zfAry(i,j) if it is smallest of its neighbors
                    if (zfAry[i][j] > minNeighbor+1){ 
                        zfAry[i][j] = minNeighbor+1;
                    }

                    if (zfAry[i][j] >  newMaxVal)  newMaxVal = zfAry[i][j];
                    if(zfAry[i][j] < newMinVal) newMinVal = zfAry[i][j];

                }

            }// inner for

        }// outer for

    }

    void imageCompression(){

        computeLocalMaxima();
        prettyPrintFile << "\nSkeleton Array after compute Local Maxima:\n";
        reformatPrettyPrint(skeletonAry, &prettyPrintFile);
        extractSkeleton(&skeletonFile);

    }

    bool isLocalMaxima(int r, int c){

        // A pixel is local maxima if >= to all its 8 neighbors
        
        for(int i=r-1; i<=r+1; i++){ // scan top, center, bottom neighbors
            for(int j=c-1; j<=c+1; j++){  // scan left, middle, right neighbors

                if( zfAry[r][c] < zfAry[i][j]) return false; // if condition occours once its false

            }
        }

        return true;

    }

    void computeLocalMaxima(){

        for(int i =1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){

                if (isLocalMaxima(i,j) ) skeletonAry[i][j] = zfAry[i][j];
                else skeletonAry[i][j] = 0;

            }
        }

    }

    void extractSkeleton(fstream* outFile){

        *outFile << numRows << " " << numCols <<" 0 " << newMaxVal << endl;
        
        // if skeletonAry[i,j] > 0 -- write its data to skeletonFile.
        for(int i =1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){
                if (skeletonAry[i][j] > 0){
                    *outFile << i-1 << " " << j-1 << " " << skeletonAry[i][j] << endl;
                }
            }
        }
    }


    // Methods for Decompression!

    void loadSkeleton(fstream* skelFile){
        // Load the skeleton file onto inside frame of ZFAry.
        
        // skip the image header in the skeleton
        int skip4;
        for(int i=0; i<4; i++){
            *skelFile >> skip4;
        }

        int i, j, val;
        while( !(skelFile->eof())){
            *skelFile >> i;
            *skelFile >> j; // i and j are true positions of the skeleton
            *skelFile >> val;
            zfAry[i+1][j+1] = val;  // its [i+1][j+1] because we are adding it backed to FRAMED array
        }
    }

    void imageDecompression(ofstream* outFile){

        *outFile << "\n********DE-COMPRESSION********\n\n";

        firstPassExpansion();
        prettyPrintFile << "\nZero Framed Array after First Expansion:\n";
        reformatPrettyPrint(zfAry, outFile);


        secondPassExpansion();
        prettyPrintFile << "\nZero Framed Array after Second Expansion:\n";
        reformatPrettyPrint(zfAry, outFile);


    }

    void firstPassExpansion(){
        
        int maxNeighbor;
        
        for(int i =1; i<=numRows; i++){
            for(int j =1; j<=numCols; j++){

                if(zfAry[i][j] == 0){   // if zfAry is 0, check its neighbors

                    maxNeighbor = max(          // mid left      top left           top center            top right
                                        max(max(zfAry[i][j-1], zfAry[i-1][j-1]), max(zfAry[i-1][j],zfAry[i-1][j+1])),
                                                // mid right      bot right          bot center           bot left 
                                        max(max(zfAry[i][j+1], zfAry[i+1][j+1]), max(zfAry[i+1][j],zfAry[i+1][j-1]))
                                    );
                    
                    // zfAry is already 0, so only update if it if we need to
                    // maxNeighbor of 1 means zf is 0
                    if(maxNeighbor > 2){   
                        zfAry[i][j] = maxNeighbor-1;

                    }


                }   // outer if

            }   // inner for
        }   // outer for
    }

    void secondPassExpansion(){
        
        int maxNeighbor;
        
        for(int i=numRows; i>=1; i--){
            for(int j =numCols; j>=1; j--){

                    maxNeighbor = max(          // mid left      top left           top center            top right
                                        max(max(zfAry[i][j-1], zfAry[i-1][j-1]), max(zfAry[i-1][j],zfAry[i-1][j+1])),
                                                // mid right      bot right          bot center           bot left 
                                        max(max(zfAry[i][j+1], zfAry[i+1][j+1]), max(zfAry[i+1][j],zfAry[i+1][j-1]))
                                    );
                    
                    
                    if( zfAry[i][j] < maxNeighbor-1) zfAry[i][j] = maxNeighbor-1;

            }   // inner for
        }   // outer for

        
    }

    
    // Methods for set up!

    void setZero(int** ary){
        // set 2D Ary to zero.

        for(int i=0; i<numRows+2; i++){
            for(int j=0; j<numCols+2; j++){
                ary[i][j] = 0;
            }
        }

    }
    
    void loadImage(){

        // Load input onto inside frame of ZFAry.
        for(int i=1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){
                inFile >> zfAry[i][j];
            }
        }

    }

    // Methods for Output
    
    void reformatPrettyPrint(int** ary, ostream* outFile){


        for(int i =0; i<numRows+2; i++){
            for(int j=0; j<numCols+2; j++){

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
            *outFile << "\n";

        }
    }

    void threshHold(int** ary, ofstream* outFile){

        for(int i =1; i<=numRows; i++){
            *outFile << "\n";
            for(int j=1; j<=numCols; j++){
                if (zfAry[i][j] >= 1) *outFile << "1 ";
                else *outFile << "0 "; 
            }
            
        }       

    }

    bool compareTextFiles(string textFile1, string textFile2){
       ifstream if1, if2;
       if1.open(textFile1);
       if2.open(textFile2);
       bool flag = true;
       int a, b;
       while( !if1.eof() && !if2.eof() ){
           if1 >> a;
           if2 >> b;
           if(a!=b){
            cout << a << " " << b << endl; 
            flag = false;
           } 
       }

        if1.close();
        if2.close();
        return flag;
    
    }
};


int main(int argc, char* argsv[]){

    if (argc != 3){	// If not the correct amount of arguments
        cout << "Error: \n Expected 2 arguments" << endl << "Received " << argc-1 << " arguments" << endl;
        return -1;
    }

    ImageCompression* i = new ImageCompression(argsv[1], argsv[2]);
}