import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Scanner;
import java.lang.Math;

class HoughTransform{
    
    // Class Variables:

        // Variables for imgAry & houghAry
    int numRows, numCols, minVal, maxVal;
    int houghDist, offSet;
    int[][] imgAry, houghAry;

        // constants for houghAry
    int houghMinVal = 9999;
    int houghMaxVal = -1;
    double convertDegtoRad =  Math.PI / 180.00 ;
    

        // Variables for input/output
    FileInputStream inFile;
    FileOutputStream outFile1, outFile2;
    Scanner scanner;

    // Constructor:
    HoughTransform(FileInputStream inf, FileOutputStream of1, FileOutputStream of2) throws IOException{

        //Step 0: Reading in data 
            // Assigning class variables for io 
        inFile = inf;
        outFile1 = of1;
        outFile2 = of2;
        scanner = new Scanner(inFile);

            // Reading in image headers
        numRows = scanner.nextInt();
        numCols = scanner.nextInt();
        minVal = scanner.nextInt();
        maxVal = scanner.nextInt();

            // Initializing the imgAry
        imgAry = new int[numRows][numCols];
        loadImage();


        //Step 1: Creating the Hough Array and Hough variables: 
            
            // Max possible distance in polar coordinates:
            // The length of the diagonal in the cartesian plane where x=numRows to y=numCols
            // diagonal = sqrt(x^2 + y^2)
            double diagLength = Math.sqrt((double)numRows*numRows + numCols*numCols);
            
            // houghDist holds the max possible distance including the offset
            // houghDist = maxDist + offset = 2 * diagonal in cartesian plain
        houghDist =(int)  Math.ceil(2* (  diagLength ));
                
            // Offset = length of diagonal
        offSet = houghDist/2;
        houghAry = new int[houghDist][180];
        zero2DAry(houghAry);    // Setting all to zero

        //Step 2: Run Algorithm
        buildHoughSpace();
        determineMinMax();

        //Step 3: Outputting Results
        prettyPrint(outFile1);
        ary2File(outFile2);

    }


    //Methods:

    void loadImage(){

        // Read each int from the image file and storing in the imgAry
        for(int i =0; i<numRows; i++){
            for(int j=0; j<numCols; j++){
                imgAry[i][j] = scanner.nextInt();
            }
        }

    }

    void zero2DAry(int[][] ary){

        // Go to each element and set its content to zero
        for(int i =0; i<ary.length; i++){
            for(int j=0; j<ary[i].length; j++){
                ary[i][j] = 0;
            }
        }
    }

    void buildHoughSpace(){

        // for every non-zero point in the imgAry, compute the votes for possible lines
        for(int i =0; i<numRows; i++){  
            for(int j=0; j<numCols; j++){
                if(imgAry[i][j] > 0) computeSinusoid(i,j);
            }
        }

    }

    void computeSinusoid(int i, int j){

        // inputs the votes for 180 possibles lines given this point in imgAry(i,j)
        double angleInRadians;
        for(int angleInDegree =0; angleInDegree<180; angleInDegree++){ // for every degree 0 - 179
            angleInRadians = angleInDegree * convertDegtoRad; // convert to radians
            int polarDist = (int) polarDistance(i,j,angleInRadians); // calculate the polar distance for this possible line at this angle
            houghAry[polarDist][angleInDegree]++;  // input the vote in respective houghAry[][]
        }

    }

    double polarDistance(int x, int y, double theta){

        // orthogonal distance = x*cos(t) + y*sin(t) + offSet
        return  x * Math.cos(theta) + y * Math.sin(theta) + offSet;

    }

    void determineMinMax(){

        // read the entire HoughAry to determine HoughMinVal and HoughMaxVal
        for(int i =0; i<houghDist; i++){
            for(int j =0; j<180; j++){
                if (houghAry[i][j] > houghMaxVal) houghMaxVal = houghAry[i][j]; // check if we set a new max
                else if (houghAry[i][j]<houghMinVal) houghMinVal = houghAry[i][j];  // check if we set a new min
            }
        }

    }

    void prettyPrint(FileOutputStream outFile) throws IOException{

        for(int i=0; i<houghDist; i++){
            outFile.write("\n".getBytes());
            for(int j=0; j<180; j++){
                if(houghAry[i][j] > 0 ) outFile.write( "_ ".getBytes() );
                else outFile.write( "  ".getBytes() );
            }
        }

    }

    void ary2File(FileOutputStream outFile) throws IOException{

        // output HoughAry to outFile2
        outFile.write( ("\n" + houghDist + " 180 " + houghMinVal + " " + houghMaxVal ).getBytes() );

        for(int i =0; i<houghDist; i++){
            outFile.write( "\n".getBytes() );
            for(int j =0; j<180; j++){
                outFile.write((houghAry[i][j] + " ").getBytes() );
            }
        }


    }

    public static void main(String[] args) throws IOException{

        if(args.length != 3) {
            System.out.println("\nError! Expected 3 args, received " + args.length);
            return;
        }

        FileInputStream inFile = new FileInputStream(args[0]);
        FileOutputStream outFile1 = new FileOutputStream(args[1]);
        FileOutputStream outFile2 = new FileOutputStream(args[2]);

        new HoughTransform(inFile, outFile1, outFile2);

        inFile.close();
        outFile1.close();
        outFile2.close();
        

    }

}



