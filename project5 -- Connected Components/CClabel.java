import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Scanner;

class Property{
    int label, numPixels, minR, minC, maxR, maxC;

    public Property(int l){
        label = l;
        numPixels = 0;
        minR = 9999;
        minC = 9999;
        maxR = -1;
        maxC = -1;
    }
}


class CClabel{

// MEMBER VARIABLES
    int numRows, numCols, minVal, maxVal;
    int newMin, newMax, trueNumCC, connectedness;
    int newLabel;
    int [][] zeroFramedAry;
    int [] nonZeroNeighborAry;
    int [] eqAry;
    Property [] propertyAry;
    Property ccProperty;
    FileOutputStream prettyPrintFile, labelFile, propertyFile;
    FileInputStream inFile;
    Scanner scanner;

// CONSTRUCTOR
    public CClabel(int arg0, String data, String ppf, String lf, String pf) throws IOException{
        //constructor(...) // need to dynamically allocate all arrays; and assign values to numRows,, etc.
    
        inFile = new FileInputStream(data);
        prettyPrintFile = new FileOutputStream(ppf);
        labelFile = new FileOutputStream(lf);
        propertyFile = new FileOutputStream(pf);

        scanner = new Scanner(inFile);
        connectedness = arg0;
        numRows = scanner.nextInt();
        numCols = scanner.nextInt();
        minVal = scanner.nextInt();
        maxVal = scanner.nextInt();

        zeroFramedAry = new int[numRows+2][numCols+2];
        eqAry = new int [ (numRows/2) * (numCols/2) ];
        nonZeroNeighborAry = new int[4];
        newLabel = 0;
        
        // step 1
        zero2D(zeroFramedAry);

        //step 2
        loadImage();
        inFile.close();

        prettyPrintFile.write("\n\n***************************************\nFirst Pass:".getBytes());
        pass1();
        imgReformat();
        printEqAry();

        prettyPrintFile.write("\n\n***************************************\nSecond Pass:".getBytes());
        pass2();
        imgReformat();
        printEqAry();

        // step 6
        prettyPrintFile.write("\n\n***************************************\nManaged EQ Array:".getBytes());
        trueNumCC = manageEqAry();
        printEqAry();

        // step 7 -- 9
        prettyPrintFile.write("\n\n***************************************\nThird Pass:".getBytes());
        pass3();
        imgReformat();
        printEqAry();

        // step 10 -- complete label file
        printImg();

        // step 12 -- complete property file
        printCCproperty();
        drawBoxes();
        imgReformat();

        prettyPrintFile.write(("\n\nTrue Number of Connected Components: " + trueNumCC).getBytes());

        // close all files
        prettyPrintFile.close();
        labelFile.close();
        propertyFile.close();

        
    }


// METHODS

    void pass1(){
        int whichCase;
        for(int i =1; i<=numRows; i++){ // left to right
            for(int j =1; j<=numCols; j++){    // top to bottom
                if(zeroFramedAry[i][j] > 0){
                    minus1D(nonZeroNeighborAry);
                    whichCase = fillNeighborAry(i, j, 1);
                    // case 1
                    if(whichCase == 1){
                        newLabel++;
                        eqAry[newLabel] = newLabel;
                        zeroFramedAry[i][j] = newLabel;
                    }
                    else if(whichCase == 2){ // case 2
                        zeroFramedAry[i][j] = findMin();
                    }
                    else{   // case 3
                        zeroFramedAry[i][j] = findMin();
                        updateEq(zeroFramedAry[i][j]);
                    }
                }
            }
        }
    }
    
    void pass2(){
        int whichCase, min;
        for(int i =numRows; i>=1; i--){ // right to left
            for(int j =numCols; j>=1; j--){ // bottom to top
                if(zeroFramedAry[i][j] > 0){
                    minus1D(nonZeroNeighborAry);
                    whichCase = fillNeighborAry(i, j, 2);
                    if(whichCase == 3){
                        min = findMin();
                        if (zeroFramedAry[i][j] < min){
                            eqAry[min] = zeroFramedAry[i][j];
                        }
                        else{
                            eqAry[zeroFramedAry[i][j]] = min;
                            zeroFramedAry[i][j] = min;    
                        }
                    }
                }
            }
        }
    }

    void pass3(){
        propertyAry = new Property[newLabel+1];
        for(int i =1; i<propertyAry.length; i++){
            propertyAry[i] = new Property(i);
        }

        int finalLabel;
        for(int i =numRows; i>=1; i--){
            for(int j=numCols; j>=1; j--){
                if(zeroFramedAry[i][j] > 0){
                    finalLabel = eqAry[zeroFramedAry[i][j]];
                    zeroFramedAry[i][j] = finalLabel;
                    propertyAry[finalLabel].numPixels++;
                    propertyAry[finalLabel].minR = Math.min(propertyAry[finalLabel].minR, i);
                    propertyAry[finalLabel].minC = Math.min(propertyAry[finalLabel].minC, j);
                    propertyAry[finalLabel].maxR = Math.max(propertyAry[finalLabel].maxR, i);
                    propertyAry[finalLabel].maxC = Math.max(propertyAry[finalLabel].maxC, j);
                }
            }
        }

    }

