#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cmath>
using namespace std;

class box{

    public:
        int minRow, minCol, maxRow, maxCol;

        box(){
            minRow = 0;
            minCol = 0;
            maxRow = 0;
            maxCol = 0;
        }

        box(int a, int b, int c, int d){
            minRow = a;
            minCol = b;
            maxRow = c;
            maxCol = d;
        }
};

class boxNode{

    public:
    int boxType;  // 1 for page, 2 for column, 3 for zone, 4 for texLine, 5 textWord, etc
                    // in this project we use 3 and 4
    box* boundBox;   // bounding box
    boxNode* next;  // points to boxNode in the same level

    boxNode(){
        boxType = 99;
        next = nullptr;
    }
    
    boxNode(int t, box* b){
        boxType = t;
        boundBox = b;
        next = nullptr;
    }

};

class boxQueue{

    public:

    boxNode* front;
    boxNode* back;

    boxQueue(){
        front = new boxNode();
        back = new boxNode();
        back->next = front;

    }

    void insert(boxNode* q){
        q->next = back->next; 
        back->next = q;   
    }

    boxNode* pop(){
        boxNode* temp = back;
        boxNode* hold;
            // check if empty
        if(isEmpty()) return nullptr;

            // go to the 2nd to the front node
        while(temp->next->next != front) temp=temp->next;

            // set 2nd front to point to front
            // remove any references to the old front 
        if(temp->next->next == front){
            hold = temp->next;
            temp->next = front;
            return temp;
        }


        return nullptr;
        
    }

    bool isEmpty(){
        return back->next == front;
    }

};


/*                  Project Overview
                                
    As taught in class, one major task in the document image analysis is to decompose a given document into a
    hierarchical tree structures where the root is the whole document (could be one page or multiple pages); the next level
    under a page are one or more column blocks, each column block could consists of text zones and other none-text
    zones (such as figures, graphic, table, math equations, ...); the next level under text-zones are paragraphs; below
    paragraph are text-lines; below text-lines are text-words; below text-words are characters. An Optical Character
    Recognition (OCR) system begins it document recognition from bottom up of the document hierarchical tree; OCR
    first performs character recognition, then, up to form words, up to form text-lines, and so for. A highly effective
    technique for document image decomposition is using the projection profiles of a given document to construct the
    document hierarchy top-down.

    The Projection Profiles (also called a Signature Analysis of a Binary Image) is a projection (summing up) object
    (none-zero) pixels within a given “zone” within the image, taking on a given direction i.e. vertical or horizontal,
    therefore are called Horizontal Projection Profile (HPP) and Vertical Projection Profile (VPP).
    As taught in class, the HPP and VPP can also be used to determine the reading direction of a given document, by
    analyzing the patterns of HPP and VPP.

    In this project, the input image contains a "zone" from a document. If the zone is a none-text zone, your program will
    say so. If the zone is a text-zone, via HPP and VPP, you are 1) to determine and overlay the zone bounding boxes of
    the input image; 2) to determine and overlay the bounding boxes of text-lines within the zone and 3) to determine the
    reading direction of the document.

*/

class docImage{

    public:

        // Class Variables

        int numRows, numCols, minVal, maxVal;
        int** imgAry;
        boxQueue* queue;
        boxNode* listHead;
        box* zoneBox;


        int* HPP;   // a 1D array to store the horizontal/vertical projection profile
        int* VPP;   
        int* HPPbin;    // a 1D array of binarized HPP/VPP. dynamically allocate at runtime, size as HPP/VPP;
        int* VPPbin;    
        int* HPPmorph;
        int* VPPmorph;
        int HPPruns, VPPruns, thrVal;

        int readingDir;

        ifstream inFile;
        ofstream outFile1, outFile2;

