package com.yourpackagename.hmtest;

import java.util.Arrays;

public class ByteUtil {
    private static final int EVERY_LINE=10;
    public static String byteArray2String(byte[] data){
        StringBuilder sb=new StringBuilder();
        //Begin line.
        sb.append("\n");
        int line_count=1;
        for (int i=0;i<data.length;i++){
            sb.append(data[i]);
            if(line_count<=EVERY_LINE){
                sb.append(" ");
                line_count++;
            }else{
                byte[] pl_data= Arrays.copyOfRange(data,i,i+EVERY_LINE);
                String str=new String(pl_data);
                sb.append("    ");
                sb.append(str);
                sb.append("\n");
                line_count=1;
            }
        }
        //End line.
        sb.append("\n");
        return sb.toString();
    }
}