    int findMin(){
        // function to find the minimum of a neighborAry[]
        int min = 9999;
        for(int i=0; i<4; i++){
            if (min > nonZeroNeighborAry[i] && nonZeroNeighborAry[i] != -1) min = nonZeroNeighborAry[i];
        }
        return min;
    }
    
    int fillNeighborAry(int i , int j, int whichPass){
        // function to fills neighbor array
        // returns whether case 1, 2 or 3
        int count = 0;
        if(whichPass == 1){     // for first pass
            if (connectedness == 4){
                if(zeroFramedAry[i-1][j] > 0){    // top center
                    nonZeroNeighborAry[count] = zeroFramedAry[i-1][j];
                    count++;
                } 
                if(zeroFramedAry[i][j-1] > 0){  // middle left
                    nonZeroNeighborAry[count] = zeroFramedAry[i][j-1];
                    count++;
                }
                if (count == 0) return 1;   // case 1   -- all zeroes
                if (count == 1) return 2;   // case 2 -- only one of the two
                return 3;   // case 3 -- return the min
            }
            else{

                if(zeroFramedAry[i-1][j] > 0){    // top center
                    nonZeroNeighborAry[count] = zeroFramedAry[i-1][j];
                    count++;
                } 
                if(zeroFramedAry[i][j-1] > 0){  // middle left
                    nonZeroNeighborAry[count] = zeroFramedAry[i][j-1];
                    count++;
                }
                if(zeroFramedAry[i-1][j+1] > 0){    // top right
                    nonZeroNeighborAry[count] = zeroFramedAry[i-1][j+1];
                    count++;
                }
                if(zeroFramedAry[i-1][j-1] > 0){ // top left
                        nonZeroNeighborAry[count] = zeroFramedAry[i-1][j-1];
                        count++;
                }

                if (count == 0) return 1;   // case 1   -- all zeroes
                if (count == 1) return 2;   // case 2 -- only one is non zero
                else{   // case 2 or case 3
                    for(int ii=0; ii<count-1; ii++){
                        if (nonZeroNeighborAry[ii] != nonZeroNeighborAry[ii+1]) return 3; // check if case 3
                    }
                    return 2;   // if all the same, then case 2

                }
        }
        }
        else{   // for second pass
            if (connectedness == 4){
                if(zeroFramedAry[i+1][j] > 0){    // bottom center
                    nonZeroNeighborAry[count] = zeroFramedAry[i+1][j];
                    count++;
                } 
                if(zeroFramedAry[i][j+1] > 0){  // middle right
                    nonZeroNeighborAry[count] = zeroFramedAry[i][j+1];
                    count++;
                }
                if (count == 0) return 1;   // case 1   -- all zeroes
                for(int ii=0; ii<count; ii++){  // check if P(i,j) == all of them.
                    if( nonZeroNeighborAry[ii] != zeroFramedAry[i][j]) return 3;    // if more than 1 label, case 3
                }
                return 2;
            }
            else{

                if(zeroFramedAry[i+1][j] > 0){    // bottom center
                    nonZeroNeighborAry[count] = zeroFramedAry[i+1][j];
                    count++;
                } 
                if(zeroFramedAry[i][j+1] > 0){  // middle right
                    nonZeroNeighborAry[count] = zeroFramedAry[i][j+1];
                    count++;
                }
                if(zeroFramedAry[i+1][j+1] > 0){    // bottom right
                    nonZeroNeighborAry[count] = zeroFramedAry[i+1][j+1];
                    count++;
                }
                if(zeroFramedAry[i+1][j-1] > 0){ // bottom left
                        nonZeroNeighborAry[count] = zeroFramedAry[i+1][j-1];
                        count++;
                    }

                if (count == 0) return 1;   // case 1   -- all zeroes
                if (count == 1) return 2;   // case 2 -- only one is non zero
                else{   // case 2 or case 3
                    for(int ii=0; ii<count-1; ii++){
                        if (nonZeroNeighborAry[ii] != nonZeroNeighborAry[ii+1]) return 3; // check if case 3
                    }
                    return 2;   // if all the same, then case 2

                }

        }
     }        


    }

