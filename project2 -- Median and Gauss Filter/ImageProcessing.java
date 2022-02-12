import java.io.*;
import java.util.Scanner;

class ImageProcessing{

    int numRows, numCols, minVal, maxVal, maskRows, maskCols, maskMin, maskMax, maskTotal, thrVal, newMin, newMax;
    int[][] mirrorFramedAry, medianAry, gaussAry, thrAry, maskAry;
    int[] neighborAry;
    Scanner inputScanner, maskScanner;

    public ImageProcessing(String arg0, String arg1, int tv, String arg3, String arg4, String arg5, String arg6, String arg7, String arg8, String arg9) throws IOException
    {
        // threshhold variable
        thrVal = tv;

        FileInputStream data = new FileInputStream(arg0);
        FileInputStream mask = new FileInputStream(arg1);

        // image variables
        inputScanner = new Scanner(data);
        numRows = inputScanner.nextInt();
        numCols = inputScanner.nextInt();
        minVal = inputScanner.nextInt();
        maxVal = inputScanner.nextInt();

        // mask variables
        maskScanner = new Scanner(mask);
        maskRows = maskScanner.nextInt();
        maskCols = maskScanner.nextInt();
        maskMin = maskScanner.nextInt();
        maskMax = maskScanner.nextInt();

        // open all outfiles
        FileOutputStream inputImg = new FileOutputStream(arg3);
        FileOutputStream medianOutImg = new FileOutputStream(arg4);
        FileOutputStream medianThrImg = new FileOutputStream(arg5);
        FileOutputStream medianPrettyPrint = new FileOutputStream(arg6);
        FileOutputStream gaussOutImg = new FileOutputStream(arg7);
        FileOutputStream gaussThrImg = new FileOutputStream(arg8);
        FileOutputStream gaussPrettyPrint = new FileOutputStream(arg9);

        // dynamically allocate all 1-D and 2-D arrays
        mirrorFramedAry = new int[numRows+2][numCols+2];
        medianAry = new int[numRows+2][numCols+2];
        gaussAry = new int[numRows+2][numCols+2];
        thrAry = new int[numRows+2][numCols+2];
        maskAry = new int[maskRows][maskCols];
        neighborAry = new int[9];

        // step 3 thru 6 -- prepare image 
        loadMask();
        loadImage();
        mirrorFraming();
        imgReformat(mirrorFramedAry, minVal, maxVal, inputImg);
        
        // step 7 thru 11 -- perform median image processing
        computeMedian();
        imgReformat(medianAry, newMin, newMax, medianOutImg);
        threshold(medianAry, thrAry);
        imgReformat(thrAry, newMin, newMax, medianThrImg);
        prettyPrint(thrAry, medianPrettyPrint);

        //step 12 thru 16 -- perform gaussian array image processing
        computeGauss();
        imgReformat(gaussAry, newMin, newMax, gaussOutImg);
        threshold(gaussAry, thrAry);
        imgReformat(thrAry, newMin, newMax, gaussThrImg);
        prettyPrint(thrAry, gaussPrettyPrint);

        // step 17 -- close all open files
        data.close();
        mask.close();
        inputImg.close();
        medianOutImg.close();
        medianPrettyPrint.close();
        medianThrImg.close();
        gaussOutImg.close();
        gaussPrettyPrint.close();
        gaussThrImg.close();

    }
    
