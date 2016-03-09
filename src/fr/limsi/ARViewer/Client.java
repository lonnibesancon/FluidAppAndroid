package fr.limsi.ARViewer;

import android.os.AsyncTask;
import android.util.Log;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;


public class Client extends AsyncTask<String, String, String>{


	protected String hostName = "192.168.1.41" ;
    protected int portNumber = 8500;
    //protected Socket clientSocket ;
    protected DatagramSocket clientSocket ;
    protected InetAddress serverAddr;

    protected boolean connected = false;
    protected boolean closeConnection = false ;
    protected boolean valuesupdated = false ;
    protected boolean firstConnection = true ;

    protected String dataMatrix = "1;0;0;0;0;1;0;0;0;0;1;0;0;0;0;1;";
    protected String sliceMatrix = "1;0;0;0;0;1;0;0;0;0;1;0;0;0;0;1;";
    protected String seedPoint = "-1;-1;-1";
    protected String dataToSend = "1;0;0;0;0;1;0;0;0;0;1;0;0;0;0;1;1;0;0;0;0;1;0;0;0;0;1;0;0;0;0;1;-1;-1;-1;" ;
    protected String msg ;

    private long mLastTimestamp = 0;
    private long currentTimestamp = 1000000 ;
    private long refresh = 60;
    protected int counterTries = 0 ;

    protected boolean initDone = false ;

    

    public Client(){
        super();
        Log.e("Client created", "Client Created");
    }

    public Client(String hostName){
    	this.hostName = hostName ;
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();
        if (connected == false) {
            try {
                serverAddr = InetAddress.getByName(hostName);
                Log.d("ClientActivity", "C: Connecting...");
                //clientSocket = new Socket(serverAddr, portNumber);
                clientSocket = new DatagramSocket();
                connected = true;
                Log.d("Connection status", "CONNECTED");
            } catch (Exception e) {
                Log.e("Client Application", "Error Opening"+e.getMessage(), e);
            }
        }
    }

    @Override
    protected void onPostExecute(String result){
        clientSocket.close();
        connected = false;
        Log.d("Socket State","Closed");
        super.onPostExecute(result);
    }

    @Override
    protected String doInBackground(String... f_url) {
    	if (connected == false) {
            try {
                serverAddr = InetAddress.getByName(hostName);
                Log.d("ClientActivity", "C: Connecting...");
                //clientSocket = new Socket(serverAddr, portNumber);
                clientSocket = new DatagramSocket();
                connected = true;
                Log.d("Connection status", "CONNECTED");
                String sentence = "coucou" ;
                byte[] sendData = sentence.getBytes();       
                DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, serverAddr, portNumber);       
                //clientSocket.send(sendPacket);
            } catch (Exception e) {
                Log.e("Client Application", "Error Opening"+e.getMessage(), e);
            }
        }

        while(closeConnection == false) {
            currentTimestamp = System.currentTimeMillis() ;
            long diff = currentTimestamp - mLastTimestamp ;
            //Log.d("Connected == ", "Connected = "+initDone);
            if (connected == true && initDone == true && valuesupdated == true && diff >= refresh || firstConnection){
            	//msg = ""+MATRIXCHANGED+";"+interactionMode+";"+mapperSelected+";"+matrix+PositionAndOrientation+this.seedPoint ;
                msg = ""+dataToSend;
                byte[] data = msg.getBytes();
                DatagramPacket dp = new DatagramPacket(data, data.length, this.serverAddr, portNumber);
                counterTries = 0 ;
                //Log.d("Diff", "Diff = "+diff);
                do {
                    try {
                        clientSocket.send(dp);
                        Log.d("MessageSent", ""+msg);
                        break ;
                    }catch (Exception e) {
                        Log.e("ClientActivity", "SENDING ERROR "+ counterTries, e);
                        counterTries ++ ;
                    }
                }while(counterTries < 4);


                mLastTimestamp = currentTimestamp ;
                this.valuesupdated = false ;
                firstConnection = false ;
            }
        }

        if(closeConnection==true){
            connected = false ;
            clientSocket.close();
        }

        return "";
    }

    protected void setData(String s){
    	if(s.equals(this.dataToSend) == false){
    		this.dataToSend = s ;
	    	//Log.d("DataToSend", ""+dataToSend);
	    	this.valuesupdated = true ;
	        initDone = true ;	
    	}
    }

    protected void setDataMatrixString(String s){
        this.dataMatrix = s ;
        this.valuesupdated = true ;
        initDone = true ;
    }

    protected void setSliceMatrixString(String s){
    	this.sliceMatrix = s ;
    	this.valuesupdated = true ;
    	initDone = true ;
    }

    protected void setSeedPoint(String s){
        this.seedPoint = s ;
        this.valuesupdated = true ;
        initDone = true ;
    }

}