    void updateEq(int minLabel){
        for(int i =0; i<4; i++){
            if(nonZeroNeighborAry[i] != minLabel && nonZeroNeighborAry[i] != -1){
                eqAry[nonZeroNeighborAry[i]] = minLabel;
            }
        }
    }           

    int manageEqAry(){

        int readLabel = 0;
        int index = 1;
        while(index <= newLabel){
            if(index != eqAry[index]){  // if eq[3] != 3, then it is connected to 1 or 2
                eqAry[index] = eqAry[eqAry[index]]; // set eq[3] to eq[ eq[3] ]
            }
            else{   // else, that means eq[3] == 3 and this is a new component
                readLabel++;    
                eqAry[index] = readLabel;   // set it to its true value
            }
            index++;
        }
        return readLabel;
    }

    // basic output methods

    void drawBoxes(){
        int index, mirow, micol, marow, macol, lab; 
        index = 1;
        while(index <= trueNumCC){
            mirow = propertyAry[index].minR;
            micol = propertyAry[index].minC;
            marow = propertyAry[index].maxR;
            macol = propertyAry[index].maxC;

            lab = propertyAry[index].label;
            for(int i =mirow; i<=marow; i++){   // top to bottom
                for(int j = micol; j<=macol; j++){   // left to right
                    zeroFramedAry[i][j] = lab;  // only objects in the box
                }
            }
            index++;

        }
    }

    void printCCproperty() throws IOException{
        
        propertyFile.write( ( numRows + " " + numCols + " " + minVal + " " + maxVal ).getBytes());
        propertyFile.write( ( "\n" +trueNumCC ).getBytes());
        for(int i =1; i<=trueNumCC; i++){
                propertyFile.write( ( "\n" + i ).getBytes());
                propertyFile.write( ( "\n" + propertyAry[i].numPixels ).getBytes());
                propertyFile.write( ( "\n" + propertyAry[i].minR + " " + propertyAry[i].minC ).getBytes());
                propertyFile.write( ( "\n" + propertyAry[i].maxR + " " + propertyAry[i].maxC ).getBytes());

        }

    }

    void imgReformat() throws IOException{
        //imgReformat (zeroFramedAry, RFprettyPrintFile) // Print zeroFramedAry to RFprettyPrintFile
        prettyPrintFile.write("\n\nZeroFramedAry: ".getBytes());
        for(int i =0; i<=numRows+1; i++){
            prettyPrintFile.write("\n".getBytes());
            for(int j=0; j<=numCols+1; j++){
                if(zeroFramedAry[i][j] > 0){
                    prettyPrintFile.write( (zeroFramedAry[i][j] + "").getBytes());
                    for(int l =3; l>Integer.toString(zeroFramedAry[i][j]).length(); l--){
                        prettyPrintFile.write(" ".getBytes());
                    }
                }
                else{
                    prettyPrintFile.write("   ".getBytes());
                }

            }
        }

    }

    void printEqAry() throws IOException{

        prettyPrintFile.write(("\n\nEquivalence Array").getBytes());
        for(int i =1; i<=newLabel; i++){
            prettyPrintFile.write( ("\n" + i + " " + eqAry[i]).getBytes() );
        }

    }

    void printImg() throws IOException{

        labelFile.write((numRows + " " + numCols + " 1"  + " "+ trueNumCC).getBytes());

        for(int i=1; i<=numRows; i++){
            labelFile.write("\n".getBytes());
            for(int j =1; j<=numCols; j++){
                labelFile.write((zeroFramedAry[i][j] + "").getBytes());
                for(int l =3; l>Integer.toString(zeroFramedAry[i][j]).length(); l--){
                    labelFile.write(" ".getBytes());
                }
            }
        }

    }
    
    void zero2D(int[][] ary){
        for(int i =0; i<ary.length; i++){
            for(int j =0; j<ary[0].length; j++){
                ary[i][j]=0;
            }
        }
    }

    void minus1D(int[] ary){
        //minus1D (...) // ** Initialized a 1-D array to -1.
        for(int i=0; i<ary.length; i++){
            ary[i] = -1;
        }
    }

    void loadImage(){
        for(int i =1; i<=numRows; i++){
            for(int j=1; j<=numCols; j++ ){
                zeroFramedAry[i][j] = scanner.nextInt();
            }
        }
        scanner.close();
    }

    public static void main(String[] args) throws IOException{
        
        if(args.length != 5){
            System.out.println("Incorrect num of arguments! This program takes 4 arguments. \nYou passed " + args.length + " arguments");
            System.exit(0);
        }
        int connect = Integer.parseInt(args[0]);

        if(connect != 4 && connect != 8){
            System.out.println("Invalid inpuit for connectedness. Only '4' or '8' \nYou passed " + connect);
            System.exit(0);
        }

        new CClabel(connect,args[1], args[2], args[3], args[4]);



    }

}