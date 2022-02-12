#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;


class BiMean{


    public:

    ifstream inFile;
    ofstream outFile1, outFile2;
    int numRows, numCols, minVal, maxVal, maxHeight, maxGVal, offSet, dividePt;
    int* histAry;
    int* gaussAry;
    int** histGraph;
    int** gaussGraph;
    int** gapGraph;


    BiMean(string in, string o1, string o2){

        // step 0: inFile, outFile1, outFile2  open via argv[]
        inFile.open(in,ios::in);	
        outFile1.open(o1, ios::out);
        outFile2.open(o2, ios::out);

        //  step 1: assign numerical vals
        inFile >> numRows;
        inFile >> numCols;
        inFile >> minVal;
        inFile >> maxVal;
        offSet = (maxVal - minVal)/10;
        dividePt = offSet;

        // step 3: dynamically allocate all arrays
        histAry = new int[maxVal+1];
        gaussAry = new int[maxVal+1];
        histGraph = new int*[maxVal+1];
        gaussGraph = new int*[maxVal+1];
        gapGraph = new int*[maxVal+1];

        set1DZero(histAry);
        set1DZero(gaussAry);
        maxHeight = loadHist(histAry, &inFile);

        for(int i =0; i<maxVal+1; i++){ 
            histGraph[i] = new int[maxHeight+1];    // set 2d arrays to new array
            gaussGraph[i] = new int[maxHeight+1];
            gapGraph[i] = new int[maxHeight+1];        
        }

        // SET 2D ARRAYS TO ZERO
        set2DZero(histGraph);
        set2DZero(gaussGraph);
        set2DZero(gapGraph);


        // step 5 thru 7    -- obtain threshold val
        plotHistGraph();
        outFile1 << "\n\nHistogram Output below: \n";
        prettyPrint(histGraph, &outFile1);
        int bestThrVal = biMeanGauss(dividePt, &outFile2);
        outFile1 << "\n\nBest Thresh Val: " << bestThrVal;

        // step 8-9 -- obtain the results
        bestFitPlot(bestThrVal);
        outFile1 << "\n\nResult of Gaussian curve below: \n";
        prettyPrint(gaussGraph, &outFile1);
        outFile1 << "\n\nResult of Gap Graph below:\n";
        prettyPrint(gapGraph, &outFile1);

        // step 10: close all files
        inFile.close();
        outFile1.close();
        outFile2.close();
    }

    int loadHist(int* histAry, istream* inFile){
        int i;
        int max=-1;
        // int maxPos; // might be needed to track position of max histAry[i] 
        *inFile >> i;   // read number 

        while(inFile->eof() == 0){  // while file is valid
            *inFile >> histAry[i];  // read next value for histAry[i]
            if(histAry[i] > max) max = histAry[i];  // check for max
            *inFile >> i;   // read next i
        }
        return max;

    }

    void plotHistGraph(){
        for(int i =0; i<=maxVal; i++){
            for(int j=0; j<histAry[i]; j++){
                histGraph[i][j] = 1;
            }
        }
    }
       
    void set1DZero(int* inAry){
        for(int i =0; i<=maxVal; i++){
            inAry[i] = 0;
        }
    }

    void set2DZero(int** inAry){
        for(int r =0; r<=maxVal; r++){
            for(int c=0; c<=maxHeight; c++){                
                inAry[r][c] = 0;
            }
        }

    }

    int biMeanGauss(int dp, ostream* outFile){

        double sum1, sum2, total, minSumDiff;
        int bestThr = dp;
        int end = maxVal-offSet;
        minSumDiff = 999999.9;
        
        while (dp < end ){

            set1DZero(gaussAry);    // get arrays prepared
            set2DZero(gaussGraph);
            set2DZero(gapGraph);

            
            sum1 = fitGauss(0,dp);    // calculate ints

            sum2 = fitGauss(dp, maxVal);
            total = sum1 + sum2;
            *outFile << "\n\nSum1: " << sum1 << "    Sum2: " << sum2 << "     Total: " << total; 
            
            if (total < minSumDiff){    // if new best
                minSumDiff = total;
                bestThr = dp;

            }
            
            *outFile << "\nDividePt: " << dp << "     MinSumDiff: " << minSumDiff << "      bestThr: " << bestThr;
            dp++;
            prettyPrint (gaussGraph, outFile);
            plotGaps();
            prettyPrint (gapGraph, outFile);

        }
        
        return bestThr;

       }

    double fitGauss(int leftIndex, int rightIndex ){

       double mean, var, sum, gVal, maxGVal;
       sum = 0.0;
       mean = computeMean(leftIndex, rightIndex);
       var = computeVar(leftIndex, rightIndex, mean);
       outFile2 << "\nLeftIndex: " << leftIndex << "      RightIndex: " << rightIndex << "    Mean: " << mean;

       int index = leftIndex;
       
       while(index <= rightIndex){
            gVal = modifiedGauss(index, mean, var);
            sum += abs(gVal- (double)histAry[index]);
            int gv = (int) gVal;
            gaussAry[index] = gv;

            gaussGraph[index][gv] = 1;
            index++;
       }

       return sum;
    }
 
    double computeMean(int leftIndex, int rightIndex){

        int maxh = 0;
        int sum=0;
        int numPixels = 0;
        int index = leftIndex;
        while(index < rightIndex){
            sum += (histAry[index] * index);
            numPixels += histAry[index];

            if (histAry[index] > maxh){
                maxh = histAry[index];
            }
            index++;

        }

        return (double) sum / (double) numPixels;
    }

    double computeVar(int leftIndex, int rightIndex, double mean){

       double var;
       double sum = 0.0;
       int numPixels = 0;
       int index = leftIndex;
       while(index < rightIndex){
           var = ((double)index - mean) * ((double)index - mean);
           sum += (double)histAry[index] * var;
           numPixels += histAry[index];
           index++;
       }

       return (double) (sum / ((double) numPixels));
    }

    double modifiedGauss(int index, double mean, double var){

        double num = ((double) index) - mean;
        num = num*num;
        return  (double) ( (double)maxHeight * exp( - ( num / (2.0*var) ) ) );
    
    }

    void bestFitPlot(int bestThrVal){

        double sum1, sum2;
        set1DZero(gaussAry);
        set2DZero(gaussGraph);
        set2DZero(gapGraph);

        sum1 = fitGauss(0, bestThrVal);
        sum2 = fitGauss(bestThrVal, maxVal);
        plotGaps();

    }
        
    void plotGaps(){

        int first, last;
        int index = minVal;
        while(index < maxVal){
            first = min(histAry[index], gaussAry[index]);
            last = max(histAry[index], gaussAry[index]);
            while(first < last){
                gapGraph[index][first] = 1;
                first++;
            }
            index++;
        }
    }

    void prettyPrint(int** ary, ostream* outFile){

        *outFile << "\n";
        for(int i =0; i<=maxVal; i++){
            *outFile << "\n";
            for(int j=0; j<=maxHeight; j++){
                if( ary[i][j] <= 0) *outFile << " ";
                else *outFile << ". ";
            }
        }
    }

};  // end BiMean class

int main(int argc, char* argsv[]){

    if (argc != 4){	// If not the correct amount of arguments
        cout << "Error: \n Expected 3 arguments" << endl << "Received " << argc-1 << " arguments" << endl;
        return -1;
    }

    BiMean* b = new BiMean(argsv[1], argsv[2], argsv[3]);


}