    void loadImage(){
        //  Read from input file and load onto mirrorFramedAry begin at [1][1].
        for(int i =1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++ ){
                mirrorFramedAry[i][j] = inputScanner.nextInt();
            }
        }
        inputScanner.close();
    }

    void loadMask(){
        maskTotal =0;   // better to store this once, rather than recalculate it in gaussian every time
        for(int i=0; i<maskRows; i++){
            for(int j=0; j<maskCols; j++){
                maskAry[i][j] = maskScanner.nextInt();
                maskTotal += maskAry[i][j]; 
            }
        }
        maskScanner.close();
    }
    
    void mirrorFraming(){
        
        // 4 corners
        mirrorFramedAry[0][0] = mirrorFramedAry[1][1];  // top left
        mirrorFramedAry[0][numCols+1] = mirrorFramedAry[1][numCols];  // top right
        mirrorFramedAry[numRows+1][0] = mirrorFramedAry[numRows][1];    // bottom left
        mirrorFramedAry[numRows+1][numCols+1] = mirrorFramedAry[numRows][numCols];  // bottom right

        // fills in the top and bottom
        for(int col =1; col<numRows; col++){   // traverse each coloumn left to right
            mirrorFramedAry[0][col] = mirrorFramedAry[1][col];  // fill top row
            mirrorFramedAry[numRows+1][col] = mirrorFramedAry[numRows][col];    //fill  bottom row
        }

        // fills in the sides
        for(int row=1; row<numCols; row++){  // traverse each row top down
            mirrorFramedAry[row][0] = mirrorFramedAry[row][1];   // left side
            mirrorFramedAry[row][numCols+1] = mirrorFramedAry[row][numCols]; //right side
        }

    }

    void loadNeighbors(int a, int b){
        // - loadNeighbors(...) // On your own. Load the 3 x 3 neighbors of mirrorFramedAry (i,j) into neighborAry,
        // // using 2 loops; do NOT write 9 assignments.
        int spot =0;
        for(int i =a-1; i<=a+1; i++){
            for (int j=b-1; j<=b+1; j++){
                neighborAry[spot] = mirrorFramedAry[i][j];
                spot++;
            }
        }
    }

    void computeMedian(){

        newMin = 9999;
        newMax = 0;
        for(int i=1; i<=numRows; i++){
            for(int j=1; j<= numCols; j++){
                loadNeighbors(i,j);
                sort(neighborAry);
                medianAry[i][j] = neighborAry[4];
                if (newMin > medianAry[i][j]) newMin = medianAry[i][j];
                if (newMax < medianAry[i][j]) newMax = medianAry[i][j];
            }
        }
    }

    void sort(int[] ary){

        int temp;
        boolean sorted = false;
        while(sorted!=true){
            sorted=true;    // assume it is sorted
            for(int i =1; i<9; i++){

                if(ary[i-1] > ary[i]){  // swap if any non sorted found
                    sorted=false;
                    temp = ary[i-1];
                    ary[i-1] = ary[i];
                    ary[i] = temp;
                }
            }

        }   // break from this loop after every element is larger than previous one in list
    }

    void computeGauss(){

        newMin = 9999;
        newMax =0;

        for(int i=1; i<= numRows; i++){
            for(int j=1; j<= numCols; j++){
                gaussAry[i][j] = convolution(i,j);
                if (newMin > gaussAry[i][j]){
                    newMin = gaussAry[i][j];
                }
                if (newMax < gaussAry[i][j]){
                    newMax = gaussAry[i][j];
                }
            }
        }
    }

    int convolution(int a, int b){
        
        int total =0;
        int spot=0;
        loadNeighbors(a,b);
        for(int i=0; i<maskRows; i++){
            for(int j=0; j<maskRows; j++){
                total+= maskAry[i][j] * neighborAry[spot];
                spot++;
            }
        }
        return total/maskTotal;
    }   
    
    void imgReformat(int[][] inAry, int newMin, int newMax, FileOutputStream outImg) throws IOException{

        outImg.write((numRows + " " + numCols + " " + newMin + " " +newMax + "\n").getBytes() );
        int width = (Integer.toString(newMax)).length();
        int ww;
        for(int r =1; r<= numRows; r++){
            for(int c=1; c<= numCols; c++){
                outImg.write( (inAry[r][c] + "").getBytes());
                ww = Integer.toString(inAry[r][c]).length();
                while(ww<width){
                    outImg.write( " ".getBytes() );
                    ww++;
                }
            }
            outImg.write("\n".getBytes());
        }
        
    }

    void threshold(int[][] inAry, int[][] threshAry){

        for(int i=1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){
                if (inAry[i][j] >= thrVal) threshAry[i][j] = 1;
                else threshAry[i][j] =0;
            }
        }
    }

    void prettyPrint(int[][] inAry, FileOutputStream outFile) throws IOException{

        for(int i=1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++){
                if(inAry[i][j] >0) outFile.write((inAry[i][j] + " ").getBytes());
                else outFile.write((". ").getBytes()); 
            }
            outFile.write("\n".getBytes());
        }
    }
    
    public static void main(String[] args) throws IOException{
            
        if(args.length != 10){
            System.out.println("Incorrect num of arguments! This program takes 10 arguments. \nYou passed " + args.length + " arguments");
            System.exit(0);
        }

        new ImageProcessing(args[0],args[1], Integer.parseInt(args[2]), args[3], args[4], args[5], args[6], args[7], args[8], args[9] );

    }


}