        // Constructor
        docImage(string f, int tv){

                // Read input data, load image 
            inFile.open(f);
            thrVal = tv;
            loadImage();

            outFile1.open("outFile1.txt");
            outFile2.open("outFile2.txt");

                // Dynamically allocate a 1D array for the project profiles
            HPP = new int[numRows+2]; HPPbin = new int[numRows+2]; HPPmorph = new int[numRows+2]; 
            VPP = new int[numCols+2]; VPPbin = new int[numCols+2]; VPPmorph = new int[numCols+2];

            for(int i =0; i<numRows+2; i++){    // initialize to zero
                HPP[i] = 0; HPPbin[i] = 0; HPPmorph[i] = 0;
                VPP[i] = 0; VPPbin[i]=0; VPPmorph[i] = 0;
            }

                //  Compute the HPP and the VPP from the input image.
            computeHPP(); computeVPP();
            outFile2 << "\nHPP  : \n";  printPP(HPP, numRows, &outFile2);
            outFile2 << "\nVPP  : \n"; printPP(VPP, numCols, &outFile2);

                // Threshold and output binary PP
            threshold(tv);
            outFile2 << "\n\nHPP Binary Threshold : \n";  printPP(HPPbin, numRows, &outFile2);
            outFile2 << "\nVPP Binary Threshold : \n"; printPP(VPPbin, numCols, &outFile2);

                //  Compute the zone bounding box based on HPPBinary and VPPBinary.      
            computeZoneBox();
            outFile2 << "\n\nZone Bounding Box : \n" << zoneBox->minRow << " " << zoneBox->minCol << " " << zoneBox->maxRow << " " << zoneBox->maxCol;

                //  Apply 1D morphological closing
            morphClosing();
            outFile2 << "\n\nHPP Morph : \n";  printPP(HPPmorph, numRows, &outFile2);
            outFile2 << "\nVPP Morph : \n"; printPP(VPPmorph, numCols, &outFile2);



                // Start building the box queue
            queue = new boxQueue();
            queue->insert(new boxNode(3,zoneBox));
            printBoxQueue(&outFile2);

                // Compute the number of runs
            HPPruns = computePPRuns(HPPbin, numRows);
            VPPruns = computePPRuns(VPPbin, numCols);
            outFile2 << "\n\nHPP Runs: " << HPPruns << "        VPP Runs: " << VPPruns; 

                //  Using HPPMorph and VPPMorph to determine the reading direction of the text-zone.
            readingDir = determineReadingDirection();
            outFile1 << "\n\nReading Direction : ";

                // Determine the reading direction
            if(readingDir == 1) {outFile1 << "Horizontal\n"; computeTBoxHorizontal();}
            else if(readingDir == 2){ outFile1 << "Vertical\n"; computeTBoxVertical();}
            else {outFile1 << "The zone may be a non-text zone!\n"; }
            
                // Overlay the zone box and text-line bounding boxes onto the image array.
            printBoxQueue(&outFile2);
            overlayImgAry();
            reformatPrettyPrint(imgAry, numRows, numCols, &outFile1);

                // close resources
            inFile.close();
            outFile1.close();
            outFile2.close();
        }

        

        void overlayImgAry(){

            boxNode* thisBox = queue->pop();    
            int label = 1;
            int minR, minC, maxR, maxC;

            while(thisBox != 0 &&  thisBox != queue->back){
                minR = thisBox->boundBox->minRow;
                maxR = thisBox->boundBox->maxRow;
                minC = thisBox->boundBox->minCol;
                maxC = thisBox->boundBox->maxCol;
                for(int i = minR; i<=maxR; i++){
                    for(int j=minC; j<=maxC; j++){
                        imgAry[i][j] = label;
                    }
                }

                thisBox = queue->pop();
            }

        }

        void computeTBoxHorizontal(){

            int minR = zoneBox->minRow; int maxR = minR;
            int minC = zoneBox->minCol; int maxC = zoneBox->maxCol;

            while(maxR <= numRows){
                    // find the start row of this text box
                while (HPPmorph[maxR] == 0 && maxR<= numRows) maxR++;

                    // find the end row of this text box
                minR = maxR;
                while(HPPmorph[maxR] > 0 && maxR <= numRows) maxR++;

                    // insert this node in the queue
                queue->insert( new boxNode( 4 , new box(minR, minC, maxR, maxC) ) );

                    // skip zeroes in between to next text box
                minR = maxR;
                while(minR == 0 && minR <= numRows) minR++;   
            }
        }

        void computeTBoxVertical(){

            int minR = zoneBox->minRow; int maxR = zoneBox->maxRow;
            int minC = zoneBox->minCol; int maxC = minC;
            while(maxC <= numCols){
                    // find the start col of this text box
                while (VPPmorph[maxC] == 0 && maxC<= numCols) maxC++;

                    // find the end col of this text box
                minC = maxC;
                while(VPPmorph[maxC] > 0 && maxC <= numCols) maxC++;

                    // insert this node in the queue
                queue->insert( new boxNode( 4 , new box(minR, minC, maxR, maxC) ) );


                    // skip zeroes in between to next text box
                minC = maxC;
                while(minC == 0 && minC <= numCols) minC++;   
            }
            
        }

        int determineReadingDirection(){

            int factor = 2;

            if(HPPruns <= 2 && VPPruns <= 2) return 0;
            
            else if(HPPruns >= factor*VPPruns) return 1;
            
            else if(VPPruns >= factor*HPPruns) return 2;
            
            else return 0;
        }

