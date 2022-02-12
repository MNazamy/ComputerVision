import java.io.*;
import java.util.Scanner;  // Import the Scanner class


public class Image{

    // class variables
    int numRows, numCols, minVal, maxVal, threshVal;
    int[] histAry;
    Scanner scanner;

    //constructor
    public Image(String inFile1Name, int tv, String out1, String out2, String out3 , String out4 ) throws IOException{
        threshVal = tv;
        FileInputStream inFile1 = new FileInputStream(inFile1Name);
        scanner = new Scanner(inFile1);
        numRows = scanner.nextInt();    // read from image header
        numCols = scanner.nextInt();
        minVal = scanner.nextInt();
        maxVal = scanner.nextInt();

        histAry = new int[maxVal+1]; // dynamically create histAry[]
        for(int i=0; i<=maxVal; i++){
            histAry[i] = 0;
        }

        computeHist();


        FileOutputStream outFile1,outFile2,outFile3,outFile4;
        outFile1 = new FileOutputStream(out1);
        outFile2 = new FileOutputStream(out2);
        outFile3 = new FileOutputStream(out3);
        outFile4 = new FileOutputStream(out4);

        printHist(outFile1);
        dispHist(outFile2);

        inFile1.close();
        inFile1 = new FileInputStream(inFile1Name);
        threshhold(inFile1, outFile3, outFile4);    
        
        System.out.println("Project successfully ran! =) \n- Matin");

    }

    void computeHist(){
       // scans each int 1 by 1
       // increment the array by 1 
        while(scanner.hasNextInt()){
            int temp = scanner.nextInt();
            histAry[temp]++;
        }

        scanner.close();

    }

    void printHist(FileOutputStream outFile1) throws IOException{

        // print header
        outFile1.write( (numRows + " " + numCols + " " + minVal + " " + maxVal + "\n").getBytes() );

        // loop through array and print each slot 1 by 1
        for(int i =0; i< histAry.length; i++){
            outFile1.write((i + " " + histAry[i] + "\n").getBytes());
        }
        outFile1.close();
    }

    void dispHist(FileOutputStream outFile2) throws IOException{

        // print header
        outFile2.write( (numRows + " " + numCols + " " + minVal + " " + maxVal + "\n").getBytes() );
        int count;  // local var to make more efficient

        for(int i =0; i< histAry.length; i++){
            count = histAry[i];
            outFile2.write((i + " (" + count + "): ").getBytes());

            for(int j =0; j<count; j++){    // loop to print '+++'
                outFile2.write("+".getBytes());
            }
            outFile2.write("\n".getBytes());
        }

        outFile2.close();
    }

    void threshhold(FileInputStream inFile1, FileOutputStream outFile3, FileOutputStream outFile4) throws IOException{
        int min =0;
        int max =1;
        int count =0;
        int pixelVal;
        scanner = new Scanner(inFile1);

        scanner.nextInt();  // skip the 4 header numbers since we know them
        scanner.nextInt();
        scanner.nextInt();
        scanner.nextInt();

        outFile3.write( (numRows + " " + numCols + " " + min + " " + max + "\n").getBytes() );
        outFile4.write( (numRows + " " + numCols + " " + min + " " + max + "\n").getBytes() );

        while(scanner.hasNextInt()){
            count++;
            pixelVal = scanner.nextInt();
            if (pixelVal >= threshVal){
                outFile3.write( "1 ".getBytes());
                outFile4.write( "1 ".getBytes());
            }
            else{
                outFile3.write( "0 ".getBytes());
                outFile4.write( ". ".getBytes());
            }
            if (count%numCols == 0){    // start new line in output file
                outFile3.write( "\n".getBytes());
                outFile4.write( "\n".getBytes());  
            }
        }

        inFile1.close();
        outFile3.close();
        outFile4.close();
        scanner.close();

    }

    public static void main(String[] args) throws IOException{
        
        if(args.length != 6){
            System.out.println("Incorrect num of arguments! This program takes 6 arguments. \nYou passed " + args.length + " arguments");
            System.exit(0);
        }


        new Image(args[0],Integer.parseInt(args[1]), args[2], args[3], args[4], args[5]);



    }

}