        void computeZoneBox(){
            // see algorithm below.
            //Computes the zone bounding box based on HPPBinary and VPPBinary.
            // A bounding box is represented by 2 points, four integers (minRow, minCol) and (maxRow, maxCol).
            int minR = 1;
            int minC = 1;
            int maxR = numRows;
            int maxC = numCols;

                // step 1
            while(HPPbin[minR] == 0 && minR <= numRows )  minR++;

                // step 3
            while(HPPbin[maxR] == 0 && maxR >= 1) maxR--;
               
                // step 6
            while( VPPbin[minC] == 0 && minC <= numCols) minC++;

                // step 8
            while(VPPbin[maxC] == 0 && maxC >=1) maxC--;

            zoneBox = new box(minR, minC, maxR, maxC);

        }

        int computePPRuns(int* pp, int l){
            // computes the number of run in morphPP, labelling each run, in sequence: 1, 2, 3, ...
            // overwriting morphPP and returns the number of runs.
            int numRuns = 0;
            int i =0;
            while(i<=l){

                while(pp[i] == 0 && i<=l) i++;  // skip zeroes

                if(pp[i] > 0){  // once we found a run, skip through this run
                    numRuns++;
                    while( pp[i] > 0 && i<=l) i++;
                }
            }
            return numRuns;
        }

        void morphClosing(){

                    // hpp morph
                for(int i =1; i<=numRows; i++) if( HPPbin[i-1] == 1 && HPPbin[i] == 1 && HPPbin[i+1] == 1) HPPmorph[i] = 1;

                    // vpp morph
                for(int i =1; i<=numCols; i++) if( VPPbin[i-1] == 1 && VPPbin[i] == 1 && VPPbin[i+1] == 1) VPPmorph[i] = 1;
                
        }

        void computeHPP(){
            // compute the horizontal projection profile of object pixels within imgBox.
            for(int row = 1; row <= numRows; row++){
                int numThisRow = 0;
                for(int col =1; col <= numCols; col++) if (imgAry[row][col] > 0) numThisRow++;
                HPP[row] = numThisRow;
            }
        }

        void computeVPP(){
            // compute the vertical projection profile of object pixels within imgBox.
            for(int col =1; col <= numCols; col++){
                int numThisRow = 0;
                for(int row = 1; row <= numRows; row++) if (imgAry[row][col] > 0) numThisRow++;
                VPP[col] = numThisRow;
            }
        }

        void threshold(int val){

                // thresholding HPP
            for(int i =0; i<numRows+2; i++){
                if (HPP[i] >= val) HPPbin[i] = 1;
                else HPPbin[i] = 0;
            }

                // thresholding VPP
            for(int j=0; j<numCols+2; j++){
                if (VPP[j] >= val) VPPbin[j] = 1;
                else VPPbin[j] = 0;
            }
        }

 
            // I/O Methods

        void printBoxQueue(ofstream* outFile){

            *outFile << "\n\nPrinting Box Queue:\n\n";

            boxNode* temp = queue->back->next;

            while(temp != queue->front ){
                *outFile << temp->boxType << endl;
                if(temp->boxType == 4){
                    *outFile << temp->boundBox->minRow << " " << temp->boundBox->minCol << " " << temp->boundBox->maxRow << " " << temp->boundBox->maxCol << endl;
                }
                temp = temp->next;
            }

        }

        void printPP(int* ary, int l, ofstream* outFile ){
            // reuse code from your previous project

            for(int i =1; i<=l; i++) *outFile << ary[i] << " "; 
            
            *outFile << "\n";
        }

        void reformatPrettyPrint(int** ary, int r, int c, ofstream* outFile ){

            // reuse code from your previous project
            for(int i =1; i<=r; i++){
                for(int j=1; j<=c; j++){

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
                    else *outFile << ".   ";

                }
                *outFile << "\n";
            }
        }
        
        void loadImage(){
            
                // Read in img header 
            inFile >> numRows >> numCols >> minVal >> maxVal;

                // build our 2D array
            imgAry = new int*[numRows+2];
            for(int i =0; i<numRows+2; i++){
                imgAry[i] = new int[numCols+2];
            }

                // Initialize to zero
            /* try this one:
            imgAry = {0};
            */
            for(int i=0; i<numRows+2; i++){
                for(int j =0; j<numCols+2; j++){
                    imgAry[i][j] = 0;
                }
            }

                // Fill in our img Ary
            for(int i =1; i<=numRows; i++){
                for(int j =1; j<= numCols; j++){
                    inFile >> imgAry[i][j];
                }
            }

        }

};


int main(int argc, char* argsv[]){

    if (argc != 3){	// If not the correct amount of arguments
        cout << "Error: \n Expected 2 arguments" << endl << "Received " << argc-1 << " arguments" << endl;
        return -1;
    }

    int threshVal = stoi(argsv[2]);

    docImage* d = new docImage(argsv[1], threshVal);

}